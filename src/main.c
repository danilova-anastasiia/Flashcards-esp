#include <stdbool.h>
#include <stdint.h>
#include <sys/stat.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_lvgl_port.h"
#include "nvs_flash.h"

#include "cards/sd_card.h"
#include "display/display.h"
#include "session/session.h"
#include "web/web.h"
#include "ui/src/ui/ui.h"

#define LOOP_DELAY_MS     20

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

    if (lvgl_port_lock(0)) {
        loadScreen(SCREEN_ID_DECKS);
        ui_set_web(false);
        lvgl_port_unlock();
    }

    refreshDecks(true);

    statuses state = STATE_DECKS;

    while(true){
        uint8_t pressed = scanButtons();
        if(pressed & (1u << B_WEB)){
            if (state == STATE_DECKS) {
                toggleWeb();
            } else {
                studyStop();
                showDecks();
                state = STATE_DECKS;
            }
            pressed = 0;
        }

        switch(state){
        case STATE_DECKS:
            if(pressed&(1u << B_LEFT)){
                if(lvgl_port_lock(0)){ 
                    ui_decks_move(-1); lvgl_port_unlock(); 
                }
            }
            if(pressed&(1u << B_RIGHT)){
                if(lvgl_port_lock(0)){ 
                    ui_decks_move(+1); 
                    lvgl_port_unlock(); 
                }
            }
            if(pressed&(1u << B_MID)){
                if(decksCount()<= 0){
                    refreshDecks(true);
                }else{
                    state = startStudy();
                }
            }
            refreshDecksIfDue();
            break;

        case STATE_STUDY:
            if(pressed & ((1u << B_LEFT) | (1u << B_MID) | (1u << B_RIGHT))){
                if(lvgl_port_lock(0)){
                    if(pressed & (1u << B_MID)){
                        flip();
                    }
                    if(pressed & (1u << B_RIGHT)){
                        studyAnswer(true);
                    }else if(pressed&(1u << B_LEFT)){
                        studyAnswer(false);
                    }
                    bool finished = studyIsFinished();
                    lvgl_port_unlock();
                    if(finished){
                        state = STATE_DONE;
                    }
                }
            }
            break;

        case STATE_DONE:
            if(pressed&(1u << B_MID)){
                studyStop();
                showDecks();
                state = STATE_DECKS;
            }
            break;
        }

        vTaskDelay(pdMS_TO_TICKS(LOOP_DELAY_MS));
    }
}