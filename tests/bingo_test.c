#include <assert.h>
#include <stdint.h>
#include <string.h>
#include <sys/mman.h>

#include "../c/bingo.h"
#include "../c/counts.h"
#include "../c/save.h"
#include "../c/z64.h"

#define TEST_MEMORY_START ((void*)0x80100000)
#define TEST_MEMORY_SIZE 0x00100000

static uint8_t global_counts[256];
static uint8_t dungeon_counts[256][3];
static uint8_t area_counts[16][2];
static bool objectives[OBJECTIVE_COUNT];
static bool dungeon_complete[20];
static bool boss_clear[20];
static uint8_t rusty_switches;

uint8_t get_global_count(uint8_t count_index) {
    return global_counts[count_index];
}

uint8_t get_fire_goron_total(void) {
    return z64_file.master_quest ? 4 : 9;
}

uint8_t get_dungeon_count(uint8_t dungeon_index, uint8_t count_index) {
    if (dungeon_index >= 256 || count_index == DCOUNT_SMALL_KEY) {
        return dungeon_index < 256 ? dungeon_counts[dungeon_index][2] : 0xFF;
    }
    return dungeon_counts[dungeon_index][count_index];
}

uint8_t get_area_count(uint8_t area_index, uint8_t count_index) {
    return area_index < 16 && count_index < 2 ? area_counts[area_index][count_index] : 0xFF;
}

uint8_t get_current_area_index(void) {
    return AREA_HYRULE_FIELD;
}

bool count_tracker_is_dungeon_completed(uint8_t dungeon_index) {
    return dungeon_complete[dungeon_index];
}

bool count_tracker_is_boss_cleared(uint8_t dungeon_index) {
    return boss_clear[dungeon_index];
}

uint8_t count_tracker_get_spirit_rusty_switch_count(void) {
    return rusty_switches;
}

bool count_objective_is_complete(uint8_t objective_id) {
    return objective_id < OBJECTIVE_COUNT && objectives[objective_id];
}

uint8_t count_completed_objectives(uint8_t first, uint8_t count) {
    uint8_t completed = 0;
    for (uint8_t i = 0; i < count; i++) {
        completed += count_objective_is_complete(first + i);
    }
    return completed;
}

bool get_trial_beam_cleared(uint8_t trial_index) {
    return trial_index < 6 && global_counts[GCOUNT_TRIAL_BEAMS] > trial_index;
}

bool count_scene_switch_set(uint8_t scene, uint8_t flag) {
    return z64_file.scene_flags[scene].swch & (1u << flag);
}

bool count_scene_chest_opened(uint8_t scene, uint8_t flag) {
    return z64_file.scene_flags[scene].chest & (1u << flag);
}

bool count_scene_collect_set(uint8_t scene, uint8_t flag) {
    return z64_file.scene_flags[scene].collect & (1u << flag);
}

bool count_event_check_set(uint8_t flag) {
    return z64_file.event_chk_inf[flag >> 4] & (1u << (flag & 0xF));
}

uint8_t get_tokens_with_mask(uint8_t index, uint8_t mask) {
    (void)index;
    return global_counts[0xFE] & mask;
}

static void reset_state(void) {
    memset((void*)TEST_MEMORY_START, 0, TEST_MEMORY_SIZE);
    memset(global_counts, 0, sizeof(global_counts));
    memset(dungeon_counts, 0, sizeof(dungeon_counts));
    memset(area_counts, 0, sizeof(area_counts));
    memset(objectives, 0, sizeof(objectives));
    memset(dungeon_complete, 0, sizeof(dungeon_complete));
    memset(boss_clear, 0, sizeof(boss_clear));
    rusty_switches = 0;
    for (uint8_t scene = 0; scene < 19; scene++) {
        z64_file.dungeon_keys[scene] = -1;
    }
    for (uint8_t slot = 0; slot < 24; slot++) {
        z64_file.items[slot] = Z64_ITEM_NULL;
    }
}

static void test_all_goals_have_evaluators(void) {
    for (int goal = 0; goal < BINGO_GOAL_COUNT; goal++) {
        bingo_progress_t progress = bingo_get_goal_progress((bingo_goal_t)goal);
        assert(progress.target > 0);
        assert(progress.complete == (progress.current >= progress.target) ||
               goal == BINGO_GOAL_EXACTLY_20_DEKU_STICKS ||
               goal == BINGO_GOAL_EXACTLY_30_DEKU_STICKS);
    }
    assert(bingo_get_goal_progress(BINGO_GOAL_COUNT).target == 0);
}

