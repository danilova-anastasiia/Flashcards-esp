#ifndef EEZ_LVGL_UI_GUI_H
#define EEZ_LVGL_UI_GUI_H

#include <lvgl.h>

#include "screens.h"

#define FLASH_MS 300

#define CARD_BG_COLOR 0xf7c2d4
#define CARD_COLOR 0xCB6085
#define KNOWN_COLOR 0xcde874
#define UNKNOWN_COLOR 0xee383e

static lv_timer_t *flashTimer = NULL;


#ifdef __cplusplus
extern "C" {
#endif

void ui_init();
void ui_tick();

void loadScreen(enum ScreensEnum screenId);

#ifdef __cplusplus
}
#endif

#endif // EEZ_LVGL_UI_GUI_H