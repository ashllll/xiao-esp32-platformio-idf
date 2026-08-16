#include "interaction_demo.h"

#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include "board_pins.h"
#include "button_gpio.h"
#include "driver/gpio.h"
#include "driver/i2c_master.h"
#include "driver/ledc.h"
#include "driver/pulse_cnt.h"
#include "esp_check.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_sh1106.h"
#include "esp_log.h"
#include "esp_lvgl_port.h"
#include "esp_system.h"
#include "esp_timer.h"
#include "fan_logic.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "iot_button.h"
#include "iot_knob.h"
#include "lvgl.h"

enum {
    OLED_WIDTH = 128,
    OLED_HEIGHT = 64,
    OLED_I2C_FREQUENCY_HZ = 400000,
    OLED_I2C_PORT = 0,
    INPUT_QUEUE_DEPTH = 32,
    DUTY_MIN = 0,
    DUTY_MAX = 100,
    DUTY_STEP = 1,
    ANIMATION_MS_PER_PERCENT = 10,
    ANIMATION_MIN_MS = 70,
    PAGE_SLIDE_DISTANCE = 20,
    PAGE_SLIDE_DURATION_MS = 120,
    FAN_PWM_FREQUENCY_HZ = 25000,
    FAN_PWM_MAX_DUTY = 1023,
    FAN_TACH_PULSES_PER_REVOLUTION = 2,
    FAN_TACH_SAMPLE_MS = 1000,
    FAN_TACH_GLITCH_FILTER_NS = 1000,
};

typedef enum {
    INPUT_EVENT_ROTATE_LEFT,
    INPUT_EVENT_ROTATE_RIGHT,
    INPUT_EVENT_SINGLE_CLICK,
    INPUT_EVENT_DOUBLE_CLICK,
    INPUT_EVENT_PAGE_PREVIOUS,
    INPUT_EVENT_PAGE_NEXT,
} input_event_t;

typedef enum {
    UI_PAGE_MAIN,
    UI_PAGE_INPUT,
    UI_PAGE_SYSTEM,
    UI_PAGE_COUNT,
} ui_page_t;

static const char *TAG = "interaction_demo";
static QueueHandle_t s_input_queue;
static portMUX_TYPE s_snapshot_mux = portMUX_INITIALIZER_UNLOCKED;
static interaction_demo_snapshot_t s_snapshot;

static lv_display_t *s_display;
static lv_obj_t *s_status_badge;
static lv_obj_t *s_status_label;
static lv_obj_t *s_rpm_label;
static lv_obj_t *s_percent_label;
static lv_obj_t *s_bar;
static lv_obj_t *s_pages[UI_PAGE_COUNT];
static lv_obj_t *s_input_target_label;
static lv_obj_t *s_input_display_label;
static lv_obj_t *s_input_encoder_label;
static lv_obj_t *s_input_last_label;
static lv_obj_t *s_system_uptime_label;
static lv_obj_t *s_system_heap_label;
static ui_page_t s_active_page = UI_PAGE_MAIN;
static lv_obj_t *s_transition_from;
static bool s_page_transitioning;
static knob_handle_t s_knob;
static button_handle_t s_push_button;
static button_handle_t s_back_button;
static button_handle_t s_confirm_button;
static pcnt_unit_handle_t s_tach_pcnt_unit;

static uint32_t fan_pwm_gpio_high_duty(uint8_t percent)
{
    if (percent > DUTY_MAX) {
        percent = DUTY_MAX;
    }
    /*
     * The external N-channel MOSFET is an inverting open-drain stage: GPIO
     * high pulls the fan PWM input low. Express that inversion explicitly in
     * software so the 0% boundary is an unambiguous steady low at the fan.
     */
    return ((uint32_t)(DUTY_MAX - percent) * FAN_PWM_MAX_DUTY + 50U) /
           DUTY_MAX;
}

static esp_err_t fan_pwm_apply(bool running, uint8_t target_percent)
{
    const uint8_t applied_percent = running ? target_percent : 0U;
    if (applied_percent == 0U) {
        /* Exact GPIO high: MOSFET on, fan control input held low. */
        return ledc_stop(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, 1U);
    }
    ESP_RETURN_ON_ERROR(
        ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0,
                      fan_pwm_gpio_high_duty(applied_percent)),
        TAG, "set fan PWM duty");
    return ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
}

