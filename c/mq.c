#include "mq.h"
#include "mq_data.h"

#define MQ_SCENE_HEADROOM 0x200

typedef void* (*game_state_alloc_func_t)(void* heap, uint32_t size);
typedef void (*overlay_load_func_t)(
    uint32_t vrom_start,
    uint32_t vrom_end,
    void* vram_start,
    void* vram_end,
    void* loaded_ram
);

static game_state_alloc_func_t const game_state_alloc =
    (game_state_alloc_func_t)0x800A01B8;
static overlay_load_func_t const overlay_load =
    (overlay_load_func_t)0x800CCBB8;

uint32_t CFG_MASTER_QUEST;

static int mq_enabled() {
    CFG_MASTER_QUEST = z64_file.master_quest == 1;
    return CFG_MASTER_QUEST;
}

static void apply_mq_runs(
    uint32_t first_run,
    uint16_t run_count,
    uint8_t* segment
) {
    uint32_t i;

    for (i = 0; i < run_count; i++) {
        const mq_patch_run_t* run = &MQ_PATCH_RUNS[first_run + i];
        const uint8_t* data = &MQ_PATCH_DATA[run->data_offset];
        uint32_t j;

        for (j = 0; j < run->length; j++) {
            segment[run->offset + j] = data[j];
        }
    }
}

void* allocate_mq_scene_buffer(void* heap, uint32_t size) {
    if (mq_enabled()) {
        size += MQ_SCENE_HEADROOM;
    }
    return game_state_alloc(heap, size);
}

static void apply_mq_patches(
    uint8_t scene_id,
    int8_t room_id,
    uint8_t* segment
) {
    uint32_t i;

    if (!mq_enabled()) {
        return;
    }

    for (i = 0; i < MQ_PATCH_FILE_COUNT; i++) {
        const mq_file_patch_t* file = &MQ_PATCH_FILES[i];

        if (file->scene_id != scene_id || file->room_id != room_id) {
            continue;
        }

        apply_mq_runs(file->first_run, file->run_count, segment);
        return;
    }
}

void apply_mq_scene_patches(z64_game_t* game, uint8_t* segment) {
    apply_mq_patches(game->scene_index, -1, segment);
}

void apply_mq_room_patches(z64_game_t* game, uint8_t* segment) {
    int8_t room_id = *((int8_t*)game + 0x11CBC);
    apply_mq_patches(game->scene_index, room_id, segment);
}

void load_mq_patched_overlay(
    uint32_t vrom_start,
    uint32_t vrom_end,
    void* vram_start,
    void* vram_end,
    uint8_t* loaded_ram
) {
    uint32_t i;

    overlay_load(vrom_start, vrom_end, vram_start, vram_end, loaded_ram);
    if (!mq_enabled()) {
        return;
    }

    for (i = 0; i < MQ_PATCH_GLOBAL_COUNT; i++) {
        const mq_global_patch_t* patch = &MQ_PATCH_GLOBALS[i];

        if (patch->vrom_start == vrom_start) {
            apply_mq_runs(patch->first_run, patch->run_count, loaded_ram);
            return;
        }
    }
}
