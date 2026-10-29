#include "save.h"

#include "memory.h"
#include "z64.h"

#define SRAM_BASE 0x08000000
#define SRAM_SIZE 0x8000
#define SRAM_FILE_SIZE 0x1450
#define SRAM_PRIMARY_FILES_START 0x20
#define SRAM_BACKUP_FILES_START 0x3D10
#define SRAM_EXTENSION_START 0x7A00
#define SRAM_EXTENSION_RECORD_SIZE 0x30
#define SRAM_EXTENSION_RECORD_COUNT 6
#define COUNT_SAVE_MAGIC 0x42514354
#define COUNT_SAVE_VERSION 3
#define OS_READ 0
#define OS_WRITE 1

typedef void (*sram_read_write_func_t)(uint32_t address, void* dram_address, size_t size, uint32_t direction);

typedef struct {
    uint32_t magic;
    uint16_t version;
    uint16_t size;
    uint16_t base_checksum;
    uint16_t checksum;
    extended_savecontext_static_t counts;
    uint8_t completed_encounters[COUNT_ENCOUNTER_BYTES];
    uint64_t completed_objectives;
} count_save_record_t;

#ifdef COUNT_SAVE_TEST
extern void count_save_test_sram_read_write(uint32_t address, void* dram_address, size_t size, uint32_t direction);
static sram_read_write_func_t const sram_read_write = count_save_test_sram_read_write;
#else
static sram_read_write_func_t const sram_read_write = (sram_read_write_func_t)0x80091474;
#endif

extended_savecontext_static_t extended_savectx;
static uint8_t completed_encounters[COUNT_ENCOUNTER_BYTES];
static uint64_t completed_objectives;
static count_save_record_t cached_records[SRAM_EXTENSION_RECORD_COUNT];
static int8_t loaded_file_index = -1;

typedef char assert_extended_silver_rupee_rooms_cleared_offset
    [__builtin_offsetof(extended_savecontext_static_t, silver_rupee_rooms_cleared) == 0x00 ? 1 : -1];
typedef char
    assert_extended_trap_count_offset[__builtin_offsetof(extended_savecontext_static_t, trap_count) == 0x01 ? 1 : -1];
typedef char assert_extended_gold_rupee_count_offset
    [__builtin_offsetof(extended_savecontext_static_t, gold_rupee_count) == 0x02 ? 1 : -1];
typedef char assert_extended_enemy_counts_offset[__builtin_offsetof(extended_savecontext_static_t, enemy_counts) == 0x03
                                                     ? 1
                                                     : -1];
typedef char assert_extended_savecontext_size[sizeof(extended_savecontext_static_t) == 0x0C ? 1 : -1];
typedef char assert_count_save_record_size[sizeof(count_save_record_t) == SRAM_EXTENSION_RECORD_SIZE ? 1 : -1];
typedef char assert_count_objective_capacity[OBJECTIVE_COUNT <= 64 ? 1 : -1];
typedef char assert_sram_extension_bounds
    [SRAM_EXTENSION_START + SRAM_EXTENSION_RECORD_COUNT * SRAM_EXTENSION_RECORD_SIZE <= SRAM_SIZE ? 1 : -1];

static void clear_bytes(void* destination, size_t size) {
    uint8_t* destination_bytes = destination;

    for (size_t i = 0; i < size; i++) {
        destination_bytes[i] = 0;
    }
}

static uint16_t calculate_checksum(const count_save_record_t* record) {
    const uint16_t* values = (const uint16_t*)record;
    uint16_t checksum = 0;

    for (size_t i = 0; i < sizeof(*record) / sizeof(*values); i++) {
        if (i != __builtin_offsetof(count_save_record_t, checksum) / sizeof(*values)) {
            checksum += values[i];
        }
    }

    return checksum;
}

static bool is_valid_record(const count_save_record_t* record, uint16_t base_checksum) {
    return record->magic == COUNT_SAVE_MAGIC && record->version == COUNT_SAVE_VERSION &&
           record->size == sizeof(*record) && record->base_checksum == base_checksum &&
           record->checksum == calculate_checksum(record);
}

static void make_record(count_save_record_t* record) {
    clear_bytes(record, sizeof(*record));
    record->magic = COUNT_SAVE_MAGIC;
    record->version = COUNT_SAVE_VERSION;
    record->size = sizeof(*record);
    record->base_checksum = z64_file.checksum;
    memory_copy(&record->counts, &extended_savectx, sizeof(record->counts));
    memory_copy(record->completed_encounters, completed_encounters, sizeof(record->completed_encounters));
    record->completed_objectives = completed_objectives;
    record->checksum = calculate_checksum(record);
}

static void load_record(const count_save_record_t* record) {
    memory_copy(&extended_savectx, &record->counts, sizeof(extended_savectx));
    memory_copy(completed_encounters, record->completed_encounters, sizeof(completed_encounters));
    completed_objectives = record->completed_objectives;
}

static void clear_working_record() {
    clear_bytes(&extended_savectx, sizeof(extended_savectx));
    clear_bytes(completed_encounters, sizeof(completed_encounters));
    completed_objectives = 0;
}

void count_save_cache_sram(const void* sram_buffer) {
    const uint8_t* extension = (const uint8_t*)sram_buffer + SRAM_EXTENSION_START;

    memory_copy(cached_records, extension, sizeof(cached_records));
    loaded_file_index = -1;
}

