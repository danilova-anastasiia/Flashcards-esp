#include <stdio.h>
#include <unistd.h>
#include "esp_lcd_panel_st7789.h"
#include "esp_lcd_panel_dev.h"
#include "esp_lcd_panel_interface.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_panel_ops.h"
#include "hal/spi_types.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "esp_err.h"

#include "display/display_config.h"
#include "display/display.h"
#include "display/test_bitmap.h"

esp_lcd_panel_io_handle_t lcd_io_handle = NULL;
esp_lcd_panel_handle_t lcd_panel_handle = NULL;

static void displayInit(){
    gpio_set_direction(PIN_BL,GPIO_MODE_DEF_OUTPUT);
    gpio_set_level(PIN_BL,1);

    spi_bus_config_t buscfg = {
    .sclk_io_num = PIN_SCLK,
    .mosi_io_num = PIN_MOSI,
    .miso_io_num = PIN_MISO,
    .quadwp_io_num = -1,
    .quadhd_io_num = -1,
    .max_transfer_sz = LCD_H_RES * 80 * sizeof(uint16_t),
    };
    ESP_ERROR_CHECK(spi_bus_initialize(LCD_HOST, &buscfg, SPI_DMA_CH_AUTO));
    esp_lcd_panel_io_spi_config_t io_config = {
        .dc_gpio_num = PIN_DC,
        .cs_gpio_num = PIN_CS,
        .pclk_hz = PIXEL_CLOCK_HZ,
        .lcd_cmd_bits = LCD_CMD_BITS,
        .lcd_param_bits = LCD_PARAM_BITS,
        .spi_mode = 0,
        .trans_queue_depth = 10,
    };
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)LCD_HOST, &io_config, &lcd_io_handle));
    esp_lcd_panel_dev_config_t panel_config = {
        .reset_gpio_num = PIN_RST,
        .rgb_ele_order = LCD_RGB_ELEMENT_ORDER_BGR,
        .bits_per_pixel = 16,
    };
    ESP_ERROR_CHECK(esp_lcd_new_panel_st7789(lcd_io_handle, &panel_config, &lcd_panel_handle));

    esp_lcd_panel_reset(lcd_panel_handle);
    esp_lcd_panel_init(lcd_panel_handle);
    esp_lcd_panel_set_gap(lcd_panel_handle, 0, 0);
    esp_lcd_panel_disp_on_off(lcd_panel_handle, true);
}


void testDisplay(){
    displayInit();
    esp_lcd_panel_draw_bitmap(lcd_panel_handle,0,0, TEST_IMG_W, TEST_IMG_H, test_bitmap);
}