#include "pause_menu.h"

#include "text.h"

#define PAUSE_ITEM 0

pause_item_menu_t pause_item_menu_active(void) {
    if (z64_game.pause_ctxt.state != 6 || z64_game.pause_ctxt.screen_idx != PAUSE_ITEM ||
        z64_game.pause_ctxt.changing || !z64_ctxt.input[0].raw.pad.a) {
        return PAUSE_ITEM_MENU_NONE;
    }

    bool left = z64_ctxt.input[0].raw.pad.dl;
    bool right = z64_ctxt.input[0].raw.pad.dr;
    bool up = z64_ctxt.input[0].raw.pad.du;
    bool down = z64_ctxt.input[0].raw.pad.dd;
    uint8_t direction_count = left + right + up + down;

    if (direction_count == 0) {
        return PAUSE_ITEM_MENU_DUNGEONS;
    }
    if (direction_count != 1) {
        return PAUSE_ITEM_MENU_NONE;
    }
    if (left) {
        return PAUSE_ITEM_MENU_COUNTS;
    }
    if (right) {
        return PAUSE_ITEM_MENU_AREAS;
    }
    if (up) {
        return PAUSE_ITEM_MENU_EVENTS;
    }
    return PAUSE_ITEM_MENU_GEAR;
}

void pause_menu_begin(z64_disp_buf_t* db, int left, int top, int width, int height, uint8_t alpha) {
    db->p = db->buf;
    gSPDisplayList(db->p++, &setup_db);

    gDPSetCombineMode(db->p++, G_CC_PRIMITIVE, G_CC_PRIMITIVE);
    gDPSetPrimColor(db->p++, 0, 0, 0, 0, 0, alpha);
    gSPTextureRectangle(db->p++, left << 2, top << 2, (left + width) << 2, (top + height) << 2, 0, 0, 0, 1 << 10,
                        1 << 10);

    gDPPipeSync(db->p++);
    gDPSetCombineMode(db->p++, G_CC_MODULATEIA_PRIM, G_CC_MODULATEIA_PRIM);
    gDPSetPrimColor(db->p++, 0, 0, 0xFF, 0xFF, 0xFF, 0xFF);
}

void pause_menu_end(z64_disp_buf_t* db) {
    gDPPipeSync(db->p++);
    gDPSetPrimColor(db->p++, 0, 0, 0xFF, 0xFF, 0xFF, 0xFF);
    text_flush(db);
    gDPFullSync(db->p++);
    gSPEndDisplayList(db->p++);
}

void pause_menu_format_value(char text[4], uint16_t value) {
    if (value >= 100) {
        text[0] = '0' + (value / 100) % 10;
        text[1] = '0' + (value / 10) % 10;
        text[2] = '0' + value % 10;
        text[3] = '\0';
    } else if (value >= 10) {
        text[0] = '0' + value / 10;
        text[1] = '0' + value % 10;
        text[2] = '\0';
    } else {
        text[0] = '0' + value;
        text[1] = '\0';
    }
}

int pause_menu_text_width(const char* text) {
    int length = 0;
    while (text[length] != '\0') {
        length++;
    }
    return length * font_sprite.tile_w;
}

void pause_menu_draw_sprite(z64_disp_buf_t* db, sprite_t* sprite, int tile, int left, int top, int width, int height,
                            bool active, uint8_t red, uint8_t green, uint8_t blue) {
    gDPPipeSync(db->p++);
    gDPSetCombineMode(db->p++, G_CC_MODULATEIA_PRIM, G_CC_MODULATEIA_PRIM);
    gDPSetPrimColor(db->p++, 0, 0, red, green, blue, active ? 0xFF : 0x40);
    sprite_load(db, sprite, tile, 1);
    sprite_draw(db, sprite, 0, left, top, width, height);
}
