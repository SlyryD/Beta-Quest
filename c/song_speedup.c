#include "song_speedup.h"

#define ARRAY_COUNT(array) (sizeof(array) / sizeof((array)[0]))
#define SCENE_ICE_CAVERN 0x09
#define SCENE_TEMPLE_OF_TIME 0x43
#define SCENE_CASTLE_COURTYARD_ZELDA 0x4A
#define SCENE_SACRED_FOREST_MEADOW 0x56
#define SCENE_DEATH_MOUNTAIN_CRATER 0x61

typedef struct {
    uint16_t offset;
    uint8_t size;
    uint8_t data[6];
} song_patch_t;

extern uint32_t CFG_SONG_SPEEDUPS;

typedef char assert_scene_segment_offset[
    __builtin_offsetof(z64_game_t, scene_segment) == 0xB0 ? 1 : -1];

static const song_patch_t castle_courtyard_patches[] = {
    { 0x2916, 6, { 0x00, 0x3B, 0x00, 0x3C, 0x00, 0x3C } },
    { 0x2924, 2, { 0x00, 0x17 } },
    { 0x2929, 5, { 0x10, 0x00, 0x02, 0x08, 0x8B } },
    { 0x2930, 2, { 0x00, 0xD4 } },
    { 0x2933, 1, { 0x11 } },
    { 0x2935, 1, { 0x20 } },
};

static const song_patch_t sacred_forest_meadow_patches[] = {
    { 0x3F86, 2, { 0x00, 0x3C } },
    { 0x3F91, 1, { 0x11 } },
    { 0x3F95, 1, { 0x10 } },
    { 0x3FC1, 6, { 0x00, 0x3E, 0x00, 0x11, 0x00, 0x20 } },
    { 0x3FC8, 1, { 0x00 } },
    { 0x4492, 4, { 0x00, 0x21, 0x00, 0x22 } },
    { 0x44CA, 4, { 0x00, 0x00, 0x00, 0x00 } },
    { 0x4808, 2, { 0x00, 0x0F } },
    { 0x480C, 6, { 0x00, 0x10, 0x00, 0x02, 0x08, 0x8B } },
    { 0x4814, 6, { 0x00, 0x73, 0x00, 0x11, 0x00, 0x20 } },
};

static const song_patch_t death_mountain_crater_patches[] = {
    { 0x45D6, 2, { 0x00, 0x3C } },
    { 0x45E1, 1, { 0x11 } },
    { 0x45E5, 1, { 0x10 } },
    { 0x4611, 1, { 0x3E } },
    { 0x4613, 1, { 0x11 } },
    { 0x4615, 1, { 0x20 } },
    { 0x47F9, 1, { 0x00 } },
    { 0x4829, 1, { 0x00 } },
    { 0x4859, 1, { 0x00 } },
    { 0x4889, 1, { 0x00 } },
    { 0x67F0, 2, { 0x00, 0x10 } },
    { 0x67F5, 5, { 0x10, 0x00, 0x02, 0x08, 0x8B } },
    { 0x67FC, 2, { 0x00, 0x74 } },
    { 0x67FF, 1, { 0x11 } },
    { 0x6801, 1, { 0x20 } },
};

static const song_patch_t ice_cavern_patches[] = {
    { 0x0256, 2, { 0x00, 0x3C } },
    { 0x0261, 1, { 0x11 } },
    { 0x0265, 1, { 0x10 } },
    { 0x0291, 1, { 0x3E } },
    { 0x0293, 1, { 0x11 } },
    { 0x0295, 1, { 0x20 } },
    { 0x0539, 1, { 0x00 } },
    { 0x0548, 1, { 0x80 } },
    { 0x0554, 1, { 0x80 } },
    { 0x1852, 4, { 0x00, 0x21, 0x00, 0x22 } },
    { 0x1888, 2, { 0x00, 0x11 } },
    { 0x188D, 5, { 0x10, 0x00, 0x02, 0x08, 0x8B } },
    { 0x1894, 2, { 0x00, 0x75 } },
    { 0x1897, 1, { 0x11 } },
    { 0x1899, 1, { 0x20 } },
};

static const song_patch_t temple_of_time_patches[] = {
    { 0x6D26, 2, { 0x00, 0x3C } },
    { 0x6F1D, 1, { 0x00 } },
    { 0x8328, 2, { 0x00, 0x14 } },
    { 0x832D, 5, { 0x10, 0x00, 0x02, 0x08, 0x8B } },
    { 0x8334, 2, { 0x00, 0x78 } },
    { 0x8337, 1, { 0x11 } },
    { 0x8339, 1, { 0x20 } },
    { 0x83DA, 4, { 0x00, 0x21, 0x00, 0x22 } },
};

static void apply_patches(uint8_t* scene, const song_patch_t* patches, uint32_t count) {
    uint32_t i;
    uint32_t j;

    for (i = 0; i < count; i++) {
        for (j = 0; j < patches[i].size; j++) {
            scene[patches[i].offset + j] = patches[i].data[j];
        }
    }
}

void apply_song_speedups(z64_game_t* game) {
    const song_patch_t* patches;
    uint32_t count;

    if (CFG_SONG_SPEEDUPS == 0 || game->scene_segment == NULL) {
        return;
    }

    switch (game->scene_index) {
        case SCENE_ICE_CAVERN:
            patches = ice_cavern_patches;
            count = ARRAY_COUNT(ice_cavern_patches);
            break;
        case SCENE_TEMPLE_OF_TIME:
            patches = temple_of_time_patches;
            count = ARRAY_COUNT(temple_of_time_patches);
            break;
        case SCENE_CASTLE_COURTYARD_ZELDA:
            patches = castle_courtyard_patches;
            count = ARRAY_COUNT(castle_courtyard_patches);
            break;
        case SCENE_SACRED_FOREST_MEADOW:
            patches = sacred_forest_meadow_patches;
            count = ARRAY_COUNT(sacred_forest_meadow_patches);
            break;
        case SCENE_DEATH_MOUNTAIN_CRATER:
            patches = death_mountain_crater_patches;
            count = ARRAY_COUNT(death_mountain_crater_patches);
            break;
        default:
            return;
    }

    apply_patches(game->scene_segment, patches, count);
}
