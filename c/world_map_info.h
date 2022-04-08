#ifndef WORLD_MAP_INFO_H
#define WORLD_MAP_INFO_H

#include "z64.h"
#include "gfx.h"
#include "text.h"
#include "util.h"

extern uint32_t gGsFlagsMasks[4];
extern uint32_t gGsFlagsShifts[4];

int8_t get_tokens(uint8_t index);

#define DEKU_TREE 0x0
#define DODONGOS_CAVERN 0x1
#define JABU_JABU 0x2
#define FOREST_TEMPLE 0x3
#define FIRE_TEMPLE 0x4
#define WATER_TEMPLE 0x5
#define SPIRIT_TEMPLE 0x6
#define SHADOW_TEMPLE 0x7
#define BOTTOM_OF_THE_WELL 0x8
#define ICE_CAVERN 0x9

#define GERUDO_TRAINING_GROUND 0xB
#define THIEVES_HIDEOUT 0xC
#define INSIDE_GANONS_CASTLE 0xD

#define TREASURE_BOX_SHOP 0x10

#define GOHMA 0x11
#define KING_DODONGO 0x12
#define BARINADE 0x13
#define PHANTOM_GANON 0x14
#define VOLVAGIA 0x15
#define MORPHA 0x16
#define TWINROVA 0x17
#define BONGO_BONGO 0x18

#define DOG_LADY 0x35
#define LAKESIDE_LABORATORY 0x38
#define GROTTOS 0x3E
#define REDEAD_GRAVE 0x3F
#define SHOOTING_GALLERY 0x42
#define DAMPES_GRAVE_WINDMILL 0x48
#define FISHING_POND 0x49
#define BOMBCHU_BOWLING_ALLEY 0x4B
#define RANCH_HOUSE_SILO 0x4C
#define HOUSE_OF_SKULLTULA 0x50

#define KAKARIKO_VILLAGE 0x52
#define GRAVEYARD 0x53
#define ZORAS_RIVER 0x54
#define KOKIRI_FOREST 0x55
#define LAKE_HYLIA 0x57
#define ZORAS_DOMAIN 0x58
#define ZORAS_FOUNTAIN 0x59
#define GERUDO_VALLEY 0x5A
#define LOST_WOODS 0x5B
#define DESERT_COLOSSUS 0x5C
#define GERUDOS_FORTRESS 0x5D
#define HAUNTED_WASTELAND 0x5E
#define DEATH_MOUNTAIN_TRAIL 0x60
#define DEATH_MOUNTAIN_CRATER 0x61
#define GORON_CITY 0x62

void draw_world_map_info();

#endif
