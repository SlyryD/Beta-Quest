#include "world_map_info.h"

#include "bingo.h"
#include "gfx.h"
#include "text.h"
#include "util.h"
#include "z64.h"

#define PAUSE_MAP 1

typedef enum {
    HS_HBA = 0x00, // horseback archery
    HS_POE_POINTS = 0x01,
    HS_FISHING = 0x02,
    HS_HORSE_RACE = 0x03,
    HS_MARATHON = 0x04,
    HS_UNK_05 = 0x05,
    HS_DAMPE_RACE = 0x06
} high_scores_t;

typedef struct {
    struct {
        uint8_t is_chest : 1;
        uint8_t is_collect : 1;
        uint8_t is_high_scores : 1;
        uint8_t is_event_chk_inf : 1;
        uint8_t is_item_get_inf : 1;
        uint8_t is_inf_table : 1;
    };
    uint8_t scene_index;
    int32_t flag;
} flag_info_t;

#define COLOSSUS_FREESTANDING_HP                      \
    {                                                 \
        0, 1, 0, 0, 0, 0, SCENE_DESERT_COLOSSUS, 0x0D \
    }
#define GF_CHEST                                       \
    {                                                  \
        1, 0, 0, 0, 0, 0, SCENE_GERUDOS_FORTRESS, 0x00 \
    }
#define GF_HBA_1000_POINTS                              \
    {                                                   \
        0, 0, 0, 0, 0, 1, SCENE_GERUDOS_FORTRESS, 0x190 \
    }
#define GF_CHILD_HP                                    \
    {                                                  \
        0, 1, 0, 0, 0, 0, SCENE_GERUDOS_FORTRESS, 0x01 \
    }
#define GV_CRATE_FREESTANDING_HP                    \
    {                                               \
        0, 1, 0, 0, 0, 0, SCENE_GERUDO_VALLEY, 0x02 \
    }
#define GV_WATERFALL_FREESTANDING_HP                \
    {                                               \
        0, 1, 0, 0, 0, 0, SCENE_GERUDO_VALLEY, 0x01 \
    }
#define LH_CHILD_FISHING                    \
    {                                       \
        0, 0, 1, 0, 0, 0, HS_FISHING, 0x400 \
    }
#define LH_LAB_DIVE                                       \
    {                                                     \
        0, 0, 0, 0, 1, 0, SCENE_LAKESIDE_LABORATORY, 0x10 \
    }
#define LH_FREESTANDING_HP                       \
    {                                            \
        0, 1, 0, 0, 0, 0, SCENE_LAKE_HYLIA, 0x1E \
    }
#define LLR_FREESTANDING_HP                            \
    {                                                  \
        0, 1, 0, 0, 0, 0, SCENE_RANCH_HOUSE_SILO, 0x01 \
    }
#define MARKET_BOMBCHU_BOWLING_SECOND_PRIZE                 \
    {                                                       \
        0, 0, 0, 0, 1, 0, SCENE_BOMBCHU_BOWLING_ALLEY, 0x12 \
    }
#define MARKET_LOST_DOG                               \
    {                                                 \
        0, 0, 0, 0, 0, 1, SCENE_DOG_LADY_HOUSE, 0x191 \
    }
#define MARKET_TREASURE_CHEST_GAME_REWARD               \
    {                                                   \
        1, 0, 0, 0, 0, 0, SCENE_TREASURE_BOX_SHOP, 0x0A \
    }
#define HF_TEKTITE_GROTTO_FREESTANDING_HP     \
    {                                         \
        0, 1, 0, 0, 0, 0, SCENE_GROTTOS, 0x01 \
    }
#define HF_DEKU_SCRUB_GROTTO                  \
    {                                         \
        0, 0, 0, 0, 1, 0, SCENE_GROTTOS, 0x0B \
    }
#define DMT_FREESTANDING_HP                                \
    {                                                      \
        0, 1, 0, 0, 0, 0, SCENE_DEATH_MOUNTAIN_TRAIL, 0x1E \
    }
