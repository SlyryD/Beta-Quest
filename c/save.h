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
    uint8_t enemy_counts[8];
    uint8_t pad;
} extended_savecontext_static_t;

extern extended_savecontext_static_t extended_savectx;

void count_save_cache_sram(const void* sram_buffer);
void count_save_load_file();
void count_save_initialize_file(uint8_t file_index, void* sram_buffer);
void count_save_write();
bool count_encounter_is_complete(uint16_t encounter_id);
void count_encounter_complete(uint16_t encounter_id);

#endif
