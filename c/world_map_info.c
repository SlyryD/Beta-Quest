#include "world_map_info.h"

#include "gfx.h"
#include "text.h"
#include "util.h"
#include "z64.h"

#define PAUSE_MAP 1

typedef struct
{
    uint8_t index;
    char name[15];
} world_map_area_entry_t;

world_map_area_entry_t world_map_areas[] = {
    {21, "Wasteland"},
    {20, "Fortress"},
    {19, "Gerudo Valley"},
    {18, "Lake Hylia"},
    {11, "Lon Lon Ranch"},
    {14, "Market"},
    {10, "Hyrule Field"},
    {15, "Death Mountain"},
    {16, "Kakariko"},
    {13, "Lost Woods"},
    {12, "Kokiri Forest"},
    {17, "Zoras Domain"},
};

int world_map_area_count = array_size(world_map_areas);

extern uint32_t CFG_WORLD_MAP_INFO_ENABLE;

uint32_t gGsFlagsMasks[] = {0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000};
uint32_t gGsFlagsShifts[] = {0, 8, 16, 24};

// gs_flags at 8011B46C
#define GET_GS_FLAGS(index) \
    ((z64_file.gs_flags[(index) >> 2] & gGsFlagsMasks[(index)&3]) >> gGsFlagsShifts[(index)&3])

int8_t get_tokens(uint8_t index)
{
    int8_t tokens = 0;
    int32_t token_flags = GET_GS_FLAGS(index);
    while (token_flags)
    {
        tokens += token_flags & 1;
        token_flags >>= 1;
    }

    return tokens;
}

void draw_world_map_info(z64_disp_buf_t *db)
{
    int draw = CFG_WORLD_MAP_INFO_ENABLE &&
               z64_game.pause_ctxt.state == 6 &&
               z64_game.pause_ctxt.screen_idx == PAUSE_MAP &&
               !z64_game.pause_ctxt.changing &&
               z64_ctxt.input[0].raw.pad.a;
    if (!draw)
    {
        return;
    }

    // Not a dungeon scene
    switch (z64_game.scene_index)
    {
    case DEKU_TREE:
    case DODONGOS_CAVERN:
    case JABU_JABU:
    case FOREST_TEMPLE:
    case FIRE_TEMPLE:
    case WATER_TEMPLE:
    case SPIRIT_TEMPLE:
    case SHADOW_TEMPLE:
    case BOTTOM_OF_THE_WELL:
    case ICE_CAVERN:
    case GOHMA:
    case KING_DODONGO:
    case BARINADE:
    case PHANTOM_GANON:
    case VOLVAGIA:
    case MORPHA:
    case TWINROVA:
    case BONGO_BONGO:
        return;
    }

    db->p = db->buf;

    // Call setup display list

    gSPDisplayList(db->p++, &setup_db);

    // Set up dimensions

    int icon_size = 16;
    int padding = 1;
    int bg_width =
        padding +
        (14 * font_sprite.tile_w) + // world map area names
        padding +
        (4 * (icon_size + padding)); // skull, skull count, hp, hp count
    int bg_height = padding + (world_map_area_count * (icon_size + padding));
    int bg_left = (Z64_SCREEN_WIDTH - bg_width) / 2;
    int bg_top = (Z64_SCREEN_HEIGHT - bg_height) / 2;

    int left = bg_left + padding;
    int start_top = bg_top + padding;

    // Draw background

    gDPSetCombineMode(db->p++, G_CC_PRIMITIVE, G_CC_PRIMITIVE);
    gDPSetPrimColor(db->p++, 0, 0, 0x00, 0x00, 0x00, 0xD0);
    gSPTextureRectangle(db->p++,
                        bg_left << 2, bg_top << 2,
                        (bg_left + bg_width) << 2, (bg_top + bg_height) << 2,
                        0,
                        0, 0,
                        1 << 10, 1 << 10);

    // Set the primary color to white for drawing sprites

    gDPPipeSync(db->p++);
    gDPSetCombineMode(db->p++, G_CC_MODULATEIA_PRIM, G_CC_MODULATEIA_PRIM);

    gDPSetPrimColor(db->p++, 0, 0, 0xFF, 0xFF, 0xFF, 0xFF);

    // Draw area names

    for (int i = 0; i < world_map_area_count; i++)
    {
        world_map_area_entry_t *w = &(world_map_areas[i]);
        int top = start_top + ((icon_size + padding) * i) + 1;
        text_print(w->name, left, top);
    }

    left += (14 * font_sprite.tile_w) + padding;

    // Draw skull sprite

    sprite_load(db, &quest_items_sprite, 11, 1);

    for (int i = 0; i < world_map_area_count; i++)
    {
        int top = start_top + ((icon_size + padding) * i);
        sprite_draw(db, &quest_items_sprite, 0,
                    left, top, icon_size, icon_size);
    }

    left += icon_size + padding;

    // Draw skull count

    for (int i = 0; i < world_map_area_count; i++)
    {
        world_map_area_entry_t *w = &(world_map_areas[i]);

        int8_t tokens = get_tokens(w->index);

        char count[2] = "0";
        count[0] += (tokens % 10);
        int top = start_top + ((icon_size + padding) * i) + 1;
        text_print(count, left, top);
    }

    left += icon_size + padding;

    // Draw hp sprite

    sprite_load(db, &quest_items_sprite, 13, 1);

    for (int i = 0; i < world_map_area_count; i++)
    {
        int top = start_top + ((icon_size + padding) * i);
        sprite_draw(db, &quest_items_sprite, 0,
                    left, top, icon_size, icon_size);
    }

    left += icon_size + padding;

    // Draw small hp counts

    for (int i = 0; i < world_map_area_count; i++)
    {
        world_map_area_entry_t *w = &(world_map_areas[i]);

        int8_t hps = 1; // z64_file.world_map_area_keys[w->index];

        char count[2] = "0";
        count[0] += (hps % 10);
        int top = start_top + ((icon_size + padding) * i) + 1;
        text_print(count, left, top);
    }

    left += icon_size + padding;

    // Finish

    text_flush(db);

    gDPFullSync(db->p++);
    gSPEndDisplayList(db->p++);
}