#define DMC_VOLCANO_FREESTANDING_HP                         \
    {                                                       \
        0, 1, 0, 0, 0, 0, SCENE_DEATH_MOUNTAIN_CRATER, 0x08 \
    }
#define DMC_WALL_FREESTANDING_HP                            \
    {                                                       \
        0, 1, 0, 0, 0, 0, SCENE_DEATH_MOUNTAIN_CRATER, 0x02 \
    }
#define GC_POT_FREESTANDING_HP                   \
    {                                            \
        0, 1, 0, 0, 0, 0, SCENE_GORON_CITY, 0x1F \
    }
#define KAK_IMPAS_HOUSE_FREESTANDING_HP           \
    {                                             \
        0, 1, 0, 0, 0, 0, SCENE_IMPAS_HOUSE, 0x01 \
    }
#define KAK_WINDMILL_FREESTANDING_HP                        \
    {                                                       \
        0, 1, 0, 0, 0, 0, SCENE_DAMPES_GRAVE_WINDMILL, 0x01 \
    }
#define KAK_MAN_ON_ROOF                                \
    {                                                  \
        0, 0, 0, 0, 1, 0, SCENE_KAKARIKO_VILLAGE, 0x15 \
    }
#define KAK_50_GOLD_SKULLTULA_REWARD                     \
    {                                                    \
        0, 0, 0, 1, 0, 0, SCENE_HOUSE_OF_SKULLTULA, 0xDE \
    }
#define GRAVEYARD_HEART_PIECE_GRAVE_CHEST          \
    {                                              \
        1, 0, 0, 0, 0, 0, SCENE_REDEAD_GRAVE, 0x00 \
    }
#define GRAVEYARD_FREESTANDING_HP               \
    {                                           \
        0, 1, 0, 0, 0, 0, SCENE_GRAVEYARD, 0x04 \
    }
#define GRAVEYARD_DAMPE_GRAVEDIGGING_TOUR       \
    {                                           \
        0, 0, 0, 0, 1, 0, SCENE_GRAVEYARD, 0x1C \
    }
#define GRAVEYARD_DAMPE_RACE_FREESTANDING_HP                \
    {                                                       \
        0, 1, 0, 0, 0, 0, SCENE_DAMPES_GRAVE_WINDMILL, 0x07 \
    }
#define LW_OCARINA_MEMORY_GAME                   \
    {                                            \
        0, 0, 0, 0, 1, 0, SCENE_LOST_WOODS, 0x17 \
    }
#define LW_SKULL_KID                             \
    {                                            \
        0, 0, 0, 0, 1, 0, SCENE_LOST_WOODS, 0x16 \
    }
#define ZR_FROGS_IN_THE_RAIN                      \
    {                                             \
        0, 0, 0, 1, 0, 0, SCENE_ZORAS_RIVER, 0xD6 \
    }
#define ZR_FROGS_OCARINA_GAME                     \
    {                                             \
        0, 0, 0, 1, 0, 0, SCENE_ZORAS_RIVER, 0xD0 \
    }
#define ZR_NEAR_OPEN_GROTTO_FREESTANDING_HP       \
    {                                             \
        0, 1, 0, 0, 0, 0, SCENE_ZORAS_RIVER, 0x04 \
    }
#define ZR_NEAR_DOMAIN_FREESTANDING_HP            \
    {                                             \
        0, 1, 0, 0, 0, 0, SCENE_ZORAS_RIVER, 0x0B \
    }
#define ZD_CHEST                                   \
    {                                              \
        1, 0, 0, 0, 0, 0, SCENE_ZORAS_DOMAIN, 0x00 \
    }
#define ZF_ICEBERG_FREESTANDING_HP                   \
    {                                                \
        0, 1, 0, 0, 0, 0, SCENE_ZORAS_FOUNTAIN, 0x01 \
    }
#define ZF_BOTTOM_FREESTANDING_HP                    \
    {                                                \
        0, 1, 0, 0, 0, 0, SCENE_ZORAS_FOUNTAIN, 0x14 \
    }

typedef struct {
    uint8_t index;
    uint8_t gs_flags_index;
    flag_info_t hp_flags[8];
    int hp_flags_length;
    char name[15];
} world_map_area_entry_t;

