#include <sys/stat.h>

#include "freertos/FreeRTOS.h"
#include "esp_lvgl_port.h"
#include "nvs_flash.h"

#include "cards/sd_card.h"
#include "display/display.h"
#include "session/session.h"
#include "web/web.h"
#include "ui/src/ui/ui.h"

#define LOOP_DELAY_MS 20

void app_main(void) {
    nvs_flash_init();

    displayInit();

    if(sdOpen()==ESP_OK){
        mkdir(DECK_DIR, 0775);
    }else{
        printf("sd card not available");
    }

    lvglInit();

    studyButtonsInit();

    if(lvgl_port_lock(0)){
        loadScreen(SCREEN_ID_DECKS);
        ui_set_web(false);
        lvgl_port_unlock();
    }

    refreshDecks(true);

    statuses state = STATE_DECKS;
}