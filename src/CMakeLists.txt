idf_component_register(
    SRCS "main.c"
    INCLUDE_DIRS "../include"
    REQUIRES fatfs sdmmc driver esp_driver_spi
)