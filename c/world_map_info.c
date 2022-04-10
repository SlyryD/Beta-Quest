#include "world_map_info.h"

#include "gfx.h"
#include "text.h"
#include "util.h"
#include "z64.h"

#define PAUSE_MAP 1

#define COLOSSUS_FREESTANDING_HP             \
    {                                        \
        0, 1, 0, 0, 0, DESERT_COLOSSUS, 0x0D \
    }
#define GF_CHEST                              \
    {                                         \
        1, 0, 0, 0, 0, GERUDOS_FORTRESS, 0x00 \
    }
#define GF_HBA_1000_POINTS                    \
    {                                         \
        0, 0, 1, 0, 0, GERUDOS_FORTRESS, 0x3E \
    }
#define GV_CRATE_FREESTANDING_HP           \
    {                                      \
        0, 1, 0, 0, 0, GERUDO_VALLEY, 0x02 \
    }
#define GV_WATERFALL_FREESTANDING_HP       \
    {                                      \
        0, 1, 0, 0, 0, GERUDO_VALLEY, 0x01 \
    }
#define LH_CHILD_FISHING                  \
    {                                     \
        0, 0, 1, 0, 0, FISHING_POND, 0x3E \
    }
#define LH_LAB_DIVE                              \
    {                                            \
        0, 0, 1, 0, 0, LAKESIDE_LABORATORY, 0x3E \
    }
#define LH_FREESTANDING_HP              \
    {                                   \
        0, 1, 0, 0, 0, LAKE_HYLIA, 0x1E \
    }
#define LLR_FREESTANDING_HP                   \
    {                                         \
        0, 1, 0, 0, 0, RANCH_HOUSE_SILO, 0x01 \
    }
#define MARKET_BOMBCHU_BOWLING_SECOND_PRIZE        \
    {                                              \
        0, 0, 1, 0, 0, BOMBCHU_BOWLING_ALLEY, 0x3E \
    }
#define MARKET_LOST_DOG                     \
    {                                       \
        0, 0, 1, 0, 0, DOG_LADY_HOUSE, 0x3E \
    }
#define MARKET_TREASURE_CHEST_GAME_REWARD      \
    {                                          \
        1, 0, 0, 0, 0, TREASURE_BOX_SHOP, 0x0A \
    }
#define HF_TEKTITE_GROTTO_FREESTANDING_HP \
    {                                     \
        0, 1, 0, 0, 0, GROTTOS, 0x01      \
    }
#define HF_DEKU_SCRUB_GROTTO         \
    {                                \
        0, 0, 0, 0, 0, GROTTOS, 0x3E \
    }
#define DMT_FREESTANDING_HP                       \
    {                                             \
        0, 1, 0, 0, 0, DEATH_MOUNTAIN_TRAIL, 0x1E \
    }
#define DMC_VOLCANO_FREESTANDING_HP                \
    {                                              \
        0, 1, 0, 0, 0, DEATH_MOUNTAIN_CRATER, 0x08 \
    }
#define DMC_WALL_FREESTANDING_HP                   \
    {                                              \
        0, 1, 0, 0, 0, DEATH_MOUNTAIN_CRATER, 0x02 \
    }
#define GC_POT_FREESTANDING_HP          \
    {                                   \
        0, 1, 0, 0, 0, GORON_CITY, 0x1F \
    }
#define KAK_IMPAS_HOUSE_FREESTANDING_HP  \
    {                                    \
        0, 1, 0, 0, 0, IMPAS_HOUSE, 0x01 \
    }
#define KAK_WINDMILL_FREESTANDING_HP               \
    {                                              \
        0, 1, 0, 0, 0, DAMPES_GRAVE_WINDMILL, 0x01 \
    }
#define KAK_MAN_ON_ROOF                       \
    {                                         \
        0, 0, 1, 0, 0, KAKARIKO_VILLAGE, 0x3E \
    }
#define KAK_50_GOLD_SKULLTULA_REWARD            \
    {                                           \
        0, 0, 1, 0, 0, HOUSE_OF_SKULLTULA, 0x3E \
    }
#define GRAVEYARD_HEART_PIECE_GRAVE_CHEST \
    {                                     \
        1, 0, 0, 0, 0, REDEAD_GRAVE, 0x00 \
    }
#define GRAVEYARD_FREESTANDING_HP      \
    {                                  \
        0, 1, 0, 0, 0, GRAVEYARD, 0x04 \
    }
#define GRAVEYARD_DAMPE_GRAVEDIGGING_TOUR \
    {                                     \
        0, 1, 0, 0, 0, GRAVEYARD, 0x08    \
    }
#define GRAVEYARD_DAMPE_RACE_FREESTANDING_HP       \
    {                                              \
        0, 1, 0, 0, 0, DAMPES_GRAVE_WINDMILL, 0x07 \
    }
#define LW_OCARINA_MEMORY_GAME          \
    {                                   \
        0, 0, 1, 0, 0, LOST_WOODS, 0x76 \
    }
#define LW_SKULL_KID                    \
    {                                   \
        0, 0, 1, 0, 0, LOST_WOODS, 0x3E \
    }
#define ZR_FROGS_IN_THE_RAIN             \
    {                                    \
        0, 0, 1, 0, 0, ZORAS_RIVER, 0x3E \
    }
#define ZR_FROGS_OCARINA_GAME            \
    {                                    \
        0, 0, 1, 0, 0, ZORAS_RIVER, 0x76 \
    }
