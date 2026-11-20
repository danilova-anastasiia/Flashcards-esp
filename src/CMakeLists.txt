file(GLOB_RECURSE APP_SRCS "*.c" "../src/*.c")

file(GLOB_RECURSE UI_SRCS "../include/ui/src/ui/*.c")

idf_component_register(
    SRCS ${APP_SRCS} ${UI_SRCS}
    INCLUDE_DIRS "../include" "../include/ui/src/ui"
    INCLUDE_DIRS "../include"
    EMBED_TXTFILES "../include/web/index.html"
    REQUIRES fatfs sdmmc driver esp_driver_spi esp_driver_gpio esp_lcd esp_timer
             esp_event nvs_flash
)