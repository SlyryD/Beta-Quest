#include <assert.h>
#include <stdint.h>
#include <string.h>

#define Z64_H
#define COUNT_SAVE_TEST

typedef struct {
    uint8_t file_index;
    uint16_t checksum;
} test_file_t;

static test_file_t z64_file;
static uint8_t test_sram[0x8000];

void count_save_test_sram_read_write(
    uint32_t address,
    void* dram_address,
    size_t size,
    uint32_t direction
) {
    size_t offset = address - 0x08000000;

    assert(direction == 1);
    assert(offset + size <= sizeof(test_sram));
    memcpy(test_sram + offset, dram_address, size);
}

#include "../c/save.c"

static void reset_test_state(uint8_t file_index, uint16_t checksum) {
    memset(test_sram, 0, sizeof(test_sram));
    memset(cached_records, 0, sizeof(cached_records));
    clear_working_record();
    loaded_file_index = -1;
    z64_file.file_index = file_index;
    z64_file.checksum = checksum;
}

static count_save_record_t* primary_record(uint8_t file_index) {
    return (count_save_record_t*)(
        test_sram + SRAM_EXTENSION_START +
        file_index * SRAM_EXTENSION_RECORD_SIZE
    );
}

static count_save_record_t* backup_record(uint8_t file_index) {
    return primary_record(file_index + 3);
}

static void test_initialize_and_round_trip() {
    reset_test_state(1, 0x1234);
    count_save_initialize_file(1, test_sram);

    assert(is_valid_record(primary_record(1), 0x1234));
    assert(is_valid_record(backup_record(1), 0x1234));

    extended_savectx.trap_count = 7;
    extended_savectx.gold_rupee_count = 3;
    extended_savectx.silver_rupee_rooms_cleared = 4;
    extended_savectx.enemy_counts[2] = 9;
    count_encounter_complete(73);
    count_save_write();

    assert(memcmp(
        primary_record(1),
        backup_record(1),
        sizeof(count_save_record_t)
    ) == 0);

    clear_working_record();
    count_save_cache_sram(test_sram);
    count_save_load_file();

    assert(extended_savectx.trap_count == 7);
    assert(extended_savectx.gold_rupee_count == 3);
    assert(extended_savectx.silver_rupee_rooms_cleared == 4);
    assert(extended_savectx.enemy_counts[2] == 9);
    assert(count_encounter_is_complete(73));
}

static void test_backup_fallback() {
    primary_record(1)->checksum++;
    clear_working_record();
    count_save_cache_sram(test_sram);
    count_save_load_file();

    assert(extended_savectx.trap_count == 7);
    assert(extended_savectx.enemy_counts[2] == 9);
    assert(count_encounter_is_complete(73));
}

static void test_invalid_records_clear_state() {
    backup_record(1)->checksum++;
    extended_savectx.trap_count = 99;
    count_encounter_complete(12);
    count_save_cache_sram(test_sram);
    count_save_load_file();

    assert(extended_savectx.trap_count == 0);
    assert(!count_encounter_is_complete(12));
    assert(!count_encounter_is_complete(73));
}

static void test_unsaved_changes_roll_back() {
    reset_test_state(0, 0xCAFE);
    count_save_initialize_file(0, test_sram);
    extended_savectx.gold_rupee_count = 5;
    count_encounter_complete(40);
    count_save_write();

    extended_savectx.gold_rupee_count = 8;
    count_encounter_complete(41);
    count_save_cache_sram(test_sram);
    count_save_load_file();

    assert(extended_savectx.gold_rupee_count == 5);
    assert(count_encounter_is_complete(40));
    assert(!count_encounter_is_complete(41));
}

static void test_encounter_bounds() {
    assert(!count_encounter_is_complete(COUNT_ENCOUNTER_CAPACITY));
    count_encounter_complete(COUNT_ENCOUNTER_CAPACITY);
    assert(!count_encounter_is_complete(COUNT_ENCOUNTER_CAPACITY));
}

static void test_file_copy_and_recreate() {
    reset_test_state(0, 0xBEEF);
    count_save_initialize_file(0, test_sram);
    extended_savectx.trap_count = 11;
    count_encounter_complete(55);
    count_save_write();

    z64_file.file_index = 1;
    count_save_cache_sram(test_sram);
    count_save_load_file();
    assert(extended_savectx.trap_count == 11);
    assert(count_encounter_is_complete(55));

    count_save_write();
    assert(is_valid_record(primary_record(1), 0xBEEF));
    assert(primary_record(1)->counts.trap_count == 11);

    count_save_initialize_file(1, test_sram);
    assert(extended_savectx.trap_count == 0);
    assert(!count_encounter_is_complete(55));
    assert(primary_record(1)->counts.trap_count == 0);
    assert(backup_record(1)->counts.trap_count == 0);
}

int main() {
    test_initialize_and_round_trip();
    test_backup_fallback();
    test_invalid_records_clear_state();
    test_unsaved_changes_roll_back();
    test_encounter_bounds();
    test_file_copy_and_recreate();
    return 0;
}
