#include "world_map_info.h"

#include "gfx.h"
#include "text.h"
#include "util.h"
#include "z64.h"

#define PAUSE_MAP 1

world_map_area_entry_t world_map_areas[] = {
    {21, 0, {DESERT_COLOSSUS}, "Waste/Col"},                                                                                  // 4 tokens, 1 HPs
    {20, 0, {GERUDOS_FORTRESS}, "Fortress"},                                                                                  // 2 tokens, 2 HPs
    {19, 0, {GERUDO_VALLEY}, "Valley"},                                                                                       // 4 tokens, 2 HPs
    {18, 0, {LAKESIDE_LABORATORY, FISHING_POND, LAKE_HYLIA}, "Lake"},                                                         // 5 tokens, 3 HPs
    {11, 0, {RANCH_HOUSE_SILO}, "Ranch"},                                                                                     // 4 tokens, 1 HPs
    {14, 0, {TREASURE_BOX_SHOP, BOMBCHU_BOWLING_ALLEY, DOG_LADY_HOUSE}, "Market"},                                            // 4 tokens, 3 HPs
    {10, 0, {GROTTOS}, "Field"},                                                                                              // 2 tokens, 2 HPs
    {15, 0, {DEATH_MOUNTAIN_TRAIL, DEATH_MOUNTAIN_CRATER, GORON_CITY}, "Death Mtn"},                                          // 8 tokens, 4 HPs
    {16, 0, {IMPAS_HOUSE, REDEAD_GRAVE, DAMPES_GRAVE_WINDMILL, HOUSE_OF_SKULLTULA, KAKARIKO_VILLAGE, GRAVEYARD}, "Kak Area"}, // 8 tokens, 8 HPs
    {13, 0, {LOST_WOODS}, "Lost Wds"},                                                                                        // 4 tokens, 2 HPs
    {12, 0, {}, "Kokiri"},                                                                                                    // 3 tokens, 0 HPs
    {17, 0, {ZORAS_RIVER, ZORAS_DOMAIN, ZORAS_FOUNTAIN}, "Zora Area"},                                                        // 8 tokens, 7 HPs
};

int world_map_area_count = array_size(world_map_areas);

world_map_area_entry_t world_map_sub_areas[] = {
    {21, 1, {}, "Wasteland"},                                                                        // 1 tokens, 0 HPs
    {21, 1, {DESERT_COLOSSUS}, "Colossus"},                                                          // 3 tokens, 1 HPs
    {15, 1, {DEATH_MOUNTAIN_TRAIL}, "Trail"},                                                        // 4 tokens, 1 HPs
    {15, 1, {DEATH_MOUNTAIN_CRATER}, "Crater"},                                                      // 2 tokens, 2 HPs
    {15, 1, {GORON_CITY}, "Goron Cty"},                                                              // 2 tokens, 1 HPs
    {16, 0, {IMPAS_HOUSE, DAMPES_GRAVE_WINDMILL, HOUSE_OF_SKULLTULA, KAKARIKO_VILLAGE}, "Kakariko"}, // 8 tokens, 8 HPs
    {16, 0, {REDEAD_GRAVE, DAMPES_GRAVE_WINDMILL, GRAVEYARD}, "Graveyard"},                          // 8 tokens, 8 HPs
    {17, 1, {ZORAS_RIVER}, "River"},                                                                 // 4 tokens, 4 HPs
    {17, 1, {ZORAS_DOMAIN}, "Domain"},                                                               // 1 tokens, 1 HPs
    {17, 1, {ZORAS_FOUNTAIN}, "Fountain"},                                                           // 3 tokens, 2 HPs
};

int world_map_sub_area_count = array_size(world_map_sub_areas);

extern uint32_t CFG_WORLD_MAP_INFO_ENABLE;

uint32_t gGsFlagsMasks[] = {0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000};
uint32_t gGsFlagsShifts[] = {0, 8, 16, 24};

// gs_flags at 8011B46C
#define GET_GS_FLAGS(area_index) \
    ((z64_file.gs_flags[(area_index) >> 2] & gGsFlagsMasks[(area_index)&3]) >> gGsFlagsShifts[(area_index)&3])

int8_t get_tokens(uint8_t area_index)
{
    int8_t tokens = 0;
    int32_t token_flags = GET_GS_FLAGS(area_index);
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

    int areas_center_x = Z64_SCREEN_WIDTH / 4;
    draw_world_map_areas(db, world_map_areas, world_map_area_count, areas_center_x);

    int sub_areas_center_x = Z64_SCREEN_WIDTH * 3 / 4;
    draw_world_map_areas(db, world_map_sub_areas, world_map_sub_area_count, sub_areas_center_x);

    // Finish

    text_flush(db);

    gDPFullSync(db->p++);
    gSPEndDisplayList(db->p++);
}

void draw_world_map_areas(z64_disp_buf_t *db, world_map_area_entry_t *areas, int area_count, int center_x)
{
    // Call setup display list

    gSPDisplayList(db->p++, &setup_db);

    // Set up dimensions

    int icon_size = 16;
    int padding = 1;
    int bg_width =
        padding +
        (9 * font_sprite.tile_w) + // world map area names
        padding +
        (4 * (icon_size + padding)); // skull, skull count, hp, hp count
    int bg_height = padding + (area_count * (icon_size + padding));
    int bg_left = center_x - (bg_width / 2);
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

    for (int i = 0; i < area_count; i++)
    {
        world_map_area_entry_t *w = &(areas[i]);
        int top = start_top + ((icon_size + padding) * i) + 1;
        if (w->is_sub_area)
        {
            text_print(w->name, left, top);
        }
        else
        {
            text_print(w->name, left, top);
        }
    }

    left += (9 * font_sprite.tile_w) + padding;

    // Draw skull sprite

    sprite_load(db, &quest_items_sprite, 11, 1);

    for (int i = 0; i < area_count; i++)
    {
        int top = start_top + ((icon_size + padding) * i);
        sprite_draw(db, &quest_items_sprite, 0,
                    left, top, icon_size, icon_size);
    }

    left += icon_size + padding;

    // Draw skull count

    for (int i = 0; i < area_count; i++)
    {
        world_map_area_entry_t *w = &(areas[i]);

        int8_t tokens = get_tokens(w->area_index);

        char count[2] = "0";
        count[0] += (tokens % 10);
        int top = start_top + ((icon_size + padding) * i) + 1;
        if (w->is_sub_area)
        {
            text_print("-", left, top);
        }
        else
        {
            text_print(count, left, top);
        }
    }

    left += icon_size + padding;

    // Draw hp sprite

    sprite_load(db, &quest_items_sprite, 13, 1);

    for (int i = 0; i < area_count; i++)
    {
        int top = start_top + ((icon_size + padding) * i);
        sprite_draw(db, &quest_items_sprite, 0,
                    left, top, icon_size, icon_size);
    }

    left += icon_size + padding;

    // Draw small hp counts

    for (int i = 0; i < area_count; i++)
    {
        world_map_area_entry_t *w = &(areas[i]);

        int8_t hps = 1; // z64_file.world_map_area_keys[w->area_index];

        char count[2] = "0";
        count[0] += (hps % 10);
        int top = start_top + ((icon_size + padding) * i) + 1;
        if (w->is_sub_area)
        {
            text_print(count, left, top);
        }
        else
        {
            text_print("-", left, top);
        }

        text_print(count, left, top);
    }

    left += icon_size + padding;
}