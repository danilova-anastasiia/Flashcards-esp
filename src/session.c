#include <stdbool.h>
#include <stdio.h>
#include "cards.h"
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

enum {
    CARD_UNSEEN  = 0,
    CARD_KNOWN   = 1,
    CARD_UNKNOWN = 2
};

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
    if (s_buttons_inited == true) {
        return;
    }

    if (button_queue == NULL) {
        button_queue = xQueueCreate(16, sizeof(SessionBtnId));
    }

    gpio_config_t io_conf = {
        .intr_type = GPIO_INTR_NEGEDGE,
        .mode = GPIO_MODE_INPUT,
        .pin_bit_mask = (1ULL << BTN_LEFT_PIN) | (1ULL << BTN_MID_PIN) | (1ULL << BTN_RIGHT_PIN),
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .pull_up_en = GPIO_PULLUP_ENABLE
    };
    gpio_config(&io_conf);

    esp_err_t err = gpio_install_isr_service(0);
    if (err != ESP_OK && err != ESP_ERR_INVALID_STATE) {
        return;
    }

    for (int i = 0; i < 3; i++) {
        gpio_isr_handler_add(s_btn_ctx[i].pin, button_isr_handler, &s_btn_ctx[i]);
    }
    s_buttons_inited = true;
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
        lv_label_set_text(objects.card_label, "Дека порожня");
        lv_label_set_text(objects.progress, "0 / 0");
        lv_obj_set_hidden(objects.result_label, true);
        return;
    }

    if (studyIsFinished()) {
        char buffer[67];
        snprintf(buffer, sizeof(buffer), "Знаю: %d / %d", knownCount, total);
        lv_label_set_text(objects.result_label, buffer);
        lv_obj_set_hidden(objects.result_label, false);
        lv_label_set_text(objects.card_label, "Деку пройдено!");
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
    render();

}