static esp_err_t init_fan_pwm(void)
{
    /*
     * Assert the fan's 0% control state before display/control startup. The
     * gate pulldown otherwise leaves the MOSFET off while the ESP32 boots,
     * which the fan interprets as 100% PWM.
     */
    const gpio_config_t safe_gpio_config = {
        .pin_bit_mask = 1ULL << BOARD_FAN_PWM_GPIO,
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    ESP_RETURN_ON_ERROR(gpio_config(&safe_gpio_config), TAG,
                        "configure fan PWM safe GPIO");
    ESP_RETURN_ON_ERROR(gpio_set_level(BOARD_FAN_PWM_GPIO, 1), TAG,
                        "assert fan PWM 0 percent state");

    const ledc_timer_config_t timer_config = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .duty_resolution = LEDC_TIMER_10_BIT,
        .timer_num = LEDC_TIMER_0,
        .freq_hz = FAN_PWM_FREQUENCY_HZ,
        .clk_cfg = LEDC_AUTO_CLK,
    };
    ESP_RETURN_ON_ERROR(ledc_timer_config(&timer_config), TAG,
                        "configure 25 kHz fan PWM timer");

    const ledc_channel_config_t channel_config = {
        .gpio_num = BOARD_FAN_PWM_GPIO,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel = LEDC_CHANNEL_0,
        .intr_type = LEDC_INTR_DISABLE,
        .timer_sel = LEDC_TIMER_0,
        /* Preserve the GPIO-high 0% fan command while LEDC takes ownership. */
        .duty = FAN_PWM_MAX_DUTY,
        .hpoint = 0,
        .flags = {.output_invert = 0},
    };
    ESP_RETURN_ON_ERROR(ledc_channel_config(&channel_config), TAG,
                        "configure fan PWM channel");
    return ledc_stop(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, 1U);
}

