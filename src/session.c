#include <stdbool.h>
#include <stdio.h>
#include "cards.h"
#include <stdlib.h>
#include <stdint.h>
#include "session.h"
#include "pinout.h"
#include "lvgl.h"
#include "screens.h"
#include "driver/gpio.h"

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
static bool previousLeft = false;
static bool previousMid = false;
static bool previousRight = false;



//render not public
static void render(void) {

    if (total <= 0) {
        lv_label_set_text(objects.card_label, "Дека порожня");
        lv_label_set_text(objects.progress, "0 / 0");
        lv_obj_add_flag(objects.result_label, LV_OBJ_FLAG_HIDDEN);
        return;
    }

    if (studyIsFinished()) {
        char buffer[67];
        snprintf(buffer, sizeof(buffer), "Знаю: %d / %d", knownCount, total);
        lv_label_set_text(objects.result_label, buffer);
        lv_obj_clear_flag(objects.result_label, LV_OBJ_FLAG_HIDDEN);
        lv_label_set_text(objects.card_label, "Деку пройдено!");
        return;
    }

    lv_obj_add_flag(objects.result_label, LV_OBJ_FLAG_HIDDEN);

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

void studyButtonsPoll(void) {
    bool currLeft = (gpio_get_level(BTN_LEFT_PIN) == 0);
    bool currMid = (gpio_get_level(BTN_MID_PIN) == 0);
    bool currRight = (gpio_get_level(BTN_RIGHT_PIN) == 0);
    bool leftPressed = (currLeft && !previousLeft);
    bool rightPressed = (currRight && !previousRight);
    bool midPressed = (currMid && !previousMid);

    previousLeft = currLeft;
    previousMid = currMid;
    previousRight = currRight;

    if (total <= 0 || busy || studyIsFinished()) return;

    if (midPressed == true) {
        flip();
    }
    else if (rightPressed) {
        studyAnswer(true);
    }
    else if (leftPressed) {
        studyAnswer(false);
    }

}

