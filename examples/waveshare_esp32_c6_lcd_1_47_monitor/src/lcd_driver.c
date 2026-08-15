// Waveshare ESP32-C6-LCD-1.47 — ST7789 直驱 SPI 驱动
// 172x320 @ 12 MHz；面板为 BGR 像素滤波，MADCTL 置 BGR 位
#include "lcd_driver.h"
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "esp_log.h"

static const char *TAG = "lcd";

#define PIN_MOSI  6
#define PIN_SCLK  7
#define PIN_CS    14
#define PIN_DC    15
#define PIN_RST   21
#define PIN_BL    22

// 横屏（旋转 90°）：320 宽对应 GRAM 行（无偏移），172 高对应 GRAM 列（居中 34）
#define COL_OFFSET 0
#define ROW_OFFSET 34
#define SPI_FREQ   (12 * 1000 * 1000)

#define BL_MAX_PCT 40    // 背光硬上限：用户明确要求不超过 40%

static spi_device_handle_t s_spi;

// ─── SPI helpers ─────────────────────────────────────────────────────

static void spi_cmd(uint8_t cmd) {
    gpio_set_level(PIN_DC, 0);
    spi_transaction_t t = { .length = 8, .tx_buffer = &cmd };
    spi_device_polling_transmit(s_spi, &t);
}

static void spi_data(const uint8_t *data, int len) {
    if (!len) return;
    gpio_set_level(PIN_DC, 1);
    spi_transaction_t t = { .length = len * 8, .tx_buffer = data };
    spi_device_polling_transmit(s_spi, &t);
}

static void spi_byte(uint8_t v) { spi_data(&v, 1); }

static void spi_word(uint16_t v) {
    uint8_t buf[2] = { v >> 8, v & 0xFF };
    spi_data(buf, 2);
}

static void set_window(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1) {
    x0 += COL_OFFSET; x1 += COL_OFFSET;
    y0 += ROW_OFFSET; y1 += ROW_OFFSET;
    spi_cmd(0x2A); spi_word(x0); spi_word(x1);
    spi_cmd(0x2B); spi_word(y0); spi_word(y1);
    spi_cmd(0x2C);
}

// ─── ST7789 init (172x320, BGR, inversion on) ────────────────────────

static void lcd_init_regs(void) {
    spi_cmd(0x11); vTaskDelay(pdMS_TO_TICKS(120));  // Sleep out

    spi_cmd(0x36); spi_byte(0x68);  // MADCTL: landscape (MV|MX) + BGR，320x172
    spi_cmd(0x3A); spi_byte(0x55);  // 16-bit RGB565

    spi_cmd(0xB2); { uint8_t d[] = {0x0C,0x0C,0x00,0x33,0x33}; spi_data(d, 5); }
    spi_cmd(0xB7); spi_byte(0x35);
    spi_cmd(0xBB); spi_byte(0x19);
    spi_cmd(0xC0); spi_byte(0x2C);
    spi_cmd(0xC2); spi_byte(0x01);
    spi_cmd(0xC3); spi_byte(0x12);
    spi_cmd(0xC4); spi_byte(0x20);
    spi_cmd(0xC6); spi_byte(0x0F);  // 60 Hz

    spi_cmd(0xD0); { uint8_t d[] = {0xA4,0xA1}; spi_data(d, 2); }

    spi_cmd(0xE0); {
        uint8_t d[] = {0xD0,0x04,0x0D,0x11,0x13,0x2B,0x3F,0x54,0x4C,0x18,0x0D,0x0B,0x1F,0x23};
        spi_data(d, 14);
    }
    spi_cmd(0xE1); {
        uint8_t d[] = {0xD0,0x04,0x0C,0x11,0x13,0x2C,0x3F,0x44,0x51,0x2F,0x1F,0x1F,0x20,0x23};
        spi_data(d, 14);
    }

    spi_cmd(0x21);  // Inversion ON
    spi_cmd(0x29);  // Display ON
    vTaskDelay(pdMS_TO_TICKS(20));
}

// ─── Backlight (LEDC PWM) ────────────────────────────────────────────

static void bl_init(void) {
    ledc_timer_config_t tmr = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .duty_resolution = LEDC_TIMER_10_BIT,
        .timer_num = LEDC_TIMER_0,
        .freq_hz = 5000,
        .clk_cfg = LEDC_AUTO_CLK,
    };
    ledc_timer_config(&tmr);

    ledc_channel_config_t ch = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel = LEDC_CHANNEL_0,
        .timer_sel = LEDC_TIMER_0,
        .gpio_num = PIN_BL,
        .duty = 0,
    };
    ledc_channel_config(&ch);
}

void lcd_set_backlight(uint8_t pct) {
    if (pct > BL_MAX_PCT) pct = BL_MAX_PCT;   // 硬上限 40%（用户要求）
    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, (uint32_t)pct * 1023 / 100);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
}

// ─── 区域推送（16 行分块，规避 C6 SPI 单次传输上限）────────────────

void lcd_flush_area(int x0, int y0, int x1, int y1, const void *data) {
    const uint8_t *bytes = (const uint8_t *)data;
    int w = x1 - x0 + 1;
    const int rows = 16;                 // 16 行 = 5.5KB < 硬件上限

    for (int yy = y0; yy <= y1; yy += rows) {
        int ye = yy + rows - 1;
        if (ye > y1) ye = y1;
        set_window(x0, yy, x1, ye);
        gpio_set_level(PIN_DC, 1);
        int h = ye - yy + 1;
        spi_transaction_t t = {
            .length = w * h * 16,
            .tx_buffer = bytes + (yy - y0) * w * 2,
        };
        spi_device_polling_transmit(s_spi, &t);
    }
}

// ─── Init ────────────────────────────────────────────────────────────

void lcd_init(void) {
    ESP_LOGI(TAG, "ST7789 init (172x320)");

    gpio_config_t io = {
        .pin_bit_mask = (1ULL << PIN_DC) | (1ULL << PIN_RST),
        .mode = GPIO_MODE_OUTPUT,
    };
    gpio_config(&io);

    gpio_set_level(PIN_RST, 1); vTaskDelay(pdMS_TO_TICKS(10));
    gpio_set_level(PIN_RST, 0); vTaskDelay(pdMS_TO_TICKS(20));
    gpio_set_level(PIN_RST, 1); vTaskDelay(pdMS_TO_TICKS(120));

    // MISO=GPIO5 与 TF 卡共享总线（LCD 不用 MISO，但必须为 SD 注册）
    spi_bus_config_t bus = {
        .mosi_io_num = PIN_MOSI,
        .miso_io_num = 5,
        .sclk_io_num = PIN_SCLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 16384,            // 覆盖 LCD 分块与 SD 扇区传输
    };
    ESP_ERROR_CHECK(spi_bus_initialize(SPI2_HOST, &bus, SPI_DMA_CH_AUTO));

    spi_device_interface_config_t dev = {
        .clock_speed_hz = SPI_FREQ,
        .mode = 0,
        .spics_io_num = PIN_CS,
        .queue_size = 7,
    };
    ESP_ERROR_CHECK(spi_bus_add_device(SPI2_HOST, &dev, &s_spi));

    bl_init();
    lcd_init_regs();
    ESP_LOGI(TAG, "LCD ready");
}