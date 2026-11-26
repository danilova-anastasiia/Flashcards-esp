#include <stdbool.h>
#include <stdio.h>
#include "cards/cards.h"
#include <stdlib.h>
#include <stdint.h>
#include "session/session.h"
#include "session/pinout_buttons.h"
#include "lvgl.h"
#include "screens.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "esp_timer.h"
#include "esp_lvgl_port.h"
#include "freertos/task.h"
#include <dirent.h>
#include <string.h>
#include "ui/src/ui/ui.h"

enum {
    CARD_UNSEEN  = 0,
    CARD_KNOWN   = 1,
    CARD_UNKNOWN = 2
};

#define MAX_DECKS 16
#define DECK_DIR  "/sdcard/decks"
#define BTN_STABLE_SAMPLES 3

static int currentIndex;
static int total;
static bool showDef;
static bool busy;
static uint8_t *status;
static int knownCount;
static int unknownCount;

//for buttons
#define DEBOUNCE_DELAY_US 50000ULL
static QueueHandle_t button_queue = NULL;
static bool s_buttons_inited = false;

static char deckNames[MAX_DECKS][67];
static int  deckCount = 0;

static uint64_t lastScanUs = 0;
#define RESCAN_INTERVAL_US 2000000ULL

static const gpio_num_t btnPins[B_COUNT] = { BTN_LEFT_PIN, BTN_MID_PIN, BTN_RIGHT_PIN, BTN_WEB_PIN };
static uint8_t  btnLow[B_COUNT]    = { 0, 0, 0, 0 }; 


typedef enum {
    BTN_ID_LEFT = 0,
    BTN_ID_MID,
    BTN_ID_RIGHT
} SessionBtnId;

typedef struct {
    gpio_num_t pin;
    SessionBtnId id;
    uint64_t last_isr_us;
} IsrButtonCtx;

static IsrButtonCtx s_btn_ctx[3] = {
{ .pin = BTN_LEFT_PIN, .id = BTN_ID_LEFT, .last_isr_us = 0 },
{ .pin = BTN_MID_PIN, .id = BTN_ID_MID, .last_isr_us = 0 },
{ .pin = BTN_RIGHT_PIN, .id = BTN_ID_RIGHT, .last_isr_us = 0 }
};

static void IRAM_ATTR button_isr_handler(void *arg) {
    IsrButtonCtx *ctx = (IsrButtonCtx *)arg;
    uint64_t now = esp_timer_get_time();

    if ((now - ctx->last_isr_us) > DEBOUNCE_DELAY_US) {
        ctx->last_isr_us = now;
        SessionBtnId id = ctx->id;
        BaseType_t higher_priority_task_woken = pdFALSE;

        if (button_queue != NULL) {
            xQueueSendFromISR(button_queue, &id, &higher_priority_task_woken);
            if (higher_priority_task_woken == pdTRUE) {
                portYIELD_FROM_ISR();
            }
        }
    }
}

