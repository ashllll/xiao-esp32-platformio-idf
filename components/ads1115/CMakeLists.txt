idf_component_register(
    SRCS "ads1115.c"
    INCLUDE_DIRS "include"
    REQUIRES esp_driver_i2c esp_timer
)
