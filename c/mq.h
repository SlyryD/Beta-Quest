#ifndef MQ_H
#define MQ_H

#include "z64.h"

extern uint32_t CFG_MASTER_QUEST;

void* allocate_mq_scene_buffer(void* heap, uint32_t size);
void apply_mq_scene_patches(z64_game_t* game, uint8_t* segment);
void apply_mq_room_patches(z64_game_t* game, uint8_t* segment);
void load_mq_patched_overlay(
    uint32_t vrom_start,
    uint32_t vrom_end,
    void* vram_start,
    void* vram_end,
    uint8_t* loaded_ram
);

#endif