void studyButtonsInit(void) {
    gpio_config_t btnCfg = {
        .pin_bit_mask = (1ULL << BTN_LEFT_PIN) | (1ULL << BTN_MID_PIN) |
                        (1ULL << BTN_RIGHT_PIN) | (1ULL << BTN_WEB_PIN),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    gpio_config(&btnCfg);
}

void studyButtonsPoll(void) {
    if (button_queue == NULL) {
        return;
    }

    SessionBtnId pressed_id;
    while (xQueueReceive(button_queue, &pressed_id, 0) == pdTRUE) {
        if (total <= 0 || busy || studyIsFinished()) {
            continue;
        }

        switch (pressed_id) {
            case BTN_ID_MID:
                flip();
                break;
            case BTN_ID_RIGHT:
                studyAnswer(true);
                break;
            case BTN_ID_LEFT:
                studyAnswer(false);
                break;
            default:
                break;
        }
    }
}

void studyButtonsClearQueue(void) {
    if (button_queue != NULL) {
    xQueueReset(button_queue);
}
}



//render not public
static void render(void) {

    if (total <= 0) {
        lv_label_set_text(objects.card_label, "deck is empty");
        lv_obj_set_hidden(objects.result_label, true);
        lv_label_set_text(objects.progress, "0 / 0");
        return;
    }

    if (studyIsFinished()) {
        char buffer[67];
        snprintf(buffer, sizeof(buffer), "Know: %d / %d", knownCount, total);
        lv_label_set_text(objects.card_label, "Deck is finished");
        lv_label_set_text(objects.result_label, buffer);
        lv_obj_set_hidden(objects.result_label, false);
        return;
    }

    lv_obj_set_hidden(objects.result_label, true);

    const char *text = showDef ? deckDefinition(currentIndex) : deckTerm(currentIndex);
    lv_label_set_text(objects.card_label, text);

    char prog_buffer[67];
    snprintf(prog_buffer, sizeof(prog_buffer), "%d / %d", currentIndex + 1, total);
    lv_label_set_text(objects.progress, prog_buffer);

}


void studyStop(void) {
    if (status != NULL) {
        free(status);
        status = NULL;
    }

    deckClose();
    studyButtonsClearQueue();
    currentIndex = 0;
    total = 0;
    busy = false;
    knownCount = 0;
    unknownCount = 0;
    showDef = false;

    return;
}

int studyStart(const char *deck_path) {
    studyStop();

    total = deckOpen(deck_path);
    if (total <= 0) {
        render();
        return 0;
    }

    status = calloc(total, sizeof(uint8_t));
    if (status == NULL) {
        deckClose();
        return 0;
    }

    deckShuffle();
    currentIndex = 0;
    showDef = false; 
    knownCount = 0;
    unknownCount = 0;
    studyButtonsClearQueue();
    render();
    return 1;

}


void flip(void) {
    if (total <= 0 || busy || studyIsFinished()) {
        return;
    }
    else {
        showDef = !showDef;
        render();
        return;
    }
}

bool studyIsFinished(void) {
    if (total > 0 && currentIndex >= total) {
        return true;
    }
    else return false;
}

int studyKnown(void) {
    return knownCount;
}

int studyTotal(void) {
    return total;
}

static void flashEnd(lv_timer_t *t) {
    if(objects.obj0 != NULL){
        lv_obj_set_style_bg_color(objects.obj0, lv_color_hex(CARD_BG_COLOR), LV_PART_MAIN | LV_STATE_DEFAULT);
    }
    if(objects.card_label != NULL){
        lv_obj_set_style_text_color(objects.card_label, lv_color_hex(CARD_COLOR), LV_PART_MAIN | LV_STATE_DEFAULT);
    }
    if(objects.result_label!= NULL){
        lv_obj_set_style_text_color(objects.result_label, lv_color_hex(CARD_COLOR), LV_PART_MAIN | LV_STATE_DEFAULT);
    }
    flashTimer = NULL;

    if(studyIsFinished()){
        render();
    }
}

static void flashScreen(bool known) {
    if(flashTimer!= NULL){
        lv_timer_delete(flashTimer);
        flashTimer = NULL;
    }
    if(objects.obj0 != NULL){
        lv_obj_set_style_bg_color(objects.obj0, lv_color_hex(known ? KNOWN_COLOR : UNKNOWN_COLOR), LV_PART_MAIN | LV_STATE_DEFAULT);
    }
    if(objects.card_label != NULL){
        lv_obj_set_style_text_color(objects.card_label, lv_color_hex(known ? KNOWN_COLOR : UNKNOWN_COLOR), LV_PART_MAIN | LV_STATE_DEFAULT);
    }
    if(objects.result_label != NULL){
        lv_obj_set_style_text_color(objects.result_label, lv_color_hex(known ? KNOWN_COLOR : UNKNOWN_COLOR), LV_PART_MAIN | LV_STATE_DEFAULT);
    }
    flashTimer = lv_timer_create(flashEnd, FLASH_MS, NULL);
    lv_timer_set_repeat_count(flashTimer, 1);
}

void studyAnswer(bool known) {
    if (total <= 0 || busy || studyIsFinished()) {
        return;
    }

    if (known == true) {
        status[currentIndex] = CARD_KNOWN;
        knownCount++;
    }

    if (known != true) {
        status[currentIndex] = CARD_UNKNOWN;
        unknownCount++;
    }
    
    currentIndex++;
    showDef = false;

    if (!studyIsFinished()) {
        render();
    }
    flashScreen(known);

}

uint8_t scanButtons(void) {
    uint8_t pressed = 0;

    for (int i = 0; i < B_COUNT; i++) {
        if (gpio_get_level(btnPins[i]) == 0) {
            if (btnLow[i] < BTN_STABLE_SAMPLES) {
                btnLow[i]++;
                if (btnLow[i] == BTN_STABLE_SAMPLES) {
                    pressed |= (1u << i);
                }
            }
        } else {
            btnLow[i] = 0;
        }
    }
    return pressed;
}

static int scanDecks(char names[][67]) {
    for (int attempt = 0; attempt < 3; attempt++) {
        DIR *d = opendir(DECK_DIR);
        if (d != NULL) {
            closedir(d);
            return deckList(DECK_DIR, names, MAX_DECKS);
        }
        vTaskDelay(pdMS_TO_TICKS(50));
    }
    return -1;
}

void refreshDecks(bool force) {
    static char newNames[MAX_DECKS][67];
    static bool lastScanFailed = false;

    lastScanUs = esp_timer_get_time();
    int n = scanDecks(newNames);
    if (n < 0) {
        if (!lastScanFailed) {
            printf("SD read failed");
        }
        lastScanFailed = true;
        return;
    }
    lastScanFailed = false;

    bool same = !force && n == deckCount;
    for (int i = 0; same && i < n; i++) {
        if (strcmp(newNames[i], deckNames[i]) != 0) {
            same = false;
        }
    }
    if (same) {
        return;
    }

    if (lvgl_port_lock(0)) {
        int sel = ui_decks_selected();
        memcpy(deckNames, newNames, sizeof(deckNames));
        deckCount = n;
        ui_decks_set(deckNames, deckCount);
        if (sel > 0 && n > 0) {
            ui_decks_move(sel);
        }
        lvgl_port_unlock();
    }
    printf("found %d decks", n);
}

void refreshDecksIfDue(void) {
    if ((uint64_t)esp_timer_get_time() - lastScanUs > RESCAN_INTERVAL_US) {
        refreshDecks(false);
    }
}

int decksCount(void) {
    return deckCount;
}

void showDecks(void) {
    if (lvgl_port_lock(0)) {
        loadScreen(SCREEN_ID_DECKS);
        lvgl_port_unlock();
    }
    refreshDecks(true);
}

statuses startStudy(void) {
    if (deckCount <= 0) {
        return STATE_DECKS;
    }

    char path[128] = "";
    int ok = 0;

    if (lvgl_port_lock(0)) {
        int sel = ui_decks_selected();
        snprintf(path, sizeof(path), "%s/%s", DECK_DIR, deckNames[sel]);
        loadScreen(SCREEN_ID_MAIN);
        ok = studyStart(path);
        lvgl_port_unlock();
    }
    if (ok) {
        return STATE_STUDY;
    }
    else{
        return STATE_DONE;
    }
}


