#include "world_map_info.h"

#include "gfx.h"
#include "text.h"
#include "util.h"
#include "z64.h"

#define PAUSE_MAP 1

uint8_t world_map_areas[] = {
    21, // HAUNTED_WASTELAND
    20, // GERUDOS_FORTRESS
    19, // GERUDO_VALLEY
    18, // LAKE_HYLIA
    11, // LON_LON_RANCH
    14, // MARKET
    10, // HYRULE_FIELD
    15, // DEATH_MOUNTAIN
    16, // KAKARIKO_VILLAGE
    13, // LOST_WOODS
    12, // KOKIRI_FOREST
    17, // ZORAS_DOMAIN
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
        (1 * (icon_size + padding)); // skull count, hp count, planted bean count, silver rupee rooms?
    int bg_height = padding + (1 * (icon_size + padding));
    int bg_left = (Z64_SCREEN_WIDTH - bg_width) / 2;
    int bg_top = Z64_SCREEN_HEIGHT - bg_height;

    int left = bg_left + padding;
    int top = bg_top;

    // Set the primary color to white for drawing sprites

    gDPPipeSync(db->p++);
    gDPSetCombineMode(db->p++, G_CC_MODULATEIA_PRIM, G_CC_MODULATEIA_PRIM);

    gDPSetPrimColor(db->p++, 0, 0, 0xFF, 0xFF, 0xFF, 0xFF);

    // Draw skull sprite

    sprite_load(db, &quest_items_sprite, 11, 1);
    sprite_draw(db, &quest_items_sprite, 0,
                left, top, icon_size, icon_size);

    left += icon_size + padding;

    // Draw skull count

    int8_t token_flags = GET_GS_FLAGS(world_map_areas[z64_game.pause_ctxt.map_cursor]);
    int8_t tokens = 0;
    while (token_flags)
    {
        tokens += token_flags & 1;
        token_flags >>= 1;
    }

    char count[2] = "0";
    count[0] += (tokens % 10);
    text_print(count, left, top + 1);

    // Finish

    text_flush(db);

    gDPFullSync(db->p++);
    gSPEndDisplayList(db->p++);
}

void set_world_map_points(z64_game_t *game)
{
    for (int i = 0; i < ARRAY_COUNT(game->pause_ctxt.world_map_points); i++)
    {
        game->pause_ctxt.world_map_points[i] = 1;
    }
}
