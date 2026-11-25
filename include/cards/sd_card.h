#ifndef SD_CARD_H
#define SD_CARD_H

#include "esp_err.h"

#define MOUNT_POINT "/sdcard"

esp_err_t sdOpen(void);
void sdClose(void);

#endif