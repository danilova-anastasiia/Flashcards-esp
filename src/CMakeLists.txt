idf_component_register(
    SRCS "main.c"
    INCLUDE_DIRS "../include"
    REQUIRES fatfs sdmmc driver esp_driver_spi esp_lcd esp_timer
            esp_netif esp_event nvs_flash lvgl__lvgl
)