world_map_area_entry_t world_map_areas[] = {
    {AREA_WASTELAND, 21, {COLOSSUS_FREESTANDING_HP}, 1, "Wasteland Area"},                                                                                                                                                                                                                          // 4 tokens, 1 HPs
    {AREA_FORTRESS, 20, {GF_CHEST, GF_HBA_1000_POINTS, GF_CHILD_HP}, 3, "Fortress Area"},                                                                                                                                                                                                           // 2 tokens, 3 HPs
    {AREA_GERUDO_VALLEY, 19, {GV_CRATE_FREESTANDING_HP, GV_WATERFALL_FREESTANDING_HP}, 2, "Gerudo Valley"},                                                                                                                                                                                         // 4 tokens, 2 HPs
    {AREA_LAKE_HYLIA, 18, {LH_CHILD_FISHING, LH_LAB_DIVE, LH_FREESTANDING_HP}, 3, "Lake Hylia"},                                                                                                                                                                                                    // 5 tokens, 3 HPs
    {AREA_LON_LON_RANCH, 11, {LLR_FREESTANDING_HP}, 1, "Lon Lon Ranch"},                                                                                                                                                                                                                            // 4 tokens, 1 HPs
    {AREA_MARKET, 14, {MARKET_BOMBCHU_BOWLING_SECOND_PRIZE, MARKET_LOST_DOG, MARKET_TREASURE_CHEST_GAME_REWARD}, 3, "Market Area"},                                                                                                                                                                 // 4 tokens, 2 HPs
    {AREA_HYRULE_FIELD, 10, {HF_TEKTITE_GROTTO_FREESTANDING_HP, HF_DEKU_SCRUB_GROTTO}, 2, "Hyrule Field"},                                                                                                                                                                                          // 2 tokens, 2 HPs
    {AREA_DEATH_MOUNTAIN, 15, {DMT_FREESTANDING_HP, DMC_VOLCANO_FREESTANDING_HP, DMC_WALL_FREESTANDING_HP, GC_POT_FREESTANDING_HP}, 4, "Death Mountain"},                                                                                                                                           // 8 tokens, 4 HPs
    {AREA_KAKARIKO, 16, {KAK_IMPAS_HOUSE_FREESTANDING_HP, KAK_WINDMILL_FREESTANDING_HP, KAK_MAN_ON_ROOF, KAK_50_GOLD_SKULLTULA_REWARD, GRAVEYARD_HEART_PIECE_GRAVE_CHEST, GRAVEYARD_FREESTANDING_HP, GRAVEYARD_DAMPE_GRAVEDIGGING_TOUR, GRAVEYARD_DAMPE_RACE_FREESTANDING_HP}, 8, "Kakariko Area"}, // 8 tokens, 8 HPs
    {AREA_LOST_WOODS, 13, {LW_OCARINA_MEMORY_GAME, LW_SKULL_KID}, 2, "Lost Woods"},                                                                                                                                                                                                                 // 4 tokens, 2 HPs
    {AREA_KOKIRI_FOREST, 12, {}, 0, "Kokiri Forest"},                                                                                                                                                                                                                                               // 3 tokens, 0 HPs
    {AREA_ZORA, 17, {ZR_FROGS_IN_THE_RAIN, ZR_FROGS_OCARINA_GAME, ZR_NEAR_OPEN_GROTTO_FREESTANDING_HP, ZR_NEAR_DOMAIN_FREESTANDING_HP, ZD_CHEST, ZF_ICEBERG_FREESTANDING_HP, ZF_BOTTOM_FREESTANDING_HP}, 7, "Zora Area"},                                                                           // 8 tokens, 7 HPs
};

int world_map_area_count = array_size(world_map_areas);

extern uint32_t CFG_WORLD_MAP_INFO_ENABLE;

#define HAS_CHEST(hp_flag) \
    (hp_flag.is_chest && ((z64_game.scene_index == hp_flag.scene_index && z64_game.chest_flags & (1 << hp_flag.flag)) || z64_file.scene_flags[hp_flag.scene_index].chest & (1 << hp_flag.flag)))

