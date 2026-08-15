# This file was automatically generated for projects
# without default 'CMakeLists.txt' file.

FILE(GLOB_RECURSE app_sources ${CMAKE_SOURCE_DIR}/src/*.*)

idf_component_register(SRCS ${app_sources}
    REQUIRES driver esp_wifi esp_netif nvs_flash esp_http_client esp_event lvgl sdmmc fatfs lwip)
