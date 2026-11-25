#ifndef WEB_H
#define WEB_H

#define MAX_DECKS 16
#define DECK_DIR  "/sdcard/decks"

void connectWifi();

void serverInit();

void toggleWeb(void);

#endif