#include <string.h>

#include "screens.h"
#include "images.h"
#include "fonts.h"
#include "actions.h"
#include "vars.h"
#include "styles.h"
#include "ui.h"

#include <string.h>

objects_t objects;

//
// Event handlers
//

lv_obj_t *tick_value_change_obj;

//
// Screens
//

void create_screen_main() {
    lv_obj_t *obj = lv_obj_create(0);
    objects.main = obj;
    lv_obj_set_pos(obj, 0, 0);
    lv_obj_set_size(obj, 240, 320);
    lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(obj, lv_color_hex(0x6b2862), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(obj, lv_color_hex(0x3a5278), LV_PART_MAIN | LV_STATE_DEFAULT);
    {
        lv_obj_t *parent_obj = obj;
        {
            // progress
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.progress = obj;
            lv_obj_set_pos(obj, 106, 25);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xda99d1), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "0/0");
        }
        {
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.obj0 = obj;
            lv_obj_set_pos(obj, 10, 58);
            lv_obj_set_size(obj, LV_PCT(92), LV_PCT(64));
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_CLICKABLE);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xda99d1), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_shadow_color(obj, lv_color_hex(0x843179), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_shadow_width(obj, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_shadow_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_left(obj, 12, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 12, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_clip_corner(obj, true, LV_PART_MAIN | LV_STATE_DEFAULT);
            {
                lv_obj_t *parent_obj = obj;
                {
                    // card_label
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    objects.card_label = obj;
                    lv_obj_set_pos(obj, 0, 86);
                    lv_obj_set_size(obj, LV_PCT(100), LV_PCT(16));
                    lv_obj_set_style_text_font(obj, &lv_font_montserrat_16, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_color(obj, lv_color_hex(0x843179), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text(obj, "card");
                }
            }
        }
        {
            // result_label
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.result_label = obj;
            lv_obj_set_pos(obj, 97, 122);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_add_flag(obj, LV_OBJ_FLAG_HIDDEN);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_20, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xda99d1), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "Text");
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.obj1 = obj;
            lv_obj_set_pos(obj, 10, 275);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xd13f38), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "DON'T \nKNOW");
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.obj2 = obj;
            lv_obj_set_pos(obj, 182, 283);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x6fd14d), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "KNOW");
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.obj3 = obj;
            lv_obj_set_pos(obj, 94, 283);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xd7d9ad), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "SHOW");
        }
    }
    
    tick_screen_main();
}

void tick_screen_main() {
}

void create_screen_decks() {
    lv_obj_t *obj = lv_obj_create(0);
    objects.decks = obj;
    lv_obj_set_pos(obj, 0, 0);
    lv_obj_set_size(obj, 240, 320);
    lv_obj_set_style_bg_color(obj, lv_color_hex(0x6b2862), LV_PART_MAIN | LV_STATE_DEFAULT);
    {
        lv_obj_t *parent_obj = obj;
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.obj4 = obj;
            lv_obj_set_pos(obj, 85, 14);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xda99d1), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_20, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "DECKS");
        }
        {
            // wifi_label
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.wifi_label = obj;
            lv_obj_set_pos(obj, 84, 36);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_label_set_text(obj, "Web: OFF");
        }
        {
            // decks_roller
            lv_obj_t *obj = lv_roller_create(parent_obj);
            objects.decks_roller = obj;
            lv_obj_set_pos(obj, 10, 58);
            lv_obj_set_size(obj, LV_PCT(92), LV_PCT(64));
            lv_roller_set_options(obj, "no decks", LV_ROLLER_MODE_NORMAL);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x843179), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xda99d1), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_color(obj, lv_color_hex(0x6b2862), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_shadow_width(obj, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_shadow_color(obj, lv_color_hex(0x843179), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xd14dc0), LV_PART_SELECTED | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x6fd14d), LV_PART_SELECTED | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_14, LV_PART_SELECTED | LV_STATE_DEFAULT);
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.obj5 = obj;
            lv_obj_set_pos(obj, 22, 283);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_16, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xda99d1), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "UP");
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.obj6 = obj;
            lv_obj_set_pos(obj, 85, 283);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_16, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xa9da99), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "OPEN");
        }
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.obj7 = obj;
            lv_obj_set_pos(obj, 173, 283);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_16, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xda99d1), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "DOWN");
        }
    }
    
    tick_screen_decks();
}

