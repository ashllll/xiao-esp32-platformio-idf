// TF 卡模块：与 LCD 共享 SPI2 总线，SPI 1-bit 模式
// 参考文档要求：低频起步、无卡不重启、CS 独立、走标准 bus/device API
// 挂载失败时打印原始扇区诊断（MBR/GPT/exFAT/FAT32 识别），便于排查卡格式
#include "tf_card.h"
#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_log.h"
#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"
#include "driver/sdspi_host.h"

static const char *TAG = "tf";

#define PIN_TF_CS   4
#define PIN_TF_MISO 5        // 总线初始化见 lcd_driver.c（MISO 5）

#define TF_MOUNT_POINT "/sdcard"

static volatile tf_state_t s_state = TF_NONE;
static volatile uint32_t  s_size_mb = 0;
static sdmmc_card_t *s_card = NULL;
static SemaphoreHandle_t s_mux;

// 挂载失败后诊断：手动走 vfs 同款初始化流程，读 MBR/BPB 判断卡格式
static void tf_diag_after_fail(sdmmc_host_t *host_in, sdspi_device_config_t *slot_in) {
    sdmmc_host_t host = *host_in;
    if (host.init) host.init();                       // sdspi_host_init
    sdspi_dev_handle_t dev = 0;
    if (sdspi_host_init_device(slot_in, &dev) != ESP_OK) {
        ESP_LOGW(TAG, "diag: sdspi device init failed");
        return;
    }
    host.slot = (int)dev;                             // vfs 内部就是拿 device handle 当 slot
    host.check_buffer_alignment = sdspi_host_check_buffer_alignment;

    sdmmc_card_t *card = (sdmmc_card_t *)malloc(sizeof(sdmmc_card_t));
    if (!card) { sdspi_host_remove_device(dev); return; }
    if (sdmmc_card_init(&host, card) != ESP_OK) {
        ESP_LOGW(TAG, "diag: card init failed（无卡或接触不良）");
        free(card);
        sdspi_host_remove_device(dev);
        return;
    }
    ESP_LOGI(TAG, "diag: card %s, size %llu MB",
             card->cid.name,
             (unsigned long long)((uint64_t)card->csd.capacity * card->csd.sector_size / (1024 * 1024)));

    uint8_t sec[512];
    if (sdmmc_read_sectors(card, sec, 0, 1) == ESP_OK) {
        bool mbr = (sec[510] == 0x55 && sec[511] == 0xAA);
        ESP_LOGI(TAG, "diag: sector0 sig=%02x%02x %s fs=%.8s",
                 sec[510], sec[511], mbr ? "(MBR)" : "(no MBR)", (char *)sec + 3);
        if (mbr) {
            uint8_t pt = sec[450];
            uint32_t pstart = sec[454] | ((uint32_t)sec[455] << 8) |
                              ((uint32_t)sec[456] << 16) | ((uint32_t)sec[457] << 24);
            ESP_LOGI(TAG, "diag: part0 type=0x%02x start_LBA=%lu %s",
                     pt, (unsigned long)pstart,
                     pt == 0xEE ? "(GPT protective)" :
                     pt == 0x0B || pt == 0x0C ? "(FAT32)" :
                     pt == 0x07 ? "(NTFS/exFAT)" : "");
            if (pt == 0xEE && sdmmc_read_sectors(card, sec, 1, 1) == ESP_OK)
                ESP_LOGI(TAG, "diag: GPT magic=%.8s", (char *)sec);
            if (pstart && sdmmc_read_sectors(card, sec, pstart, 1) == ESP_OK)
                ESP_LOGI(TAG, "diag: part0 BPB fs=%.8s", (char *)sec + 3);
        }
    } else {
        ESP_LOGW(TAG, "diag: sector0 read failed");
    }
    free(card);
    sdspi_host_remove_device(dev);
}

static void tf_mount_task(void *arg) {
    ESP_LOGI(TAG, "tf task started");
    s_state = TF_MOUNTING;

    // 低频起步：SD 规范要求 SPI 初始化 <= 400 kHz
    sdmmc_host_t host = SDSPI_HOST_DEFAULT();
    host.max_freq_khz = SDMMC_FREQ_PROBING;   // 400 kHz

    sdspi_device_config_t slot = SDSPI_DEVICE_CONFIG_DEFAULT();
    slot.gpio_cs = PIN_TF_CS;
    slot.host_id = SPI2_HOST;              // 与 LCD 同一总线，driver 层仲裁 CS

    // 挂载（内部完成 host init/device 注册/card init；format=false 绝不格式化用户卡）
    esp_vfs_fat_sdmmc_mount_config_t mcfg = {
        .format_if_mount_failed = false,
        .max_files = 2,
        .allocation_unit_size = 16 * 1024,
    };
    esp_err_t err = esp_vfs_fat_sdspi_mount(TF_MOUNT_POINT, &host, &slot, &mcfg, &s_card);
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "TF mount failed: %s（无卡/格式不支持时属正常，绝不格式化）",
                 esp_err_to_name(err));
        tf_diag_after_fail(&host, &slot);   // 失败后读原始扇区，确定卡格式
        s_state = TF_NONE;
        vTaskDelete(NULL);
        return;
    }
    s_size_mb = (uint32_t)((uint64_t)s_card->csd.capacity *
                           s_card->csd.sector_size / (1024 * 1024));
    s_state = TF_OK;
    ESP_LOGI(TAG, "TF mounted at %s, size %lu MB", TF_MOUNT_POINT, (unsigned long)s_size_mb);
    ESP_LOGI(TAG, "card: %s %s", s_card->cid.name, s_card->cid.date);
    vTaskDelete(NULL);
}

void tf_card_start(void) {
    s_mux = xSemaphoreCreateMutex();
    if (!s_mux) return;
    xTaskCreate(tf_mount_task, "tf_mount", 8192, NULL, 4, NULL);
}

tf_state_t tf_card_get_state(void) { return s_state; }
uint32_t   tf_card_get_size_mb(void) { return s_size_mb; }

void tf_append_log(const char *line) {
    if (s_state != TF_OK) return;
    xSemaphoreTake(s_mux, portMAX_DELAY);
    FILE *f = fopen(TF_MOUNT_POINT "/weather.log", "a");
    if (f) {
        fprintf(f, "%s\n", line);
        fclose(f);
    } else {
        ESP_LOGW(TAG, "open weather.log failed");
    }
    xSemaphoreGive(s_mux);
}
