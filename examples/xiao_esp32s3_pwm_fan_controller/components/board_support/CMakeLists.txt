idf_component_register(
    SRCS "board_support.c"
    INCLUDE_DIRS "include"
    REQUIRES esp_driver_gpio esp_hw_support esp_psram spi_flash
)
