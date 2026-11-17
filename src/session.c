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
