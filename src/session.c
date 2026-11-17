#include <stdbool.h>
#include <stdio.h>
#include "cards.h"
#include <stdlib.h>
#include <stdint.h>
#include "session.h"

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

//render()

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