void count_save_load_file() {
    uint8_t file_index = z64_file.file_index;
    const count_save_record_t* record = NULL;

    if (file_index >= 3 || loaded_file_index == file_index) {
        return;
    }

    if (is_valid_record(&cached_records[file_index], z64_file.checksum)) {
        record = &cached_records[file_index];
    } else if (is_valid_record(&cached_records[file_index + 3], z64_file.checksum)) {
        record = &cached_records[file_index + 3];
    } else {
        for (uint8_t i = 0; i < SRAM_EXTENSION_RECORD_COUNT; i++) {
            if (i == file_index || i == file_index + 3) {
                continue;
            }
            if (is_valid_record(&cached_records[i], z64_file.checksum)) {
                record = &cached_records[i];
                break;
            }
        }
    }

    if (record == NULL) {
        clear_working_record();
    } else {
        load_record(record);
    }
    loaded_file_index = file_index;
}

void count_save_initialize_file(uint8_t file_index, void* sram_buffer) {
    count_save_record_t* primary_record = &cached_records[file_index];
    count_save_record_t* backup_record = &cached_records[file_index + 3];

    clear_working_record();
    loaded_file_index = file_index;
    make_record(primary_record);
    memory_copy(backup_record, primary_record, sizeof(*primary_record));

    memory_copy((uint8_t*)sram_buffer + SRAM_EXTENSION_START + file_index * SRAM_EXTENSION_RECORD_SIZE, primary_record,
                sizeof(*primary_record));
    memory_copy((uint8_t*)sram_buffer + SRAM_EXTENSION_START + (file_index + 3) * SRAM_EXTENSION_RECORD_SIZE,
                backup_record, sizeof(*backup_record));
}

void count_save_write_new_file(uint32_t address, void* sram_buffer, size_t size, uint32_t direction) {
    uint8_t file_index = z64_file.file_index;
    size_t primary_offset;
    size_t backup_offset;
    size_t primary_count_offset;
    size_t backup_count_offset;

    if (address != SRAM_BASE || size != SRAM_SIZE || direction != OS_WRITE || file_index >= 3) {
        sram_read_write(address, sram_buffer, size, direction);
        return;
    }

    count_save_initialize_file(file_index, sram_buffer);
    primary_offset = SRAM_PRIMARY_FILES_START + file_index * SRAM_FILE_SIZE;
    backup_offset = SRAM_BACKUP_FILES_START + file_index * SRAM_FILE_SIZE;
    primary_count_offset = SRAM_EXTENSION_START + file_index * SRAM_EXTENSION_RECORD_SIZE;
    backup_count_offset = SRAM_EXTENSION_START + (file_index + 3) * SRAM_EXTENSION_RECORD_SIZE;

    sram_read_write(SRAM_BASE + primary_offset, (uint8_t*)sram_buffer + primary_offset, SRAM_FILE_SIZE, OS_WRITE);
    sram_read_write(SRAM_BASE + backup_offset, (uint8_t*)sram_buffer + backup_offset, SRAM_FILE_SIZE, OS_WRITE);
    sram_read_write(SRAM_BASE + primary_count_offset, (uint8_t*)sram_buffer + primary_count_offset,
                    SRAM_EXTENSION_RECORD_SIZE, OS_WRITE);
    sram_read_write(SRAM_BASE + backup_count_offset, (uint8_t*)sram_buffer + backup_count_offset,
                    SRAM_EXTENSION_RECORD_SIZE, OS_WRITE);
}

void count_save_write() {
    uint8_t file_index = z64_file.file_index;

    if (file_index >= 3) {
        return;
    }

    count_save_record_t* primary_record = &cached_records[file_index];
    count_save_record_t* backup_record = &cached_records[file_index + 3];

    make_record(primary_record);
    memory_copy(backup_record, primary_record, sizeof(*primary_record));

    sram_read_write(SRAM_BASE + SRAM_EXTENSION_START + file_index * SRAM_EXTENSION_RECORD_SIZE, primary_record,
                    sizeof(*primary_record), OS_WRITE);
    sram_read_write(SRAM_BASE + SRAM_EXTENSION_START + (file_index + 3) * SRAM_EXTENSION_RECORD_SIZE, backup_record,
                    sizeof(*backup_record), OS_WRITE);
}

bool count_encounter_is_complete(uint16_t encounter_id) {
    if (encounter_id >= COUNT_ENCOUNTER_CAPACITY) {
        return false;
    }

    return completed_encounters[encounter_id >> 3] & (1 << (encounter_id & 7));
}

void count_encounter_complete(uint16_t encounter_id) {
    if (encounter_id < COUNT_ENCOUNTER_CAPACITY) {
        completed_encounters[encounter_id >> 3] |= 1 << (encounter_id & 7);
    }
}

bool count_objective_is_complete(uint8_t objective_id) {
    return objective_id < OBJECTIVE_COUNT && (completed_objectives & ((uint64_t)1 << objective_id));
}

void count_objective_complete(uint8_t objective_id) {
    if (objective_id < OBJECTIVE_COUNT) {
        completed_objectives |= (uint64_t)1 << objective_id;
    }
}
