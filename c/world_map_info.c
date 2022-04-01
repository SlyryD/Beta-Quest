#include "world_map_info.h"

#include "gfx.h"
#include "text.h"
#include "util.h"
#include "z64.h"

static uint32_t frames = 0;
#define FRAMES_PER_CYCLE 2
#define TOKEN_SPRITE_FRAMES 16
#define TOKEN_FRAMES_VISIBLE 100 // 20 Frames is about 1 second
#define TOKEN_FRAMES_FADE_AWAY 80
#define TOKEN_FRAMES_FADE_INTO 5

#define PAUSE_MAP 1

typedef struct
{
    uint8_t index;
    char name[10];
} world_map_area_entry_t;

world_map_area_entry_t world_map_areas[] = {
    {21, ""},
    {20, ""},
    {19, ""},
    {18, ""},
    {11, ""},
    {14, ""},
    {10, ""},
    {15, ""},
    {16, ""},
    {13, ""},
    {12, ""},
    {17, ""},
};

int world_map_area_count = array_size(world_map_areas);

extern uint32_t CFG_WORLD_MAP_INFO_ENABLE;

uint32_t gGsFlagsMasks[] = {0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000};
uint32_t gGsFlagsShifts[] = {0, 8, 16, 24};

void draw_world_map_info(z64_disp_buf_t *db)
{
    int draw = CFG_WORLD_MAP_INFO_ENABLE &&
               CAN_DRAW_TOKEN &&
               z64_game.pause_ctxt.state == 6 &&
               z64_game.pause_ctxt.screen_idx == PAUSE_MAP &&
               !z64_game.pause_ctxt.changing;
    if (!draw)
    {
        return;
    }

    // Not a dungeon scene
    switch (z64_game.scene_index)
    {
    case 0:
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
    case 6:
    case 7:
    case 8:
    case 9:
    case 17:
    case 18:
    case 19:
    case 20:
    case 21:
    case 22:
    case 23:
    case 24:
        return;
    }

    uint8_t alpha = 255;
    frames = frames % (TOKEN_SPRITE_FRAMES * FRAMES_PER_CYCLE);
    frames++;

    int8_t token_flags = GET_GS_FLAGS(world_map_areas[z64_game.pause_ctxt.map_cursor].index);
    int8_t tokens = 0;
    while (token_flags)
    {
        tokens += token_flags & 1;
        token_flags >>= 1;
    }

    // Setup draw location
    int total_w = font_sprite.tile_w + token_sprite.tile_w;
    int draw_x = Z64_SCREEN_WIDTH / 2 - total_w / 2;
    int draw_y_text = Z64_SCREEN_HEIGHT - (font_sprite.tile_h * 1.5) + 1;
    int draw_y_token = Z64_SCREEN_HEIGHT - (token_sprite.tile_h * 1.5) + 3 + 1;

    // Create collected string
    char text[2] = "0";
    text[0] += (tokens % 10);

    // Call setup display list
    gSPDisplayList(db->p++, &setup_db);
    gDPPipeSync(db->p++);
    gDPSetCombineMode(db->p++, G_CC_MODULATEIA_PRIM, G_CC_MODULATEIA_PRIM);
    gDPSetPrimColor(db->p++, 0, 0, 0xDA, 0xD3, 0x0B, alpha);

    text_print(text, draw_x, draw_y_text);
    draw_x += font_sprite.tile_w;

    gDPSetPrimColor(db->p++, 0, 0, 0xF4, 0xEC, 0x30, alpha);

    // Draw token
    int sprite = (frames / FRAMES_PER_CYCLE) % TOKEN_SPRITE_FRAMES;
    sprite_load(db, &token_sprite, sprite, 1);
    sprite_draw(db, &token_sprite, 0, draw_x, draw_y_token, token_sprite.tile_w, token_sprite.tile_h);

    text_flush(db);
    gDPFullSync(db->p++);
    gSPEndDisplayList(db->p++);
}
