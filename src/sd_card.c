#include "cards/sd_card.h"
#include "cards/sd_card_pinout.h"

#include "esp_log.h"
#include "esp_vfs_fat.h"
#include "driver/spi_common.h"
#include "sdmmc_cmd.h"

static const char *TAG = "sd";
static sdmmc_card_t *sdCard = NULL;

esp_err_t sdOpen(void){
    if(sdCard != NULL) {
        return ESP_OK;
    }

    esp_vfs_fat_sdmmc_mount_config_t mountCfg = {
        .format_if_mount_failed = false,
        .max_files = 5,
        .allocation_unit_size = 16 * 1024
    };

    spi_bus_config_t spiCfg = {
        .mosi_io_num = PIN_MOSI,
        .miso_io_num = PIN_MISO,
        .sclk_io_num = PIN_CLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 4000,
    };

    esp_err_t err = spi_bus_initialize(SPI3_HOST, &spiCfg, SPI_DMA_CH_AUTO);
    if (err != ESP_OK && err != ESP_ERR_INVALID_STATE){
        ESP_LOGE(TAG, "spi init error: %s", esp_err_to_name(err));
        return err;
    }
    
    sdmmc_host_t host = SDSPI_HOST_DEFAULT();
    host.slot = SPI3_HOST;
    host.max_freq_khz = 10000;

    sdspi_device_config_t slotCfg = SDSPI_DEVICE_CONFIG_DEFAULT();
    slotCfg.gpio_cs = PIN_CS;
    slotCfg.host_id = host.slot;

    err = esp_vfs_fat_sdspi_mount(MOUNT_POINT, &host, &slotCfg, &mountCfg, &sdCard);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "mount error: %s", esp_err_to_name(err));
        sdCard = NULL;
        return err;
    }

    sdmmc_card_print_info(stdout, sdCard);
    return ESP_OK;
}

void sdClose(void) {
    if (sdCard == NULL) {
        return;
    }

    esp_vfs_fat_sdcard_unmount(MOUNT_POINT,sdCard);
    sdCard = NULL;
}