#define HAS_COLLECT(hp_flag) \
    (hp_flag.is_collect && ((z64_game.scene_index == hp_flag.scene_index && z64_game.collect_flags & (1 << hp_flag.flag)) || z64_file.scene_flags[hp_flag.scene_index].collect & (1 << hp_flag.flag)))

#define HAS_HIGH_SCORES(hp_flag) \
    (hp_flag.is_high_scores && z64_file.high_scores[hp_flag.scene_index] & hp_flag.flag)

#define HAS_EVENT_CHK_INF(hp_flag) \
    (hp_flag.is_event_chk_inf && z64_file.event_chk_inf[hp_flag.flag >> 4] & (1 << (hp_flag.flag & 0xF)))

#define HAS_ITEM_GET_INF(hp_flag) \
    (hp_flag.is_item_get_inf && z64_file.item_get_inf[hp_flag.flag >> 4] & (1 << (hp_flag.flag & 0xF)))

#define HAS_INF_TABLE(hp_flag) \
    (hp_flag.is_inf_table && z64_file.inf_table[hp_flag.flag >> 4] & (1 << (hp_flag.flag & 0xF)))

uint8_t get_area_tokens(uint8_t area_index) {
    uint8_t tokens = 0;
    world_map_area_entry_t area = world_map_areas[area_index];
    return get_tokens(area.gs_flags_index);
}

uint8_t get_area_hps(uint8_t area_index) {
    uint8_t hps = 0;
    world_map_area_entry_t area = world_map_areas[area_index];
    for (int i = 0; i < area.hp_flags_length; i++) {
        flag_info_t hp_flag = area.hp_flags[i];
        if (HAS_CHEST(hp_flag) || HAS_COLLECT(hp_flag) || HAS_HIGH_SCORES(hp_flag) || HAS_EVENT_CHK_INF(hp_flag) || HAS_ITEM_GET_INF(hp_flag) || HAS_INF_TABLE(hp_flag))
            hps += 1;
    }

    return hps;
}

void draw_world_map_info(z64_disp_buf_t* db) {
    int draw = CFG_WORLD_MAP_INFO_ENABLE &&
        z64_game.pause_ctxt.state == 6 &&
        z64_game.pause_ctxt.screen_idx == PAUSE_MAP &&
        !z64_game.pause_ctxt.changing &&
        z64_ctxt.input[0].raw.pad.a;
    if (!draw)
        return;

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

    for (int i = 0; i < world_map_area_count; i++) {
        world_map_area_entry_t* w = &(world_map_areas[i]);
        int top = start_top + ((icon_size + padding) * i) + 1;
        text_print(w->name, left, top);
    }

    left += (14 * font_sprite.tile_w) + padding;

    // Draw skull sprite

    sprite_load(db, &quest_items_sprite, 11, 1);

    for (int i = 0; i < world_map_area_count; i++) {
        int top = start_top + ((icon_size + padding) * i);
        sprite_draw(db, &quest_items_sprite, 0,
            left, top, icon_size, icon_size);
    }

    left += icon_size + padding;

    // Draw skull count

    for (int i = 0; i < world_map_area_count; i++) {
        uint8_t tokens = get_area_tokens(i);

        char count[2] = "0";
        count[0] += (tokens % 10);
        int top = start_top + ((icon_size + padding) * i) + 1;
        text_print(count, left, top);
    }

    left += icon_size + padding;

    // Draw hp sprite

    sprite_load(db, &quest_items_sprite, 13, 1);

    for (int i = 0; i < world_map_area_count; i++) {
        world_map_area_entry_t* w = &(world_map_areas[i]);
        if (w->hp_flags_length == 0)
            continue;

        int top = start_top + ((icon_size + padding) * i);
        sprite_draw(db, &quest_items_sprite, 0,
            left, top, icon_size, icon_size);
    }

    left += icon_size + padding;

    // Draw hp counts

    for (int i = 0; i < world_map_area_count; i++) {
        world_map_area_entry_t* w = &(world_map_areas[i]);
        if (w->hp_flags_length == 0)
            continue;

        int8_t hps = get_area_hps(i);

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
