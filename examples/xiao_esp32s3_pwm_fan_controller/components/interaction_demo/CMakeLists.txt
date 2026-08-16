idf_component_register(
    SRCS "interaction_demo.c"
    INCLUDE_DIRS "include"
    REQUIRES board_support fan_logic esp_driver_gpio esp_driver_i2c esp_driver_ledc esp_driver_pcnt esp_lcd esp_lvgl_port esp_timer knob button lvgl sh1106-esp-idf
)