static esp_err_t init_fan_tach(void)
{
    const gpio_config_t tach_gpio_config = {
        .pin_bit_mask = 1ULL << BOARD_FAN_TACH_GPIO,
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    ESP_RETURN_ON_ERROR(gpio_config(&tach_gpio_config), TAG,
                        "configure fan tach GPIO");

    const pcnt_unit_config_t unit_config = {
        .low_limit = -1,
        .high_limit = 32767,
    };
    ESP_RETURN_ON_ERROR(pcnt_new_unit(&unit_config, &s_tach_pcnt_unit), TAG,
                        "create fan tach PCNT unit");

    const pcnt_glitch_filter_config_t filter_config = {
        .max_glitch_ns = FAN_TACH_GLITCH_FILTER_NS,
    };
    ESP_RETURN_ON_ERROR(
        pcnt_unit_set_glitch_filter(s_tach_pcnt_unit, &filter_config), TAG,
        "configure fan tach glitch filter");

    const pcnt_chan_config_t channel_config = {
        .edge_gpio_num = BOARD_FAN_TACH_GPIO,
        .level_gpio_num = -1,
    };
    pcnt_channel_handle_t channel = NULL;
    ESP_RETURN_ON_ERROR(
        pcnt_new_channel(s_tach_pcnt_unit, &channel_config, &channel), TAG,
        "create fan tach PCNT channel");
    ESP_RETURN_ON_ERROR(
        pcnt_channel_set_edge_action(channel,
                                     PCNT_CHANNEL_EDGE_ACTION_INCREASE,
                                     PCNT_CHANNEL_EDGE_ACTION_HOLD),
        TAG, "count fan tach rising edges");
    ESP_RETURN_ON_ERROR(pcnt_unit_enable(s_tach_pcnt_unit), TAG,
                        "enable fan tach PCNT unit");
    ESP_RETURN_ON_ERROR(pcnt_unit_clear_count(s_tach_pcnt_unit), TAG,
                        "clear fan tach PCNT count");
    return pcnt_unit_start(s_tach_pcnt_unit);
}

static esp_err_t sample_fan_tach(uint32_t *pulses, uint32_t *rpm)
{
    ESP_RETURN_ON_FALSE(pulses != NULL && rpm != NULL, ESP_ERR_INVALID_ARG,
                        TAG, "fan tach sample outputs are required");

    int pulse_count = 0;
    ESP_RETURN_ON_ERROR(pcnt_unit_get_count(s_tach_pcnt_unit, &pulse_count),
                        TAG, "read fan tach PCNT count");
    ESP_RETURN_ON_ERROR(pcnt_unit_clear_count(s_tach_pcnt_unit), TAG,
                        "reset fan tach PCNT count");

    *pulses = pulse_count > 0 ? (uint32_t)pulse_count : 0U;
    *rpm = fan_logic_rpm(*pulses, FAN_TACH_PULSES_PER_REVOLUTION,
                         FAN_TACH_SAMPLE_MS);
    return ESP_OK;
}

static void snapshot_update(bool running, uint8_t target, uint8_t displayed)
{
    taskENTER_CRITICAL(&s_snapshot_mux);
    s_snapshot.running = running;
    s_snapshot.target_percent = target;
    s_snapshot.displayed_percent = displayed;
    taskEXIT_CRITICAL(&s_snapshot_mux);
}

void interaction_demo_get_snapshot(interaction_demo_snapshot_t *snapshot)
{
    if (snapshot == NULL) {
        return;
    }
    taskENTER_CRITICAL(&s_snapshot_mux);
    *snapshot = s_snapshot;
    taskEXIT_CRITICAL(&s_snapshot_mux);
}

static void duty_animation_exec(void *object, int32_t value)
{
    (void)object;
    interaction_demo_snapshot_t snapshot;
    interaction_demo_get_snapshot(&snapshot);

    lv_bar_set_value(s_bar, value, LV_ANIM_OFF);
    lv_label_set_text_fmt(s_percent_label, "%3" LV_PRId32 "%%", value);
    lv_label_set_text_fmt(s_input_display_label, "DISPLAY %3" LV_PRId32 "%%", value);
    snapshot_update(snapshot.running, snapshot.target_percent, (uint8_t)value);
}

static void animate_duty_to(uint8_t target)
{
    interaction_demo_snapshot_t snapshot;
    interaction_demo_get_snapshot(&snapshot);
    int32_t current = snapshot.displayed_percent;
    int32_t distance = abs((int)target - (int)current);

    snapshot_update(snapshot.running, target, snapshot.displayed_percent);
    lv_label_set_text_fmt(s_input_target_label, "TARGET  %3u%%", (unsigned)target);
    lv_anim_delete(s_bar, duty_animation_exec);
    if (distance == 0) {
        duty_animation_exec(s_bar, target);
        return;
    }

    uint32_t duration = (uint32_t)distance * ANIMATION_MS_PER_PERCENT;
    if (duration < ANIMATION_MIN_MS) {
        duration = ANIMATION_MIN_MS;
    }
    lv_anim_t animation;
    lv_anim_init(&animation);
    lv_anim_set_var(&animation, s_bar);
    lv_anim_set_exec_cb(&animation, duty_animation_exec);
    lv_anim_set_values(&animation, current, target);
    lv_anim_set_duration(&animation, duration);
    lv_anim_set_path_cb(&animation, lv_anim_path_linear);
    lv_anim_start(&animation);
}

static lv_obj_t *create_page(lv_obj_t *screen)
{
    lv_obj_t *page = lv_obj_create(screen);
    lv_obj_remove_style_all(page);
    lv_obj_set_size(page, OLED_WIDTH, OLED_HEIGHT);
    lv_obj_set_pos(page, 0, 0);
    lv_obj_set_style_bg_color(page, lv_color_white(), 0);
    lv_obj_set_style_bg_opa(page, LV_OPA_COVER, 0);
    lv_obj_set_style_text_color(page, lv_color_black(), 0);
    lv_obj_clear_flag(page, LV_OBJ_FLAG_SCROLLABLE);
    return page;
}

static lv_obj_t *create_text_label(lv_obj_t *parent, const char *text,
                                   lv_align_t align, int32_t x, int32_t y)
{
    lv_obj_t *label = lv_label_create(parent);
    lv_obj_set_style_text_font(label, &lv_font_unscii_8, 0);
    lv_label_set_text(label, text);
    lv_obj_align(label, align, x, y);
    return label;
}

static void create_page_header(lv_obj_t *page, const char *title)
{
    create_text_label(page, title, LV_ALIGN_TOP_LEFT, 2, 2);
    lv_obj_t *divider = lv_obj_create(page);
    lv_obj_remove_style_all(divider);
    lv_obj_set_size(divider, 124, 1);
    lv_obj_align(divider, LV_ALIGN_TOP_MID, 0, 13);
    lv_obj_set_style_bg_color(divider, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(divider, LV_OPA_COVER, 0);
}

static void update_input_diagnostics(uint8_t target, uint8_t displayed,
                                     const char *last_direction)
{
    int knob_count = s_knob != NULL ? iot_knob_get_count_value(s_knob) : 0;
    lv_label_set_text_fmt(s_input_target_label, "TARGET  %3u%%", (unsigned)target);
    lv_label_set_text_fmt(s_input_display_label, "DISPLAY %3u%%", (unsigned)displayed);
    lv_label_set_text_fmt(s_input_encoder_label, "ENC %+6d", knob_count);
    if (last_direction != NULL) {
        lv_label_set_text_fmt(s_input_last_label, "LAST %s", last_direction);
    }
}

static void page_slide_exec(void *object, int32_t value)
{
    lv_obj_set_x((lv_obj_t *)object, value);
}

static void page_slide_completed(lv_anim_t *animation)
{
    (void)animation;
    if (s_transition_from != NULL) {
        lv_obj_add_flag(s_transition_from, LV_OBJ_FLAG_HIDDEN);
        lv_obj_set_x(s_transition_from, 0);
        s_transition_from = NULL;
    }
    s_page_transitioning = false;
}

static void show_page(ui_page_t next_page, int direction)
{
    if (next_page == s_active_page || s_page_transitioning) {
        return;
    }

    lv_obj_t *next = s_pages[next_page];
    s_transition_from = s_pages[s_active_page];
    s_page_transitioning = true;
    s_active_page = next_page;

    lv_anim_delete(next, page_slide_exec);
    lv_obj_clear_flag(next, LV_OBJ_FLAG_HIDDEN);
    lv_obj_move_foreground(next);
    lv_obj_set_x(next, direction * PAGE_SLIDE_DISTANCE);

    lv_anim_t animation;
    lv_anim_init(&animation);
    lv_anim_set_var(&animation, next);
    lv_anim_set_exec_cb(&animation, page_slide_exec);
    lv_anim_set_values(&animation, direction * PAGE_SLIDE_DISTANCE, 0);
    lv_anim_set_duration(&animation, PAGE_SLIDE_DURATION_MS);
    lv_anim_set_path_cb(&animation, lv_anim_path_ease_out);
    lv_anim_set_completed_cb(&animation, page_slide_completed);
    lv_anim_start(&animation);
}

static void refresh_run_state(bool running)
{
    interaction_demo_snapshot_t snapshot;
    interaction_demo_get_snapshot(&snapshot);
    snapshot_update(running, snapshot.target_percent, snapshot.displayed_percent);
    lv_label_set_text(s_status_label, running ? "ON" : "OFF");
    lv_obj_set_style_bg_opa(s_status_badge,
                            running ? LV_OPA_COVER : LV_OPA_TRANSP, 0);
    lv_obj_set_style_text_color(s_status_label,
                                running ? lv_color_white() : lv_color_black(), 0);
}

static void create_ui(void)
{
    lv_display_set_default(s_display);
    lv_obj_t *screen = lv_display_get_screen_active(s_display);
    lv_obj_remove_style_all(screen);
    /* esp_lvgl_port maps LVGL I1 bits to OLED page bits with inverted polarity. */
    lv_obj_set_style_bg_color(screen, lv_color_white(), 0);
    lv_obj_set_style_bg_opa(screen, LV_OPA_COVER, 0);
    lv_obj_set_style_text_color(screen, lv_color_black(), 0);

    s_pages[UI_PAGE_MAIN] = create_page(screen);
    s_pages[UI_PAGE_INPUT] = create_page(screen);
    s_pages[UI_PAGE_SYSTEM] = create_page(screen);
    lv_obj_add_flag(s_pages[UI_PAGE_INPUT], LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(s_pages[UI_PAGE_SYSTEM], LV_OBJ_FLAG_HIDDEN);

    lv_obj_t *main_page = s_pages[UI_PAGE_MAIN];
    lv_obj_t *pwm_label = lv_label_create(main_page);
    lv_obj_set_style_text_font(pwm_label, &lv_font_unscii_8, 0);
    lv_label_set_text(pwm_label, "PWM");
    lv_obj_align(pwm_label, LV_ALIGN_TOP_LEFT, 2, 2);

    s_status_badge = lv_obj_create(main_page);
    lv_obj_remove_style_all(s_status_badge);
    lv_obj_set_size(s_status_badge, 28, 10);
    lv_obj_align(s_status_badge, LV_ALIGN_TOP_LEFT, 29, 1);
    lv_obj_set_style_border_width(s_status_badge, 1, 0);
    lv_obj_set_style_border_color(s_status_badge, lv_color_black(), 0);
    lv_obj_set_style_bg_color(s_status_badge, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(s_status_badge, LV_OPA_TRANSP, 0);

    s_status_label = lv_label_create(s_status_badge);
    lv_obj_set_style_text_font(s_status_label, &lv_font_unscii_8, 0);
    lv_obj_set_style_text_color(s_status_label, lv_color_black(), 0);
    lv_label_set_text(s_status_label, "OFF");
    lv_obj_center(s_status_label);

    s_rpm_label = lv_label_create(main_page);
    lv_obj_set_style_text_font(s_rpm_label, &lv_font_unscii_8, 0);
    lv_label_set_text(s_rpm_label, "RPM 0");
    lv_obj_align(s_rpm_label, LV_ALIGN_TOP_RIGHT, -2, 2);

    lv_obj_t *divider = lv_obj_create(main_page);
    lv_obj_remove_style_all(divider);
    lv_obj_set_size(divider, 124, 1);
    lv_obj_align(divider, LV_ALIGN_TOP_MID, 0, 13);
    lv_obj_set_style_bg_color(divider, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(divider, LV_OPA_COVER, 0);

    s_percent_label = lv_label_create(main_page);
    lv_label_set_text(s_percent_label, "  0%");
    lv_obj_set_size(s_percent_label, 64, 16);
    lv_obj_set_style_text_font(s_percent_label, &lv_font_unscii_16, 0);
    lv_obj_set_style_text_align(s_percent_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(s_percent_label, LV_ALIGN_CENTER, 0, 2);

    s_bar = lv_bar_create(main_page);
    lv_obj_set_size(s_bar, 120, 5);
    lv_obj_align(s_bar, LV_ALIGN_BOTTOM_MID, 0, -3);
    lv_bar_set_range(s_bar, DUTY_MIN, DUTY_MAX);
    lv_bar_set_value(s_bar, 0, LV_ANIM_OFF);
    lv_obj_set_style_radius(s_bar, 0, LV_PART_MAIN);
    lv_obj_set_style_radius(s_bar, 0, LV_PART_INDICATOR);
    lv_obj_set_style_bg_opa(s_bar, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(s_bar, 1, LV_PART_MAIN);
    lv_obj_set_style_border_color(s_bar, lv_color_black(), LV_PART_MAIN);
    lv_obj_set_style_bg_color(s_bar, lv_color_black(), LV_PART_INDICATOR);
    lv_obj_set_style_bg_opa(s_bar, LV_OPA_COVER, LV_PART_INDICATOR);

    lv_obj_t *input_page = s_pages[UI_PAGE_INPUT];
    create_page_header(input_page, "INPUT 2/3");
    s_input_target_label = create_text_label(
        input_page, "TARGET    0%", LV_ALIGN_TOP_LEFT, 2, 17);
    s_input_display_label = create_text_label(
        input_page, "DISPLAY   0%", LV_ALIGN_TOP_LEFT, 2, 29);
    s_input_encoder_label = create_text_label(
        input_page, "ENC     +0", LV_ALIGN_TOP_LEFT, 2, 41);
    s_input_last_label = create_text_label(
        input_page, "LAST -", LV_ALIGN_TOP_LEFT, 2, 53);

    lv_obj_t *system_page = s_pages[UI_PAGE_SYSTEM];
    create_page_header(system_page, "SYSTEM 3/3");
    s_system_uptime_label = create_text_label(
        system_page, "UP 0s", LV_ALIGN_TOP_LEFT, 2, 17);
    s_system_heap_label = create_text_label(
        system_page, "HEAP 0K", LV_ALIGN_TOP_LEFT, 2, 29);
    create_text_label(system_page, "OLED SH1106 3V3",
                      LV_ALIGN_TOP_LEFT, 2, 41);
    create_text_label(system_page, "PWM3 TACH4 F12",
                      LV_ALIGN_TOP_LEFT, 2, 53);
}

static esp_err_t init_display(void)
{
    i2c_master_bus_handle_t i2c_bus = NULL;
    i2c_master_bus_config_t bus_config = {
        .i2c_port = OLED_I2C_PORT,
        .sda_io_num = BOARD_OLED_SDA_GPIO,
        .scl_io_num = BOARD_OLED_SCL_GPIO,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true,
    };
    ESP_RETURN_ON_ERROR(i2c_new_master_bus(&bus_config, &i2c_bus), TAG,
                        "create OLED I2C bus");

    esp_lcd_panel_io_handle_t panel_io = NULL;
    esp_lcd_panel_io_i2c_config_t io_config = {
        .dev_addr = BOARD_OLED_I2C_ADDRESS,
        .scl_speed_hz = OLED_I2C_FREQUENCY_HZ,
        .control_phase_bytes = 1,
        .dc_bit_offset = 6,
        .lcd_cmd_bits = 8,
        .lcd_param_bits = 8,
    };
    ESP_RETURN_ON_ERROR(esp_lcd_new_panel_io_i2c(i2c_bus, &io_config, &panel_io),
                        TAG, "attach SH1106 at 0x3C");

    esp_lcd_panel_dev_config_t panel_config = {
        .reset_gpio_num = GPIO_NUM_NC,
        .bits_per_pixel = 1,
    };
    esp_lcd_panel_handle_t panel = NULL;
    ESP_RETURN_ON_ERROR(esp_lcd_new_panel_sh1106(
                            panel_io, &panel_config, &panel),
                        TAG, "create SH1106 panel");
    ESP_RETURN_ON_ERROR(esp_lcd_panel_reset(panel), TAG, "reset SH1106");
    ESP_RETURN_ON_ERROR(esp_lcd_panel_init(panel), TAG, "initialize SH1106");
    ESP_RETURN_ON_ERROR(esp_lcd_panel_disp_on_off(panel, true), TAG,
                        "enable SH1106");

    const lvgl_port_cfg_t lvgl_config = ESP_LVGL_PORT_INIT_CONFIG();
    ESP_RETURN_ON_ERROR(lvgl_port_init(&lvgl_config), TAG, "start LVGL port");
    const lvgl_port_display_cfg_t display_config = {
        .io_handle = panel_io,
        .panel_handle = panel,
        .buffer_size = OLED_WIDTH * OLED_HEIGHT,
        .double_buffer = true,
        .hres = OLED_WIDTH,
        .vres = OLED_HEIGHT,
        .monochrome = true,
        .color_format = LV_COLOR_FORMAT_I1,
        .rotation = {
            .swap_xy = false,
            .mirror_x = false,
            /* Preserve the SH1106 driver's default COM scan direction. */
            .mirror_y = true,
        },
        .flags = {
            .swap_bytes = false,
            .sw_rotate = false,
        },
    };
    s_display = lvgl_port_add_disp(&display_config);
    ESP_RETURN_ON_FALSE(s_display != NULL, ESP_FAIL, TAG, "attach LVGL display");

    ESP_RETURN_ON_FALSE(lvgl_port_lock(0), ESP_ERR_TIMEOUT, TAG, "lock LVGL");
    lv_display_set_rotation(s_display, LV_DISPLAY_ROTATION_0);
    create_ui();
    lvgl_port_unlock();
    return ESP_OK;
}

static void queue_input_event(input_event_t event)
{
    if (xQueueSend(s_input_queue, &event, 0) != pdPASS) {
        ESP_LOGW(TAG, "Input queue full; event=%d dropped", (int)event);
    }
}

static void knob_left_callback(void *handle, void *user_data)
{
    (void)handle;
    (void)user_data;
    queue_input_event(INPUT_EVENT_ROTATE_LEFT);
}

static void knob_right_callback(void *handle, void *user_data)
{
    (void)handle;
    (void)user_data;
    queue_input_event(INPUT_EVENT_ROTATE_RIGHT);
}

static void button_single_callback(void *handle, void *user_data)
{
    (void)handle;
    (void)user_data;
    queue_input_event(INPUT_EVENT_SINGLE_CLICK);
}

static void button_double_callback(void *handle, void *user_data)
{
    (void)handle;
    (void)user_data;
    queue_input_event(INPUT_EVENT_DOUBLE_CLICK);
}

static void back_button_callback(void *handle, void *user_data)
{
    (void)handle;
    (void)user_data;
    queue_input_event(INPUT_EVENT_PAGE_PREVIOUS);
}

static void confirm_button_callback(void *handle, void *user_data)
{
    (void)handle;
    (void)user_data;
    queue_input_event(INPUT_EVENT_PAGE_NEXT);
}

static esp_err_t create_active_low_button(gpio_num_t gpio_num,
                                          button_handle_t *button)
{
    const button_config_t button_config = {0};
    const button_gpio_config_t gpio_config = {
        .gpio_num = gpio_num,
        .active_level = 0,
        .enable_power_save = false,
        .disable_pull = false,
    };
    return iot_button_new_gpio_device(&button_config, &gpio_config, button);
}

static esp_err_t init_controls(void)
{
    const knob_config_t knob_config = {
        .default_direction = 0,
        .gpio_encoder_a = BOARD_ENCODER_A_GPIO,
        .gpio_encoder_b = BOARD_ENCODER_B_GPIO,
        .enable_power_save = false,
    };
    s_knob = iot_knob_create(&knob_config);
    ESP_RETURN_ON_FALSE(s_knob != NULL, ESP_FAIL, TAG, "create EC10 knob");
    ESP_RETURN_ON_ERROR(iot_knob_register_cb(
                            s_knob, KNOB_LEFT, knob_left_callback, NULL),
                        TAG, "register left rotation");
    ESP_RETURN_ON_ERROR(iot_knob_register_cb(
                            s_knob, KNOB_RIGHT, knob_right_callback, NULL),
                        TAG, "register right rotation");

    ESP_RETURN_ON_ERROR(create_active_low_button(BOARD_ENCODER_PUSH_GPIO,
                                                  &s_push_button),
                        TAG, "create encoder push button");
    ESP_RETURN_ON_ERROR(iot_button_register_cb(
                            s_push_button, BUTTON_SINGLE_CLICK, NULL,
                            button_single_callback, NULL),
                        TAG, "register single click");
    ESP_RETURN_ON_ERROR(iot_button_register_cb(
                            s_push_button, BUTTON_DOUBLE_CLICK, NULL,
                            button_double_callback, NULL),
                        TAG, "register double click");

    ESP_RETURN_ON_ERROR(create_active_low_button(BOARD_BACK_GPIO,
                                                  &s_back_button),
                        TAG, "create BACK button");
    ESP_RETURN_ON_ERROR(iot_button_register_cb(
                            s_back_button, BUTTON_PRESS_DOWN, NULL,
                            back_button_callback, NULL),
                        TAG, "register BACK button");
    ESP_RETURN_ON_ERROR(create_active_low_button(BOARD_CONFIRM_GPIO,
                                                  &s_confirm_button),
                        TAG, "create CONFIRM button");
    return iot_button_register_cb(s_confirm_button, BUTTON_PRESS_DOWN, NULL,
                                  confirm_button_callback, NULL);
}

static void input_event_task(void *argument)
{
    (void)argument;
    input_event_t event;
    while (true) {
        if (xQueueReceive(s_input_queue, &event, portMAX_DELAY) != pdPASS) {
            continue;
        }

        interaction_demo_snapshot_t snapshot;
        interaction_demo_get_snapshot(&snapshot);
        uint8_t target = snapshot.target_percent;
        bool running = snapshot.running;

        if (event == INPUT_EVENT_ROTATE_RIGHT && target < DUTY_MAX) {
            target += DUTY_STEP;
        } else if (event == INPUT_EVENT_ROTATE_LEFT && target > DUTY_MIN) {
            target -= DUTY_STEP;
        } else if (event == INPUT_EVENT_SINGLE_CLICK) {
            running = true;
        } else if (event == INPUT_EVENT_DOUBLE_CLICK) {
            running = false;
        }

        if (!lvgl_port_lock(0)) {
            ESP_LOGE(TAG, "Unable to lock LVGL for input event");
            continue;
        }
        if ((target != snapshot.target_percent || running != snapshot.running) &&
            fan_pwm_apply(running, target) != ESP_OK) {
            ESP_LOGE(TAG, "Unable to apply fan PWM output");
            lvgl_port_unlock();
            continue;
        }
        if (target != snapshot.target_percent) {
            animate_duty_to(target);
            update_input_diagnostics(
                target, snapshot.displayed_percent,
                event == INPUT_EVENT_ROTATE_RIGHT ? "CW" : "CCW");
            ESP_LOGI(TAG, "EC10 duty=%u%%", (unsigned)target);
        }
        if (running != snapshot.running) {
            refresh_run_state(running);
            ESP_LOGI(TAG, "EC10 %s click: %s",
                     event == INPUT_EVENT_SINGLE_CLICK ? "single" : "double",
                     running ? "RUN" : "STOP");
        }
        if (event == INPUT_EVENT_PAGE_PREVIOUS) {
            ui_page_t next = s_active_page == UI_PAGE_MAIN
                                 ? UI_PAGE_SYSTEM
                                 : (ui_page_t)(s_active_page - 1);
            show_page(next, -1);
            ESP_LOGI(TAG, "BACK page=%u", (unsigned)next + 1U);
        } else if (event == INPUT_EVENT_PAGE_NEXT) {
            ui_page_t next = (ui_page_t)((s_active_page + 1) % UI_PAGE_COUNT);
            show_page(next, 1);
            ESP_LOGI(TAG, "CONFIRM page=%u", (unsigned)next + 1U);
        }
        lvgl_port_unlock();
    }
}

static void heartbeat_task(void *argument)
{
    (void)argument;
    uint32_t sequence = 0;
    TickType_t last_wake = xTaskGetTickCount();
    while (true) {
        vTaskDelayUntil(&last_wake, pdMS_TO_TICKS(FAN_TACH_SAMPLE_MS));

        interaction_demo_snapshot_t snapshot;
        interaction_demo_get_snapshot(&snapshot);
        int knob_count = s_knob != NULL ? iot_knob_get_count_value(s_knob) : 0;
        uint32_t uptime_seconds = (uint32_t)(esp_timer_get_time() / 1000000LL);
        uint32_t free_heap_kb = esp_get_free_heap_size() / 1024U;
        uint32_t tach_pulses = 0;
        uint32_t rpm = 0;
        esp_err_t tach_result = sample_fan_tach(&tach_pulses, &rpm);
        if (lvgl_port_lock(0)) {
            if (tach_result == ESP_OK) {
                lv_label_set_text_fmt(s_rpm_label, "RPM %" PRIu32, rpm);
            } else {
                lv_label_set_text(s_rpm_label, "RPM ERR");
            }
            lv_label_set_text_fmt(s_system_uptime_label, "UP %" PRIu32 "s",
                                  uptime_seconds);
            lv_label_set_text_fmt(s_system_heap_label, "HEAP %" PRIu32 "K",
                                  free_heap_kb);
            update_input_diagnostics(snapshot.target_percent,
                                     snapshot.displayed_percent, NULL);
            lvgl_port_unlock();
        }
        ESP_LOGI(TAG,
                 "HEARTBEAT sequence=%" PRIu32
                 " run=%d target=%u display=%u rpm=%" PRIu32
                 " tach_pulses=%" PRIu32
                 " knob=%d page=%u heap_kb=%" PRIu32,
                 ++sequence, snapshot.running, snapshot.target_percent,
                 snapshot.displayed_percent, rpm, tach_pulses, knob_count,
                 (unsigned)s_active_page + 1U, free_heap_kb);
    }
}

esp_err_t interaction_demo_start(void)
{
    s_input_queue = xQueueCreate(INPUT_QUEUE_DEPTH, sizeof(input_event_t));
    ESP_RETURN_ON_FALSE(s_input_queue != NULL, ESP_ERR_NO_MEM, TAG,
                        "create input event queue");
    snapshot_update(false, 0, 0);

    ESP_RETURN_ON_ERROR(init_fan_pwm(), TAG, "configure fan PWM safe output");
    ESP_RETURN_ON_ERROR(init_fan_tach(), TAG, "configure D4 fan tach input");
    ESP_RETURN_ON_ERROR(init_display(), TAG, "configure SH1106 OLED");
    ESP_RETURN_ON_ERROR(init_controls(), TAG, "configure encoder libraries");
    ESP_RETURN_ON_FALSE(xTaskCreate(input_event_task, "ec10_events", 4096,
                                    NULL, 5, NULL) == pdPASS,
                        ESP_ERR_NO_MEM, TAG, "create EC10 event task");
    ESP_RETURN_ON_FALSE(xTaskCreate(heartbeat_task, "demo_heartbeat", 3072,
                                    NULL, 2, NULL) == pdPASS,
                        ESP_ERR_NO_MEM, TAG, "create heartbeat task");
    ESP_RETURN_ON_ERROR(fan_pwm_apply(false, 0), TAG,
                        "apply initial stopped state");

    ESP_LOGI(TAG,
             "Libraries: ESP-IDF LEDC + espressif/knob + espressif/button + "
             "ESP-IDF PCNT + LVGL + SH1106");
    ESP_LOGI(TAG, "Encoder GPIO: PUSH=%d A=%d B=%d BACK=%d CONFIRM=%d; "
                  "OLED SDA=%d SCL=%d addr=0x%02X",
             BOARD_ENCODER_PUSH_GPIO, BOARD_ENCODER_A_GPIO,
             BOARD_ENCODER_B_GPIO, BOARD_BACK_GPIO, BOARD_CONFIRM_GPIO,
             BOARD_OLED_SDA_GPIO, BOARD_OLED_SCL_GPIO,
             BOARD_OLED_I2C_ADDRESS);
    ESP_LOGI(TAG,
             "Fan PWM GPIO=%d frequency=%u Hz software_invert=1 "
             "idle_0pct=HIGH; tach GPIO=%d ppr=%u sample_ms=%u",
             BOARD_FAN_PWM_GPIO, FAN_PWM_FREQUENCY_HZ, BOARD_FAN_TACH_GPIO,
             FAN_TACH_PULSES_PER_REVOLUTION, FAN_TACH_SAMPLE_MS);
    ESP_LOGW(TAG, "Fan power switching remains disabled");
    return ESP_OK;
}
