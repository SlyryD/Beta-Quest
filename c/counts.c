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

uint8_t get_item_count(uint8_t item_index) {
    switch (item_index) {
        case CI_SWORD: {
            return z64_file.kokiri_sword + z64_file.master_sword + z64_file.giants_knife + z64_file.broken_giants_knife;
        }
        case CI_SHIELD: {
            return z64_file.deku_shield + z64_file.hylian_shield + z64_file.mirror_shield;
        }
        case CI_TUNIC: {
            return z64_file.kokiri_tunic + z64_file.goron_tunic + z64_file.zora_tunic;
        }
        case CI_BOOT: {
            return z64_file.kokiri_boots + z64_file.iron_boots + z64_file.hover_boots;
        }
        case CI_SONG: {
            return z64_file.zeldas_lullaby + z64_file.eponas_song + z64_file.sarias_song
                   + z64_file.suns_song + z64_file.song_of_time + z64_file.song_of_storms
                   + z64_file.minuet_of_forest + z64_file.bolero_of_fire + z64_file.serenade_of_water
                   + z64_file.requiem_of_spirit + z64_file.nocturne_of_shadow + z64_file.prelude_of_light;
        }
        case CI_MEDALLION: {
            return get_medallion_count();
        }
        case CI_STONE: {
            return get_stone_count();
        }
        case CI_REWARD: {
            return get_medallion_count() + get_stone_count();
        }
        case CI_BOSS_KEY: {
            return get_dungeon_item_count(0b001);
        }
        case CI_COMPASS: {
            return get_dungeon_item_count(0b010);
        }
        case CI_MAP: {
            return get_dungeon_item_count(0b100);
        }
        case CI_ELEMENTAL_ARROW: {
            return get_inventory_item_count(CI_ELEMENTAL_ARROW);
        }
        case CI_FAIRY_SPELL: {
            return get_inventory_item_count(CI_FAIRY_SPELL);
        }
        case CI_BOTTLE_SLOT: {
            return z64_file.items[Z64_SLOT_BOTTLE_1] != Z64_ITEM_NULL
                + z64_file.items[Z64_SLOT_BOTTLE_2] != Z64_ITEM_NULL
                + z64_file.items[Z64_SLOT_BOTTLE_3] != Z64_ITEM_NULL
                + z64_file.items[Z64_SLOT_BOTTLE_4] != Z64_ITEM_NULL;
        }
        case CI_CARPENTER: {
            return get_carpenter_count(CI_CARPENTER);
        }
        case CI_BEAN: {
            return z64_file.magic_beans_sold;
        }
        case CI_TRAP: {
            return get_trap_count(CI_TRAP);
        }
        case CI_GOLD_RUPEE: {
            return get_gold_rupee_count(CI_GOLD_RUPEE);
        }
        // case CI_IRON_KNUCKLE: {
        //     return get_enemy_count(CI_IRON_KNUCKLE);
        // }
        // case CI_WHITE_WOLFOS: {
        //     return get_enemy_count(CI_WHITE_WOLFOS);
        // }
        // case CI_LIZALFOS: {
        //     return get_enemy_count(CI_LIZALFOS);
        // }
        // case CI_GIBDO: {
        //     return get_enemy_count(CI_GIBDO);
        // }
        // case CI_TENTACLE: {
        //     return get_enemy_count(CI_TENTACLE);
        // }
        // case CI_STALFOS: {
        //     return get_enemy_count(CI_STALFOS);
        // }
        // case CI_DEAD_HANDS: {
        //     return get_enemy_count(CI_DEAD_HANDS);
        // }
        // case CI_FLARE_DANCER: {
        //     return get_enemy_count(CI_FLARE_DANCER);
        // }
        default: {
            return 0xFF;
        }
    }
}
