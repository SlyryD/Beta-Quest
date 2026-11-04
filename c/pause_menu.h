#ifndef PAUSE_MENU_H
#define PAUSE_MENU_H

#include <stdbool.h>
#include <stdint.h>

#include "gfx.h"
#include "z64.h"

typedef enum {
    PAUSE_ITEM_MENU_NONE,
    PAUSE_ITEM_MENU_DUNGEONS,
    PAUSE_ITEM_MENU_COUNTS,
    PAUSE_ITEM_MENU_AREAS,
    PAUSE_ITEM_MENU_EVENTS,
    PAUSE_ITEM_MENU_GEAR,
} pause_item_menu_t;

pause_item_menu_t pause_item_menu_active(void);

void pause_menu_begin(z64_disp_buf_t* db, int left, int top, int width, int height, uint8_t alpha);
void pause_menu_end(z64_disp_buf_t* db);
void pause_menu_format_value(char text[4], uint16_t value);
int pause_menu_text_width(const char* text);
void pause_menu_draw_sprite(z64_disp_buf_t* db, sprite_t* sprite, int tile, int left, int top, int width, int height,
                            bool active, uint8_t red, uint8_t green, uint8_t blue);

#endif