static void test_exact_sticks_roll_back(void) {
    z64_file.ammo[Z64_SLOT_STICK] = 19;
    assert(!bingo_goal_is_complete(BINGO_GOAL_EXACTLY_20_DEKU_STICKS));
    z64_file.ammo[Z64_SLOT_STICK] = 20;
    assert(bingo_goal_is_complete(BINGO_GOAL_EXACTLY_20_DEKU_STICKS));
    z64_file.ammo[Z64_SLOT_STICK] = 21;
    assert(!bingo_goal_is_complete(BINGO_GOAL_EXACTLY_20_DEKU_STICKS));
}

static void test_live_rupees_and_bottles(void) {
    z64_file.rupees = 499;
    assert(!bingo_goal_is_complete(BINGO_GOAL_N_500_RUPEES));
    z64_file.rupees = 500;
    assert(bingo_goal_is_complete(BINGO_GOAL_N_500_RUPEES));
    z64_file.rupees = 10;
    assert(!bingo_goal_is_complete(BINGO_GOAL_N_500_RUPEES));

    for (uint8_t slot = Z64_SLOT_BOTTLE_1; slot <= Z64_SLOT_BOTTLE_4; slot++) {
        z64_file.items[slot] = Z64_ITEM_RED_POTION;
    }
    assert(bingo_goal_is_complete(BINGO_GOAL_FILL_ALL_4_BOTTLE_SLOTS));
    assert(bingo_goal_is_complete(BINGO_GOAL_RED_POTION));
    z64_file.items[Z64_SLOT_BOTTLE_4] = Z64_ITEM_NULL;
    assert(!bingo_goal_is_complete(BINGO_GOAL_FILL_ALL_4_BOTTLE_SLOTS));
}

static void test_current_key_aggregate_rolls_back(void) {
    const uint8_t scenes[] = {
        SCENE_FOREST_TEMPLE, SCENE_FIRE_TEMPLE, SCENE_WATER_TEMPLE, SCENE_SHADOW_TEMPLE, SCENE_SPIRIT_TEMPLE,
    };
    for (uint8_t i = 0; i < sizeof(scenes); i++) {
        z64_file.dungeon_keys[scenes[i]] = 1;
    }
    assert(bingo_goal_is_complete(BINGO_GOAL_N_1_UNUSED_SMALL_KEY_IN_EACH_ADULT_DUNGEON));
    z64_file.dungeon_keys[SCENE_WATER_TEMPLE] = 0;
    assert(!bingo_goal_is_complete(BINGO_GOAL_N_1_UNUSED_SMALL_KEY_IN_EACH_ADULT_DUNGEON));
}

static void test_variant_rusty_switch_target(void) {
    rusty_switches = 2;
    z64_file.master_quest = 0;
    bingo_progress_t vanilla = bingo_get_goal_progress(BINGO_GOAL_BOTH_RUSTY_SWITCHES_IN_SPIRIT_TEMPLE);
    assert(vanilla.target == 2 && vanilla.complete);

    z64_file.master_quest = 1;
    bingo_progress_t mq = bingo_get_goal_progress(BINGO_GOAL_BOTH_RUSTY_SWITCHES_IN_SPIRIT_TEMPLE);
    assert(mq.target == 4 && !mq.complete);
    rusty_switches = 4;
    assert(bingo_goal_is_complete(BINGO_GOAL_BOTH_RUSTY_SWITCHES_IN_SPIRIT_TEMPLE));
}

static void test_strength_colors_are_exact(void) {
    static const bingo_goal_t goals[] = {
        BINGO_GOAL_GORON_BRACELET,
        BINGO_GOAL_SILVER_GAUNTLETS,
        BINGO_GOAL_GOLDEN_GAUNTLETS,
        BINGO_GOAL_BLACK_GAUNTLETS,
        BINGO_GOAL_GREEN_GAUNTLETS,
        BINGO_GOAL_BLUE_GAUNTLETS,
    };
    static const uint8_t values[] = { 1, 2, 3, 4, 5, 6 };

    for (uint8_t strength = 0; strength < 8; strength++) {
        z64_file.strength_upgrade = strength;
        for (uint8_t goal = 0; goal < sizeof(goals) / sizeof(*goals); goal++) {
            assert(bingo_goal_is_complete(goals[goal]) == (strength == values[goal]));
        }
    }
}

int main(void) {
    void* memory =
        mmap(TEST_MEMORY_START, TEST_MEMORY_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED, -1, 0);
    assert(memory == TEST_MEMORY_START);

    reset_state();
    test_all_goals_have_evaluators();
    reset_state();
    test_exact_sticks_roll_back();
    reset_state();
    test_live_rupees_and_bottles();
    reset_state();
    test_current_key_aggregate_rolls_back();
    reset_state();
    test_variant_rusty_switch_target();
    reset_state();
    test_strength_colors_are_exact();
    return 0;
}
