file(GLOB_RECURSE APP_SRCS "*.c" "../src/*.c")

idf_component_register(
    SRCS ${APP_SRCS}
    INCLUDE_DIRS "../include"
    EMBED_TXTFILES "../include/web/index.html"
    REQUIRES fatfs sdmmc driver esp_driver_spi esp_driver_gpio esp_lcd esp_timer
             esp_event nvs_flash
)