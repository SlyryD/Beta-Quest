#ifndef WORLD_MAP_INFO_H
#define WORLD_MAP_INFO_H

#include "z64.h"
#include "gfx.h"
#include "text.h"
#include "util.h"

typedef enum
{
    /* 0x00 */ HS_HBA, // horseback archery
    /* 0x01 */ HS_POE_POINTS,
    /* 0x02 */ HS_FISHING,
    /* 0x03 */ HS_HORSE_RACE,
    /* 0x04 */ HS_MARATHON,
    /* 0x05 */ HS_UNK_05,
    /* 0x06 */ HS_DAMPE_RACE
} high_scores_t;

typedef enum
{
    DEKU_TREE = 0x00,
    DODONGOS_CAVERN = 0x01,
    JABU_JABU = 0x02,
    FOREST_TEMPLE = 0x03,
    FIRE_TEMPLE = 0x04,
    WATER_TEMPLE = 0x05,
    SPIRIT_TEMPLE = 0x06,
    SHADOW_TEMPLE = 0x07,
    BOTTOM_OF_THE_WELL = 0x08,
    ICE_CAVERN = 0x09,
    GERUDO_TRAINING_GROUND = 0x0B,
    THIEVES_HIDEOUT = 0x0C,
    INSIDE_GANONS_CASTLE = 0x0D,
    TREASURE_BOX_SHOP = 0x10,
    GOHMA = 0x11,
    KING_DODONGO = 0x12,
    BARINADE = 0x13,
    PHANTOM_GANON = 0x14,
    VOLVAGIA = 0x15,
    MORPHA = 0x16,
    TWINROVA = 0x17,
    BONGO_BONGO = 0x18,
    DOG_LADY_HOUSE = 0x35,
    IMPAS_HOUSE = 0x37,
    LAKESIDE_LABORATORY = 0x38,
    GROTTOS = 0x3E,
    REDEAD_GRAVE = 0x3F,
    SHOOTING_GALLERY = 0x42,
    DAMPES_GRAVE_WINDMILL = 0x48,
    FISHING_POND = 0x49,
    BOMBCHU_BOWLING_ALLEY = 0x4B,
    RANCH_HOUSE_SILO = 0x4C,
    HOUSE_OF_SKULLTULA = 0x50,
    KAKARIKO_VILLAGE = 0x52,
    GRAVEYARD = 0x53,
    ZORAS_RIVER = 0x54,
    KOKIRI_FOREST = 0x55,
    LAKE_HYLIA = 0x57,
    ZORAS_DOMAIN = 0x58,
    ZORAS_FOUNTAIN = 0x59,
    GERUDO_VALLEY = 0x5A,
    LOST_WOODS = 0x5B,
    DESERT_COLOSSUS = 0x5C,
    GERUDOS_FORTRESS = 0x5D,
    HAUNTED_WASTELAND = 0x5E,
    DEATH_MOUNTAIN_TRAIL = 0x60,
    DEATH_MOUNTAIN_CRATER = 0x61,
    GORON_CITY = 0x62
} scene_t;

int8_t get_tokens(uint8_t index);
int8_t get_hps(uint8_t area_entry_index);

void draw_world_map_info(z64_disp_buf_t *db);

#endif
