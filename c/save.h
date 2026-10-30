#ifndef SAVE_H
#define SAVE_H
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define COUNT_ENCOUNTER_CAPACITY 128
#define COUNT_ENCOUNTER_BYTES (COUNT_ENCOUNTER_CAPACITY / 8)

// Struct for storing additional data in SRAM. This has to always be a multiple of 2 bytes long supposedly.
typedef struct {
    uint8_t silver_rupee_rooms_cleared;
    uint8_t trap_count;
    uint8_t gold_rupee_count;
    uint8_t enemy_counts[9];
} extended_savecontext_static_t;

typedef enum {
    OBJECTIVE_DODONGO_LIZALFOS,
    OBJECTIVE_BIG_OCTO,
    OBJECTIVE_POE_AMY,
    OBJECTIVE_POE_JOELLE,
    OBJECTIVE_POE_BETH,
    OBJECTIVE_POE_MEG,
    OBJECTIVE_FOREST_BOSS_DOOR,
    OBJECTIVE_FIRE_FLARE_DANCERS,
    OBJECTIVE_FIRE_GOLD_RUPEE,
    OBJECTIVE_DARK_LINK,
    OBJECTIVE_NABOORU_KNUCKLE,
    OBJECTIVE_SPIRIT_BOMBCHU_CHEST,
    OBJECTIVE_SPIRIT_PLATFORM,
    OBJECTIVE_SPIRIT_RUSTY_SWITCH_1,
    OBJECTIVE_SPIRIT_RUSTY_SWITCH_2,
    OBJECTIVE_GANON_BOSS_DOOR,
    OBJECTIVE_SKY_TEMPLE,
    OBJECTIVE_BOMBCHU_BOWLING,
    OBJECTIVE_COW_0,
    OBJECTIVE_COW_1,
    OBJECTIVE_COW_2,
    OBJECTIVE_COW_3,
    OBJECTIVE_COW_4,
    OBJECTIVE_COW_5,
    OBJECTIVE_COW_6,
    OBJECTIVE_COW_7,
    OBJECTIVE_COW_8,
    OBJECTIVE_COW_9,
    OBJECTIVE_COW_10,
    OBJECTIVE_COW_11,
    OBJECTIVE_COW_12,
    OBJECTIVE_COW_13,
    OBJECTIVE_COW_14,
    OBJECTIVE_COW_15,
    OBJECTIVE_SOIL_GS_0,
    OBJECTIVE_SOIL_GS_1,
    OBJECTIVE_SOIL_GS_2,
    OBJECTIVE_SOIL_GS_3,
    OBJECTIVE_SOIL_GS_4,
    OBJECTIVE_SOIL_GS_5,
    OBJECTIVE_SOIL_GS_6,
    OBJECTIVE_SOIL_GS_7,
    OBJECTIVE_SOIL_GS_8,
    OBJECTIVE_SOIL_GS_9,
    OBJECTIVE_SPIRIT_MQ_RUSTY_SWITCH_3,
    OBJECTIVE_SPIRIT_MQ_RUSTY_SWITCH_4,
    OBJECTIVE_COUNT
} count_objective_t;

extern extended_savecontext_static_t extended_savectx;

void count_save_cache_sram(const void* sram_buffer);
void count_save_load_file();
void count_save_initialize_file(uint8_t file_index, void* sram_buffer);
void count_save_write_new_file(uint32_t address, void* sram_buffer, size_t size, uint32_t direction);
void count_save_write();
bool count_encounter_is_complete(uint16_t encounter_id);
void count_encounter_complete(uint16_t encounter_id);
bool count_objective_is_complete(uint8_t objective_id);
void count_objective_complete(uint8_t objective_id);

#endif
