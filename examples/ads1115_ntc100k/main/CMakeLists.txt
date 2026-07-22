idf_component_register(
    SRCS "main.c"
    INCLUDE_DIRS "." "../../../include"
    PRIV_REQUIRES ads1115 ntc_thermistor xiao_board esp_driver_i2c
)
