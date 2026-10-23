#include "world_map_info.h"

#include <stdbool.h>

#include "counts.h"
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

typedef enum {
    HP_SOURCE_CHEST,
    HP_SOURCE_COLLECT,
    HP_SOURCE_HIGH_SCORE,
    HP_SOURCE_EVENT_CHECK,
    HP_SOURCE_ITEM_GET,
    HP_SOURCE_INFO_TABLE,
} hp_source_t;

typedef struct {
    uint8_t source;
    uint8_t index;
    uint16_t flag;
} hp_check_t;

#define HP_CHEST(scene, flag) { HP_SOURCE_CHEST, SCENE_##scene, flag }
#define HP_COLLECT(scene, flag) { HP_SOURCE_COLLECT, SCENE_##scene, flag }
#define HP_HIGH_SCORE(score, flag) { HP_SOURCE_HIGH_SCORE, HS_##score, flag }
#define HP_EVENT_CHECK(flag) { HP_SOURCE_EVENT_CHECK, 0, flag }
#define HP_ITEM_GET(flag) { HP_SOURCE_ITEM_GET, 0, flag }
#define HP_INFO_TABLE(flag) { HP_SOURCE_INFO_TABLE, 0, flag }

static const hp_check_t wasteland_hp_checks[] = {
    HP_COLLECT(DESERT_COLOSSUS, 0x0D), // Colossus freestanding HP
};

static const hp_check_t fortress_hp_checks[] = {
    HP_CHEST(GERUDOS_FORTRESS, 0x00),   // Gerudo Fortress chest
    HP_INFO_TABLE(0x190),               // Horseback archery 1,000 points
    HP_COLLECT(GERUDOS_FORTRESS, 0x01), // Gerudo Fortress child HP
};

static const hp_check_t gerudo_valley_hp_checks[] = {
    HP_COLLECT(GERUDO_VALLEY, 0x02), // Crate freestanding HP
    HP_COLLECT(GERUDO_VALLEY, 0x01), // Waterfall freestanding HP
};

static const hp_check_t lake_hylia_hp_checks[] = {
    HP_HIGH_SCORE(FISHING, 0x400), // Child fishing
    HP_ITEM_GET(0x10),             // Laboratory dive
    HP_COLLECT(LAKE_HYLIA, 0x1E),  // Freestanding HP
};

static const hp_check_t lon_lon_ranch_hp_checks[] = {
    HP_COLLECT(RANCH_HOUSE_SILO, 0x01), // Freestanding HP
};

static const hp_check_t market_hp_checks[] = {
    HP_ITEM_GET(0x12),                 // Bombchu Bowling second prize
    HP_INFO_TABLE(0x191),              // Lost dog
    HP_CHEST(TREASURE_BOX_SHOP, 0x0A), // Treasure Chest Game reward
};

static const hp_check_t hyrule_field_hp_checks[] = {
    HP_COLLECT(GROTTOS, 0x01), // Tektite Grotto freestanding HP
    HP_ITEM_GET(0x0B),         // Deku Scrub Grotto
};

static const hp_check_t death_mountain_hp_checks[] = {
    HP_COLLECT(DEATH_MOUNTAIN_TRAIL, 0x1E),  // DMT freestanding HP
    HP_COLLECT(DEATH_MOUNTAIN_CRATER, 0x08), // DMC volcano freestanding HP
    HP_COLLECT(DEATH_MOUNTAIN_CRATER, 0x02), // DMC wall freestanding HP
    HP_COLLECT(GORON_CITY, 0x1F),            // Goron City pot freestanding HP
};

static const hp_check_t kakariko_hp_checks[] = {
    HP_COLLECT(IMPAS_HOUSE, 0x01),           // Impa's House freestanding HP
    HP_COLLECT(DAMPES_GRAVE_WINDMILL, 0x01), // Windmill freestanding HP
    HP_ITEM_GET(0x15),                       // Man on roof
    HP_EVENT_CHECK(0xDE),                    // 50 Gold Skulltula reward
    HP_CHEST(REDEAD_GRAVE, 0x00),            // Heart Piece Grave chest
    HP_COLLECT(GRAVEYARD, 0x04),             // Graveyard freestanding HP
    HP_ITEM_GET(0x1C),                       // Dampe's Gravedigging Tour
    HP_COLLECT(DAMPES_GRAVE_WINDMILL, 0x07), // Dampe race freestanding HP
};

static const hp_check_t lost_woods_hp_checks[] = {
    HP_ITEM_GET(0x17), // Ocarina memory game
    HP_ITEM_GET(0x16), // Skull Kid
};