void tick_screen_decks() {
}

typedef void (*tick_screen_func_t)();
tick_screen_func_t tick_screen_funcs[] = {
    tick_screen_main,
    tick_screen_decks,
};
void tick_screen(int screen_index) {
    if (screen_index >= 0 && screen_index < 2) {
        tick_screen_funcs[screen_index]();
    }
}
void tick_screen_by_id(enum ScreensEnum screenId) {
    tick_screen(screenId - 1);
}

//
// Fonts
//

ext_font_desc_t fonts[] = {
#if LV_FONT_MONTSERRAT_8
    { "MONTSERRAT_8", &lv_font_montserrat_8 },
#endif
#if LV_FONT_MONTSERRAT_10
    { "MONTSERRAT_10", &lv_font_montserrat_10 },
#endif
#if LV_FONT_MONTSERRAT_12
    { "MONTSERRAT_12", &lv_font_montserrat_12 },
#endif
#if LV_FONT_MONTSERRAT_14
    { "MONTSERRAT_14", &lv_font_montserrat_14 },
#endif
#if LV_FONT_MONTSERRAT_16
    { "MONTSERRAT_16", &lv_font_montserrat_16 },
#endif
#if LV_FONT_MONTSERRAT_18
    { "MONTSERRAT_18", &lv_font_montserrat_18 },
#endif
#if LV_FONT_MONTSERRAT_20
    { "MONTSERRAT_20", &lv_font_montserrat_20 },
#endif
#if LV_FONT_MONTSERRAT_22
    { "MONTSERRAT_22", &lv_font_montserrat_22 },
#endif
#if LV_FONT_MONTSERRAT_24
    { "MONTSERRAT_24", &lv_font_montserrat_24 },
#endif
#if LV_FONT_MONTSERRAT_26
    { "MONTSERRAT_26", &lv_font_montserrat_26 },
#endif
#if LV_FONT_MONTSERRAT_28
    { "MONTSERRAT_28", &lv_font_montserrat_28 },
#endif
#if LV_FONT_MONTSERRAT_30
    { "MONTSERRAT_30", &lv_font_montserrat_30 },
#endif
#if LV_FONT_MONTSERRAT_32
    { "MONTSERRAT_32", &lv_font_montserrat_32 },
#endif
#if LV_FONT_MONTSERRAT_34
    { "MONTSERRAT_34", &lv_font_montserrat_34 },
#endif
#if LV_FONT_MONTSERRAT_36
    { "MONTSERRAT_36", &lv_font_montserrat_36 },
#endif
#if LV_FONT_MONTSERRAT_38
    { "MONTSERRAT_38", &lv_font_montserrat_38 },
#endif
#if LV_FONT_MONTSERRAT_40
    { "MONTSERRAT_40", &lv_font_montserrat_40 },
#endif
#if LV_FONT_MONTSERRAT_42
    { "MONTSERRAT_42", &lv_font_montserrat_42 },
#endif
#if LV_FONT_MONTSERRAT_44
    { "MONTSERRAT_44", &lv_font_montserrat_44 },
#endif
#if LV_FONT_MONTSERRAT_46
    { "MONTSERRAT_46", &lv_font_montserrat_46 },
#endif
#if LV_FONT_MONTSERRAT_48
    { "MONTSERRAT_48", &lv_font_montserrat_48 },
#endif
};

//
// Color themes
//

uint32_t active_theme_index = 0;

//
//
//

void create_screens() {

// Set default LVGL theme
    lv_display_t *dispp = lv_display_get_default();
    lv_theme_t *theme = lv_theme_default_init(dispp, lv_palette_main(LV_PALETTE_BLUE), lv_palette_main(LV_PALETTE_RED), true, LV_FONT_DEFAULT);
    lv_display_set_theme(dispp, theme);
    
    // Initialize screens
    // Create screens
    create_screen_main();
    create_screen_decks();
}