#include "counts.h"

#include "count_tracker.h"
#include "save.h"
#include "util.h"
#include "z64.h"

uint32_t gGsFlagsMasks[] = { 0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000 };
uint32_t gGsFlagsShifts[] = { 0, 8, 16, 24 };

typedef struct {
    uint8_t scene;
    uint8_t switch_flag;
} bean_flag_t;

static bean_flag_t bean_flags[] = {
    { SCENE_KOKIRI_FOREST, 0x09 },
    { SCENE_LOST_WOODS, 0x04 },
    { SCENE_LOST_WOODS, 0x12 },
    { SCENE_ZORAS_RIVER, 0x03 },
    { SCENE_GRAVEYARD, 0x03 },
    { SCENE_DEATH_MOUNTAIN_TRAIL, 0x06 },
    { SCENE_DEATH_MOUNTAIN_CRATER, 0x03 },
    { SCENE_LAKE_HYLIA, 0x01 },
    { SCENE_GERUDO_VALLEY, 0x03 },
    { SCENE_DESERT_COLOSSUS, 0x18 },
};

// gs_flags at 8011B46C
#define GET_GS_FLAGS(gs_flags_index)                                                     \
    ((z64_file.gs_flags[(gs_flags_index) >> 2] & gGsFlagsMasks[(gs_flags_index) & 3]) >> \
     gGsFlagsShifts[(gs_flags_index) & 3])

uint8_t get_tokens(uint8_t gs_flags_index) {
    uint8_t tokens = 0;
    int32_t token_flags = GET_GS_FLAGS(gs_flags_index);
    while (token_flags) {
        tokens += token_flags & 1;
        token_flags >>= 1;
    }

    return tokens;
}

uint8_t get_global_medallions() {
    return z64_file.light_medallion + z64_file.shadow_medallion + z64_file.spirit_medallion + z64_file.water_medallion +
           z64_file.fire_medallion + z64_file.forest_medallion;
}

uint8_t get_global_stones() {
    return z64_file.kokiris_emerald + z64_file.gorons_ruby + z64_file.zoras_sapphire;
}

static uint8_t get_beans_planted() {
    uint8_t count = 0;

    for (int i = 0; i < array_size(bean_flags); i++) {
        bean_flag_t* bean = &bean_flags[i];
        uint32_t switches = z64_file.scene_flags[bean->scene].swch;

        if (z64_game.scene_index == bean->scene) {
            switches |= z64_game.swch_flags | z64_game.temp_swch_flags;
        }
        count += !!(switches & (1 << bean->switch_flag));
    }

    return count;
}

static uint8_t get_carpenters_freed(uint16_t event_flags) {
    uint8_t count = 0;
    uint16_t carpenter_flags = event_flags & 0x1111;

    while (carpenter_flags) {
        count += carpenter_flags & 1;
        carpenter_flags >>= 4;
    }

    return count;
}

uint8_t get_global_dungeon_items(uint8_t mask) {
    uint8_t count = 0;
    int dungeon_items_length = array_size(z64_file.dungeon_items);
    for (int i = 0; i < dungeon_items_length; i++) {
        if (z64_file.dungeon_items[i].items & mask) {
            count++;
        }
    }
    return count;
}

uint8_t get_global_count(uint8_t count_index) {
    switch (count_index) {
        case GCOUNT_SWORD: {
            return z64_file.kokiri_sword + z64_file.master_sword + z64_file.giants_knife + z64_file.broken_giants_knife;
        }
        case GCOUNT_SHIELD: {
            return z64_file.deku_shield + z64_file.hylian_shield + z64_file.mirror_shield;
        }
        case GCOUNT_TUNIC: {
            return z64_file.kokiri_tunic + z64_file.goron_tunic + z64_file.zora_tunic;
        }
        case GCOUNT_BOOT: {
            return z64_file.kokiri_boots + z64_file.iron_boots + z64_file.hover_boots;
        }
        case GCOUNT_SONG: {
            return z64_file.zeldas_lullaby + z64_file.eponas_song + z64_file.sarias_song + z64_file.suns_song +
                   z64_file.song_of_time + z64_file.song_of_storms + z64_file.minuet_of_forest +
                   z64_file.bolero_of_fire + z64_file.serenade_of_water + z64_file.requiem_of_spirit +
                   z64_file.nocturne_of_shadow + z64_file.prelude_of_light;
        }
        case GCOUNT_MEDALLION: {
            return get_global_medallions();
        }
        case GCOUNT_STONE: {
            return get_global_stones();
        }
        case GCOUNT_REWARD: {
            return get_global_medallions() + get_global_stones();
        }
        case GCOUNT_BOSS_KEY: {
            return get_global_dungeon_items(0b001);
        }
        case GCOUNT_COMPASS: {
            return get_global_dungeon_items(0b010);
        }
        case GCOUNT_MAP: {
            return get_global_dungeon_items(0b100);
        }
        case GCOUNT_ELEMENTAL_ARROW: {
            return (z64_file.items[Z64_SLOT_FIRE_ARROW] == Z64_ITEM_FIRE_ARROW) +
                   (z64_file.items[Z64_SLOT_ICE_ARROW] == Z64_ITEM_ICE_ARROW) +
                   (z64_file.items[Z64_SLOT_LIGHT_ARROW] == Z64_ITEM_LIGHT_ARROW);
        }
        case GCOUNT_FAIRY_SPELL: {
            return (z64_file.items[Z64_SLOT_DINS_FIRE] == Z64_ITEM_DINS_FIRE) +
                   (z64_file.items[Z64_SLOT_FARORES_WIND] == Z64_ITEM_FARORES_WIND) +
                   (z64_file.items[Z64_SLOT_NAYRUS_LOVE] == Z64_ITEM_NAYRUS_LOVE);
        }
        case GCOUNT_BOTTLE_SLOT: {
            return (z64_file.items[Z64_SLOT_BOTTLE_1] != Z64_ITEM_NULL) +
                   (z64_file.items[Z64_SLOT_BOTTLE_2] != Z64_ITEM_NULL) +
                   (z64_file.items[Z64_SLOT_BOTTLE_3] != Z64_ITEM_NULL) +
                   (z64_file.items[Z64_SLOT_BOTTLE_4] != Z64_ITEM_NULL);
        }
        case GCOUNT_CARPENTER: {
            return get_carpenters_freed(z64_file.event_chk_inf[9]);
        }
        case GCOUNT_BEANS_PLANTED: {
            return get_beans_planted();
        }
        case GCOUNT_TRAP: {
            return extended_savectx.trap_count;
        }
        case GCOUNT_GOLD_RUPEE: {
            return extended_savectx.gold_rupee_count;
        }
        case GCOUNT_SILVER_RUPEE_ROOMS: {
            return count_tracker_get_silver_rooms_cleared();
        }
        case GCOUNT_IRON_KNUCKLE:
        case GCOUNT_WHITE_WOLFOS:
        case GCOUNT_LIZALFOS:
        case GCOUNT_GIBDO:
        case GCOUNT_TENTACLE:
        case GCOUNT_STALFOS:
        case GCOUNT_DEAD_HANDS:
        case GCOUNT_FLARE_DANCER: {
            return count_tracker_get_enemy_count(count_index - GCOUNT_IRON_KNUCKLE);
        }
        default: {
            return 0xFF;
        }
    }
}
