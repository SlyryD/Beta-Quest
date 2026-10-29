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
    // clang-format off
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
    // clang-format on
};

// gs_flags at 8011B46C
#define GET_GS_FLAGS(gs_flags_index)                                                     \
    ((z64_file.gs_flags[(gs_flags_index) >> 2] & gGsFlagsMasks[(gs_flags_index) & 3]) >> \
     gGsFlagsShifts[(gs_flags_index) & 3])

uint8_t get_tokens(uint8_t gs_flags_index) {
    return get_tokens_with_mask(gs_flags_index, 0xFF);
}

uint8_t get_tokens_with_mask(uint8_t gs_flags_index, uint8_t mask) {
    uint8_t tokens = 0;
    int32_t token_flags = GET_GS_FLAGS(gs_flags_index) & mask;
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

bool count_scene_switch_set(uint8_t scene, uint8_t flag) {
    uint32_t switches = z64_file.scene_flags[scene].swch;

    if (z64_game.scene_index == scene) {
        switches |= z64_game.swch_flags | z64_game.temp_swch_flags;
    }

    return switches & (1 << flag);
}

bool count_scene_chest_opened(uint8_t scene, uint8_t flag) {
    uint32_t chest_flags = z64_file.scene_flags[scene].chest;

    if (z64_game.scene_index == scene) {
        chest_flags |= z64_game.chest_flags;
    }

    return !!(chest_flags & (1 << flag));
}

bool count_scene_collect_set(uint8_t scene, uint8_t flag) {
    uint32_t collect_flags = z64_file.scene_flags[scene].collect;

    if (z64_game.scene_index == scene) {
        collect_flags |= z64_game.collect_flags | z64_game.temp_collect_flags;
    }

    return collect_flags & (1 << flag);
}

bool count_event_check_set(uint8_t flag) {
    return z64_file.event_chk_inf[flag >> 4] & (1 << (flag & 0xF));
}

bool count_item_get_set(uint8_t flag) {
    return z64_file.item_get_inf[flag >> 4] & (1 << (flag & 0xF));
}

uint8_t count_completed_objectives(uint8_t first, uint8_t count) {
    uint8_t completed = 0;

    for (uint8_t i = 0; i < count; i++) {
        completed += count_objective_is_complete(first + i);
    }

    return completed;
}

uint8_t get_carpenters_freed(uint16_t event_flags) {
    uint8_t count = 0;
    uint16_t carpenter_flags = event_flags & 0x000F;

    while (carpenter_flags) {
        count += carpenter_flags & 1;
        carpenter_flags >>= 1;
    }

    return count;
}

uint8_t get_trial_beams_cleared(const uint16_t* event_chk_inf) {
    static const uint8_t trial_flags[] = { 0xBC, 0xBF, 0xBE, 0xBD, 0xAD, 0xBB };
    uint8_t count = 0;

    for (int i = 0; i < array_size(trial_flags); i++) {
        uint8_t flag = trial_flags[i];
        count += !!(event_chk_inf[flag >> 4] & (1 << (flag & 0xF)));
    }

    return count;
}

bool get_trial_beam_cleared(uint8_t trial_index) {
    static const uint8_t trial_flags[] = { 0xBC, 0xBF, 0xBE, 0xBD, 0xAD, 0xBB };

    return trial_index < array_size(trial_flags) && count_event_check_set(trial_flags[trial_index]);
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
        case GCOUNT_TOKEN: {
            return z64_file.gs_tokens;
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
        case GCOUNT_TRIAL_BEAMS: {
            return get_trial_beams_cleared(z64_file.event_chk_inf);
        }
        case GCOUNT_MIDOS_GREEN_RUPEE: {
            return count_scene_chest_opened(SCENE_MIDOS_HOUSE, 0x02);
        }
        case GCOUNT_WASTELAND_PURPLE_RUPEE: {
            return count_scene_chest_opened(SCENE_HAUNTED_WASTELAND, 0x00);
        }
        case GCOUNT_COW: {
            return count_completed_objectives(OBJECTIVE_COW_0, 16);
        }
        case GCOUNT_HEART: {
            return z64_file.energy_capacity / 0x10;
        }
        case GCOUNT_SOIL_SKULLTULA: {
            static const struct {
                uint8_t index;
                uint8_t mask;
            } soil_skulltulas[] = {
                { 0x0C, 0x01 }, { 0x0D, 0x01 }, { 0x0D, 0x02 }, { 0x10, 0x01 }, { 0x0F, 0x02 },
                { 0x0F, 0x01 }, { 0x12, 0x01 }, { 0x13, 0x01 }, { 0x15, 0x01 },
            };
            uint8_t count = 0;

            for (int i = 0; i < array_size(soil_skulltulas); i++) {
                count += get_tokens_with_mask(soil_skulltulas[i].index, soil_skulltulas[i].mask);
            }
            return count;
        }
        case GCOUNT_IRON_KNUCKLE:
        case GCOUNT_WHITE_WOLFOS:
        case GCOUNT_LIZALFOS:
        case GCOUNT_GIBDO:
        case GCOUNT_TENTACLE:
        case GCOUNT_STALFOS:
        case GCOUNT_DEAD_HANDS:
        case GCOUNT_FLARE_DANCER:
        case GCOUNT_SKULL_KID: {
            return count_tracker_get_enemy_count(count_index - GCOUNT_IRON_KNUCKLE);
        }
        default: {
            return 0xFF;
        }
    }
}
