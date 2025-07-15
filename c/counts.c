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

uint8_t get_item_count(uint8_t item_index) {
    switch (item_index) {
        case CI_SWORD: {
            return get_equipment_count(AREA_HYRULE_FIELD, CI_SWORD);
        }
        case CI_SHIELD: {
            return get_equipment_count(AREA_HYRULE_FIELD, CI_SHIELD);
        }
        case CI_TUNIC: {
            return get_equipment_count(AREA_HYRULE_FIELD, CI_TUNIC);
        }
        case CI_BOOT: {
            return get_equipment_count(AREA_HYRULE_FIELD, CI_BOOT);
        }
        case CI_SONG: {
            return get_quest_item_count(AREA_HYRULE_FIELD, CI_SONG);
        }
        case CI_MEDALLION: {
            return get_quest_item_count(AREA_HYRULE_FIELD, CI_MEDALLION);
        }
        case CI_STONE: {
            return get_quest_item_count(AREA_HYRULE_FIELD, CI_STONE);
        }
        case CI_REWARD: {
            return get_quest_item_count(AREA_HYRULE_FIELD, CI_REWARD);
        }
        case CI_BOSS_KEY: {
            return get_dungeon_item_count(AREA_HYRULE_FIELD, CI_BOSS_KEY);
        }
        case CI_COMPASS: {
            return get_dungeon_item_count(AREA_HYRULE_FIELD, CI_COMPASS);
        }
        case CI_MAP: {
            return get_dungeon_item_count(AREA_HYRULE_FIELD, CI_MAP);
        }
        case CI_ELEMENTAL_ARROW: {
            return get_inventory_item_count(AREA_HYRULE_FIELD, CI_ELEMENTAL_ARROW);
        }
        case CI_FAIRY_SPELL: {
            return get_inventory_item_count(AREA_HYRULE_FIELD, CI_FAIRY_SPELL);
        }
        case CI_BOTTLE_SLOT: {
            return get_inventory_item_count(AREA_HYRULE_FIELD, CI_BOTTLE_SLOT);
        }
        case CI_CARPENTER: {
            return get_inventory_item_count(AREA_HYRULE_FIELD, CI_CARPENTER);
        }
        case CI_BEAN: {
            return get_inventory_item_count(AREA_HYRULE_FIELD, CI_BEAN);
        }
        case CI_TRAP: {
            return get_inventory_item_count(AREA_HYRULE_FIELD, CI_TRAP);
        }
        case CI_GOLD_RUPEE: {
            return get_inventory_item_count(AREA_HYRULE_FIELD, CI_GOLD_RUPEE);
        }
        case CI_IRON_KNUCKLE: {
            return get_inventory_item_count(AREA_HYRULE_FIELD, CI_IRON_KNUCKLE);
        }
        case CI_WHITE_WOLFOS: {
            return get_inventory_item_count(AREA_HYRULE_FIELD, CI_WHITE_WOLFOS);
        }
        case CI_LIZALFOS: {
            return get_inventory_item_count(AREA_HYRULE_FIELD, CI_LIZALFOS);
        }
        case CI_GIBDO: {
            return get_inventory_item_count(AREA_HYRULE_FIELD, CI_GIBDO);
        }
        case CI_TENTACLE: {
            return get_inventory_item_count(AREA_HYRULE_FIELD, CI_TENTACLE);
        }
        case CI_STALFOS: {
            return get_inventory_item_count(AREA_HYRULE_FIELD, CI_STALFOS);
        }
        case CI_DEAD_HANDS: {
            return get_inventory_item_count(AREA_HYRULE_FIELD, CI_DEAD_HANDS);
        }
        case CI_FLARE_DANCER: {
            return get_inventory_item_count(AREA_HYRULE_FIELD, CI_FLARE_DANCER);
        }
        default: {
            return 0xFF;
        }
    }
}