static const hp_check_t zora_hp_checks[] = {
    HP_EVENT_CHECK(0xD6),             // Frogs in the rain
    HP_EVENT_CHECK(0xD0),             // Frogs Ocarina Game
    HP_COLLECT(ZORAS_RIVER, 0x04),    // Near Open Grotto freestanding HP
    HP_COLLECT(ZORAS_RIVER, 0x0B),    // Near Domain freestanding HP
    HP_CHEST(ZORAS_DOMAIN, 0x00),     // Zora's Domain chest
    HP_COLLECT(ZORAS_FOUNTAIN, 0x01), // Iceberg freestanding HP
    HP_COLLECT(ZORAS_FOUNTAIN, 0x14), // Bottom freestanding HP
};

#undef HP_CHEST
#undef HP_COLLECT
#undef HP_HIGH_SCORE
#undef HP_EVENT_CHECK
#undef HP_ITEM_GET
#undef HP_INFO_TABLE

typedef struct {
    uint8_t index;
    uint8_t gs_flags_index;
    const hp_check_t* hp_checks;
    uint8_t hp_check_count;
    char name[15];
} world_map_area_entry_t;

#define WORLD_MAP_AREA(area, gs_index, hp_checks, name) \
    { AREA_##area, gs_index, hp_checks, array_size(hp_checks), name }
#define WORLD_MAP_AREA_WITHOUT_HPS(area, gs_index, name) { AREA_##area, gs_index, 0, 0, name }

static const world_map_area_entry_t world_map_areas[] = {
    WORLD_MAP_AREA(WASTELAND, 21, wasteland_hp_checks, "Wasteland Area"),
    WORLD_MAP_AREA(FORTRESS, 20, fortress_hp_checks, "Fortress Area"),
    WORLD_MAP_AREA(GERUDO_VALLEY, 19, gerudo_valley_hp_checks, "Gerudo Valley"),
    WORLD_MAP_AREA(LAKE_HYLIA, 18, lake_hylia_hp_checks, "Lake Hylia"),
    WORLD_MAP_AREA(LON_LON_RANCH, 11, lon_lon_ranch_hp_checks, "Lon Lon Ranch"),
    WORLD_MAP_AREA(MARKET, 14, market_hp_checks, "Market Area"),
    WORLD_MAP_AREA(HYRULE_FIELD, 10, hyrule_field_hp_checks, "Hyrule Field"),
    WORLD_MAP_AREA(DEATH_MOUNTAIN, 15, death_mountain_hp_checks, "Death Mountain"),
    WORLD_MAP_AREA(KAKARIKO, 16, kakariko_hp_checks, "Kakariko Area"),
    WORLD_MAP_AREA(LOST_WOODS, 13, lost_woods_hp_checks, "Lost Woods"),
    WORLD_MAP_AREA_WITHOUT_HPS(KOKIRI_FOREST, 12, "Kokiri Forest"),
    WORLD_MAP_AREA(ZORA, 17, zora_hp_checks, "Zora Area"),
};

#undef WORLD_MAP_AREA
#undef WORLD_MAP_AREA_WITHOUT_HPS

static const int world_map_areas_length = array_size(world_map_areas);

extern uint32_t CFG_WORLD_MAP_INFO_ENABLE;

static bool has_hp_check(const hp_check_t* check) {
    switch (check->source) {
        case HP_SOURCE_CHEST:
            return (z64_game.scene_index == check->index && (z64_game.chest_flags & (1 << check->flag))) ||
                   (z64_file.scene_flags[check->index].chest & (1 << check->flag));
        case HP_SOURCE_COLLECT:
            return (z64_game.scene_index == check->index && (z64_game.collect_flags & (1 << check->flag))) ||
                   (z64_file.scene_flags[check->index].collect & (1 << check->flag));
        case HP_SOURCE_HIGH_SCORE:
            return z64_file.high_scores[check->index] & check->flag;
        case HP_SOURCE_EVENT_CHECK:
            return z64_file.event_chk_inf[check->flag >> 4] & (1 << (check->flag & 0xF));
        case HP_SOURCE_ITEM_GET:
            return z64_file.item_get_inf[check->flag >> 4] & (1 << (check->flag & 0xF));
        case HP_SOURCE_INFO_TABLE:
            return z64_file.inf_table[check->flag >> 4] & (1 << (check->flag & 0xF));
        default:
            return false;
    }
}

static bool is_valid_area_index(uint8_t area_index) {
    return area_index < world_map_areas_length && world_map_areas[area_index].index == area_index;
}

static uint8_t get_area_tokens(uint8_t area_index) {
    return get_tokens(world_map_areas[area_index].gs_flags_index);
}

static uint8_t get_area_hps(uint8_t area_index) {
    uint8_t hps = 0;
    const world_map_area_entry_t* area = &world_map_areas[area_index];
    for (int i = 0; i < area->hp_check_count; i++) {
        if (has_hp_check(&area->hp_checks[i])) {
            hps += 1;
        }
    }

    return hps;
}

uint8_t get_area_count(uint8_t area_index, uint8_t count_index) {
    if (!is_valid_area_index(area_index)) {
        return 0xFF;
    }

    switch (count_index) {
        case ACOUNT_TOKEN: {
            return get_area_tokens(area_index);
        }
        case ACOUNT_HEART_PIECE: {
            return get_area_hps(area_index);
        }
        default: {
            return 0xFF;
        }
    }
}

void draw_world_map_info(z64_disp_buf_t* db) {
    int draw = CFG_WORLD_MAP_INFO_ENABLE && z64_game.pause_ctxt.state == 6 &&
               z64_game.pause_ctxt.screen_idx == PAUSE_MAP && !z64_game.pause_ctxt.changing &&
               z64_ctxt.input[0].raw.pad.a;
    if (!draw) {
        return;
    }

    db->p = db->buf;

    // Call setup display list

    gSPDisplayList(db->p++, &setup_db);

    // Set up dimensions

    int icon_size = 16;
    int padding = 1;
    int bg_width = padding + (14 * font_sprite.tile_w) +  // world map area names
                   padding + (4 * (icon_size + padding)); // skull, skull count, hp, hp count
    int bg_height = padding + (world_map_areas_length * (icon_size + padding));
    int bg_left = (Z64_SCREEN_WIDTH - bg_width) / 2;
    int bg_top = (Z64_SCREEN_HEIGHT - bg_height) / 2;

    int left = bg_left + padding;
    int start_top = bg_top + padding;

    // Draw background

    gDPSetCombineMode(db->p++, G_CC_PRIMITIVE, G_CC_PRIMITIVE);
    gDPSetPrimColor(db->p++, 0, 0, 0x00, 0x00, 0x00, 0xD0);
    gSPTextureRectangle(db->p++, bg_left << 2, bg_top << 2, (bg_left + bg_width) << 2, (bg_top + bg_height) << 2, 0, 0,
                        0, 1 << 10, 1 << 10);

    // Set the primary color to white for drawing sprites

    gDPPipeSync(db->p++);
    gDPSetCombineMode(db->p++, G_CC_MODULATEIA_PRIM, G_CC_MODULATEIA_PRIM);

    gDPSetPrimColor(db->p++, 0, 0, 0xFF, 0xFF, 0xFF, 0xFF);

    // Draw area names

    for (int i = 0; i < world_map_areas_length; i++) {
        const world_map_area_entry_t* w = &world_map_areas[i];
        int top = start_top + ((icon_size + padding) * i) + 1;
        text_print(w->name, left, top);
    }

    left += (14 * font_sprite.tile_w) + padding;

    // Draw skull sprite

    sprite_load(db, &quest_items_sprite, 11, 1);

    for (int i = 0; i < world_map_areas_length; i++) {
        int top = start_top + ((icon_size + padding) * i);
        sprite_draw(db, &quest_items_sprite, 0, left, top, icon_size, icon_size);
    }

    left += icon_size + padding;

    // Draw skull count

    for (int i = 0; i < world_map_areas_length; i++) {
        uint8_t tokens = get_area_tokens(i);

        char count[2] = "0";
        count[0] += (tokens % 10);
        int top = start_top + ((icon_size + padding) * i) + 1;
        text_print(count, left, top);
    }

    left += icon_size + padding;

    // Draw hp sprite

    sprite_load(db, &quest_items_sprite, 13, 1);

    for (int i = 0; i < world_map_areas_length; i++) {
        const world_map_area_entry_t* w = &world_map_areas[i];
        if (w->hp_check_count == 0) {
            continue;
        }

        int top = start_top + ((icon_size + padding) * i);
        sprite_draw(db, &quest_items_sprite, 0, left, top, icon_size, icon_size);
    }

    left += icon_size + padding;

    // Draw hp counts

    for (int i = 0; i < world_map_areas_length; i++) {
        const world_map_area_entry_t* w = &world_map_areas[i];
        if (w->hp_check_count == 0) {
            continue;
        }

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