#define ZR_NEAR_OPEN_GROTTO_FREESTANDING_HP \
    {                                       \
        0, 1, 0, 0, 0, ZORAS_RIVER, 0x04    \
    }
#define ZR_NEAR_DOMAIN_FREESTANDING_HP   \
    {                                    \
        0, 1, 0, 0, 0, ZORAS_RIVER, 0x0B \
    }
#define ZD_CHEST                          \
    {                                     \
        1, 0, 0, 0, 0, ZORAS_DOMAIN, 0x00 \
    }
#define ZF_ICEBERG_FREESTANDING_HP          \
    {                                       \
        0, 1, 0, 0, 0, ZORAS_FOUNTAIN, 0x01 \
    }
#define ZF_BOTTOM_FREESTANDING_HP           \
    {                                       \
        0, 1, 0, 0, 0, ZORAS_FOUNTAIN, 0x14 \
    }

// Waste/Col Area Skulls
// GF Area Skulls
// GV Area HPs, Skulls
// Lake Area Skulls
// LLR Area Skulls
// HF Area Skulls
// DM Area Skulls
// DMC HPs
// Kak Area Skulls
// LW Area HPs, Skulls
// KF Area Skulls
// Zora Area HPs, Skulls

/**
 * Tests if current scene chest flag is set.
 */
int32_t Flags_GetTreasure(z64_game_t *globalCtx, uint8_t scene_index, int32_t flag)
{
    return z64_file.scene_flags[scene_index].chest & (1 << flag);
}

// Flags_GetCollectible
// Flags_GetEventChkInf

/**
 * Tests if event_chk_inf flag is set.
 */
int32_t Flags_GetItemGetInf(int32_t flag)
{
    return z64_file.item_get_inf[flag >> 4] & (1 << (flag & 0xF));
}

// Flags_GetInfTable

typedef struct
{
    struct
    {
        uint8_t is_chest : 1;
        uint8_t is_collect : 1;
        uint8_t is_event_chk_inf : 1;
        uint8_t is_item_get_inf : 1;
        uint8_t is_inf_table : 1;
    };
    uint8_t scene;
    int32_t flag;
} flag_info_t;

typedef struct
{
    uint8_t area_index;
    flag_info_t hp_flags[8];
    char name[15];
} world_map_area_entry_t;

world_map_area_entry_t world_map_areas[] = {
    {21, {COLOSSUS_FREESTANDING_HP}, "Wasteland Area"},                                                                                                                                                                                                                           // 4 tokens, 1 HPs
    {20, {GF_CHEST, GF_HBA_1000_POINTS}, "Fortress Area"},                                                                                                                                                                                                                        // 2 tokens, 2 HPs
    {19, {GV_CRATE_FREESTANDING_HP, GV_WATERFALL_FREESTANDING_HP}, "Gerudo Valley"},                                                                                                                                                                                              // 4 tokens, 2 HPs
    {18, {LH_CHILD_FISHING, LH_LAB_DIVE, LH_FREESTANDING_HP}, "Lake Hylia"},                                                                                                                                                                                                      // 5 tokens, 3 HPs
    {11, {LLR_FREESTANDING_HP}, "Lon Lon Ranch"},                                                                                                                                                                                                                                 // 4 tokens, 1 HPs
    {14, {MARKET_BOMBCHU_BOWLING_SECOND_PRIZE, MARKET_LOST_DOG, MARKET_TREASURE_CHEST_GAME_REWARD}, "Market Area"},                                                                                                                                                               // 4 tokens, 2 HPs
    {10, {HF_TEKTITE_GROTTO_FREESTANDING_HP, HF_DEKU_SCRUB_GROTTO}, "Hyrule Field"},                                                                                                                                                                                              // 2 tokens, 2 HPs
    {15, {DMT_FREESTANDING_HP, DMC_VOLCANO_FREESTANDING_HP, DMC_WALL_FREESTANDING_HP, GC_POT_FREESTANDING_HP}, "Death Mountain"},                                                                                                                                                 // 8 tokens, 4 HPs
    {16, {KAK_IMPAS_HOUSE_FREESTANDING_HP, KAK_WINDMILL_FREESTANDING_HP, KAK_MAN_ON_ROOF, KAK_50_GOLD_SKULLTULA_REWARD, GRAVEYARD_HEART_PIECE_GRAVE_CHEST, GRAVEYARD_FREESTANDING_HP, GRAVEYARD_DAMPE_GRAVEDIGGING_TOUR, GRAVEYARD_DAMPE_RACE_FREESTANDING_HP}, "Kakariko Area"}, // 8 tokens, 8 HPs
    {13, {LW_OCARINA_MEMORY_GAME, LW_SKULL_KID}, "Lost Woods"},                                                                                                                                                                                                                   // 4 tokens, 2 HPs
    {12, {}, "Kokiri Forest"},                                                                                                                                                                                                                                                    // 3 tokens, 0 HPs
    {17, {ZR_FROGS_IN_THE_RAIN, ZR_FROGS_OCARINA_GAME, ZR_NEAR_OPEN_GROTTO_FREESTANDING_HP, ZR_NEAR_DOMAIN_FREESTANDING_HP, ZD_CHEST, ZF_ICEBERG_FREESTANDING_HP, ZF_BOTTOM_FREESTANDING_HP}, "Zora Area"},                                                                       // 8 tokens, 7 HPs
};

int world_map_area_count = array_size(world_map_areas);

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

        int8_t tokens = get_tokens(w->area_index);

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

        int8_t hps = 1; // z64_file.world_map_area_keys[w->area_index];

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
