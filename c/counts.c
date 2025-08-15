#include "counts.h"

#include "z64.h"

uint32_t gGsFlagsMasks[] = { 0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000 };
uint32_t gGsFlagsShifts[] = { 0, 8, 16, 24 };

// gs_flags at 8011B46C
#define GET_GS_FLAGS(gs_flags_index) \
    ((z64_file.gs_flags[(gs_flags_index) >> 2] & gGsFlagsMasks[(gs_flags_index)&3]) >> gGsFlagsShifts[(gs_flags_index)&3])

uint8_t get_tokens(uint8_t gs_flags_index) {
    uint8_t tokens = 0;
    int32_t token_flags = GET_GS_FLAGS(gs_flags_index);
    while (token_flags) {
        tokens += token_flags & 1;
        token_flags >>= 1;
    }

    return tokens;
}

uint8_t get_medallion_count() {
    return z64_file.light_medallion + z64_file.shadow_medallion + z64_file.spirit_medallion
           + z64_file.water_medallion + z64_file.fire_medallion + z64_file.forest_medallion;
}

uint8_t get_stone_count() {
    return z64_file.kokiris_emerald + z64_file.gorons_ruby + z64_file.zoras_sapphire;
}

uint8_t get_dungeon_item_count(uint8_t mask) {
    uint8_t count = 0;
    for (int i = 0; i < 20; i++) {
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
            return z64_file.zeldas_lullaby + z64_file.eponas_song + z64_file.sarias_song
                   + z64_file.suns_song + z64_file.song_of_time + z64_file.song_of_storms
                   + z64_file.minuet_of_forest + z64_file.bolero_of_fire + z64_file.serenade_of_water
                   + z64_file.requiem_of_spirit + z64_file.nocturne_of_shadow + z64_file.prelude_of_light;
        }
        case GCOUNT_MEDALLION: {
            return get_medallion_count();
        }
        case GCOUNT_STONE: {
            return get_stone_count();
        }
        case GCOUNT_REWARD: {
            return get_medallion_count() + get_stone_count();
        }
        case GCOUNT_BOSS_KEY: {
            return get_dungeon_item_count(0b001);
        }
        case GCOUNT_COMPASS: {
            return get_dungeon_item_count(0b010);
        }
        case GCOUNT_MAP: {
            return get_dungeon_item_count(0b100);
        }
        case GCOUNT_ELEMENTAL_ARROW: {
            return z64_file.items[Z64_SLOT_FIRE_ARROW] == Z64_ITEM_FIRE_ARROW
                + z64_file.items[Z64_SLOT_ICE_ARROW] == Z64_ITEM_ICE_ARROW
                + z64_file.items[Z64_SLOT_LIGHT_ARROW] == Z64_ITEM_LIGHT_ARROW;
        }
        case GCOUNT_FAIRY_SPELL: {
            return z64_file.items[Z64_SLOT_DINS_FIRE] == Z64_ITEM_DINS_FIRE
                + z64_file.items[Z64_SLOT_FARORES_WIND] == Z64_ITEM_FARORES_WIND
                + z64_file.items[Z64_SLOT_NAYRUS_LOVE] == Z64_ITEM_NAYRUS_LOVE;
        }
        case GCOUNT_BOTTLE_SLOT: {
            return z64_file.items[Z64_SLOT_BOTTLE_1] != Z64_ITEM_NULL
                + z64_file.items[Z64_SLOT_BOTTLE_2] != Z64_ITEM_NULL
                + z64_file.items[Z64_SLOT_BOTTLE_3] != Z64_ITEM_NULL
                + z64_file.items[Z64_SLOT_BOTTLE_4] != Z64_ITEM_NULL;
        }
        case GCOUNT_CARPENTER: {
            return get_carpenter_count(GCOUNT_CARPENTER);
        }
        case GCOUNT_BEAN: {
            return z64_file.magic_beans_sold;
        }
        case GCOUNT_TRAP: {
            return get_trap_count(GCOUNT_TRAP);
        }
        case GCOUNT_GOLD_RUPEE: {
            return get_gold_rupee_count(GCOUNT_GOLD_RUPEE);
        }
        // case GCOUNT_IRON_KNUCKLE: {
        //     return get_enemy_count(GCOUNT_IRON_KNUCKLE);
        // }
        // case GCOUNT_WHITE_WOLFOS: {
        //     return get_enemy_count(GCOUNT_WHITE_WOLFOS);
        // }
        // case GCOUNT_LIZALFOS: {
        //     return get_enemy_count(GCOUNT_LIZALFOS);
        // }
        // case GCOUNT_GIBDO: {
        //     return get_enemy_count(GCOUNT_GIBDO);
        // }
        // case GCOUNT_TENTACLE: {
        //     return get_enemy_count(GCOUNT_TENTACLE);
        // }
        // case GCOUNT_STALFOS: {
        //     return get_enemy_count(GCOUNT_STALFOS);
        // }
        // case GCOUNT_DEAD_HANDS: {
        //     return get_enemy_count(GCOUNT_DEAD_HANDS);
        // }
        // case GCOUNT_FLARE_DANCER: {
        //     return get_enemy_count(GCOUNT_FLARE_DANCER);
        // }
        default: {
            return 0xFF;
        }
    }
}
