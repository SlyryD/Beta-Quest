#ifndef WORLD_MAP_INFO_H
#define WORLD_MAP_INFO_H

#include "z64.h"
#include "gfx.h"
#include "text.h"
#include "util.h"

extern uint32_t gGsFlagsMasks[4];
extern uint32_t gGsFlagsShifts[4];

#define GET_GS_FLAGS(index) \
    ((z64_file.gs_flags[(index) >> 2] & gGsFlagsMasks[(index)&3]) >> gGsFlagsShifts[(index)&3])

#define DEKU_TREE 0
#define DODONGOS_CAVERN 1
#define JABU_JABU 2
#define FOREST_TEMPLE 3
#define FIRE_TEMPLE 4
#define WATER_TEMPLE 5
#define SPIRIT_TEMPLE 6
#define SHADOW_TEMPLE 7
#define BOTTOM_OF_THE_WELL 8
#define ICE_CAVERN 9

#define GERUDO_TRAINING_GROUND 11
#define THIEVES_HIDEOUT 12
#define INSIDE_GANONS_CASTLE 13

#define GOHMA 17
#define KING_DODONGO 18
#define BARINADE 19
#define PHANTOM_GANON 20
#define VOLVAGIA 21
#define MORPHA 22
#define TWINROVA 23
#define BONGO_BONGO 24

#define BLOCK_TOKEN (0x00000001 | \
                     0x00000002 | \
                     0x00000080 | \
                     0x00000400 | \
                     0x10000000 | \
                     0x20000000)

#define CAN_DRAW_TOKEN (((z64_link.state_flags_1 & BLOCK_TOKEN) == 0) &&                     \
                        ((uint32_t)z64_ctxt.state_dtor == z64_state_ovl_tab[3].vram_dtor) && \
                        (z64_file.game_mode == 0) &&                                         \
                        ((z64_event_state_1 & 0x20) == 0))

void draw_world_map_info();

#endif
