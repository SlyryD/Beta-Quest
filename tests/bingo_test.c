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
    return (area_index < 16 && count_index < 2) ? area_counts[area_index][count_index] : 0xFF;
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

bool get_trial_beam_cleared(trial_t trial) {
    return trial < TRIAL_COUNT && global_counts[GCOUNT_TRIAL_BEAMS] > trial;
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

uint8_t get_tokens_with_mask(gs_flags_index_t index, uint8_t mask) {
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
    z64_file.rupees = 501;
    assert(!bingo_goal_is_complete(BINGO_GOAL_N_500_RUPEES));
    z64_file.rupees = 10;
    assert(!bingo_goal_is_complete(BINGO_GOAL_N_500_RUPEES));

    z64_file.items[Z64_SLOT_NUT] = Z64_ITEM_FAIRY;
    assert(bingo_goal_is_complete(BINGO_GOAL_BOTTLED_FAIRY));
    z64_file.items[Z64_SLOT_NUT] = Z64_ITEM_NULL;

    z64_file.items[Z64_SLOT_STICK] = Z64_ITEM_LETTER;
    assert(bingo_goal_is_complete(BINGO_GOAL_RUTO_S_LETTER));
    z64_file.items[Z64_SLOT_STICK] = Z64_ITEM_NULL;

    z64_file.items[Z64_SLOT_NUT] = Z64_ITEM_HALF_MILK;
    assert(!bingo_goal_is_complete(BINGO_GOAL_MILK));
    z64_file.items[Z64_SLOT_NUT] = Z64_ITEM_MILK;
    assert(bingo_goal_is_complete(BINGO_GOAL_MILK));
    z64_file.items[Z64_SLOT_NUT] = Z64_ITEM_NULL;

    for (uint8_t slot = Z64_SLOT_BOTTLE_1; slot <= Z64_SLOT_BOTTLE_4; slot++) {
        z64_file.items[slot] = Z64_ITEM_RED_POTION;
    }
    assert(bingo_goal_is_complete(BINGO_GOAL_FILL_ALL_4_BOTTLE_SLOTS));
    assert(bingo_goal_is_complete(BINGO_GOAL_RED_POTION));

    z64_file.items[Z64_SLOT_BOTTLE_1] = Z64_ITEM_BOW;
    z64_file.items[Z64_SLOT_BOTTLE_2] = Z64_ITEM_WEIRD_EGG;
    z64_file.items[Z64_SLOT_BOTTLE_3] = Z64_ITEM_HAMMER;
    z64_file.items[Z64_SLOT_BOTTLE_4] = Z64_ITEM_FISHING_POLE;
    assert(bingo_goal_is_complete(BINGO_GOAL_FILL_ALL_4_BOTTLE_SLOTS));

    z64_file.items[Z64_SLOT_BOTTLE_4] = Z64_ITEM_NULL;
    assert(!bingo_goal_is_complete(BINGO_GOAL_FILL_ALL_4_BOTTLE_SLOTS));
}

static void test_boss_door_goals_use_scene_switches(void) {
    assert(!bingo_goal_is_complete(BINGO_GOAL_UNLOCK_PHANTOM_GANON_S_BOSS_DOOR));
    assert(!bingo_goal_is_complete(BINGO_GOAL_OPEN_GANON_S_BOSS_DOOR));

    z64_file.scene_flags[SCENE_FOREST_TEMPLE].swch |= 1u << BOSS_DOOR_SWITCH_FLAG;
    assert(bingo_goal_is_complete(BINGO_GOAL_UNLOCK_PHANTOM_GANON_S_BOSS_DOOR));
    assert(!bingo_goal_is_complete(BINGO_GOAL_OPEN_GANON_S_BOSS_DOOR));

    z64_file.scene_flags[SCENE_GANONS_TOWER].swch |= 1u << BOSS_DOOR_SWITCH_FLAG;
    assert(bingo_goal_is_complete(BINGO_GOAL_OPEN_GANON_S_BOSS_DOOR));
}

static void test_exact_upgrade_and_ammo_goals(void) {
    static const bingo_goal_t bean_goals[] = {
        BINGO_GOAL_N_5_MAGIC_BEANS,
        BINGO_GOAL_N_6_MAGIC_BEANS,
        BINGO_GOAL_N_7_MAGIC_BEANS,
    };

    z64_file.diving_upgrade = 1;
    assert(bingo_get_scale_level() == 1);
    assert(bingo_goal_is_complete(BINGO_GOAL_SILVER_SCALE));
    z64_file.diving_upgrade = 2;
    assert(bingo_get_scale_level() == 2);
    assert(!bingo_goal_is_complete(BINGO_GOAL_SILVER_SCALE));
    assert(bingo_goal_is_complete(BINGO_GOAL_GOLDEN_SCALE));
    z64_file.diving_upgrade = 3;
    assert(!bingo_goal_is_complete(BINGO_GOAL_GOLDEN_SCALE));

    z64_file.wallet = 1;
    assert(bingo_goal_is_complete(BINGO_GOAL_ADULT_S_WALLET));
    z64_file.wallet = 2;
    assert(!bingo_goal_is_complete(BINGO_GOAL_ADULT_S_WALLET));
    assert(bingo_goal_is_complete(BINGO_GOAL_GIANT_S_WALLET));
    z64_file.wallet = 3;
    assert(!bingo_goal_is_complete(BINGO_GOAL_GIANT_S_WALLET));

    for (uint8_t i = 0; i < sizeof(bean_goals) / sizeof(bean_goals[0]); i++) {
        z64_file.ammo[Z64_SLOT_BEANS] = 5 + i;
        assert(bingo_goal_is_complete(bean_goals[i]));
        z64_file.ammo[Z64_SLOT_BEANS]++;
        assert(!bingo_goal_is_complete(bean_goals[i]));
    }

    z64_file.ammo[Z64_SLOT_NUT] = 30;
    assert(bingo_goal_is_complete(BINGO_GOAL_N_30_DEKU_NUTS));
    z64_file.ammo[Z64_SLOT_NUT] = 31;
    assert(!bingo_goal_is_complete(BINGO_GOAL_N_30_DEKU_NUTS));
}

static void test_equipment_set_progress(void) {
    global_counts[GCOUNT_SHIELD] = 3;
    bingo_progress_t progress = bingo_get_goal_progress(BINGO_GOAL_N_3_SHIELDS_3_TUNICS);
    assert(progress.current == 1);
    assert(progress.target == 2);
    assert(!progress.complete);

    global_counts[GCOUNT_TUNIC] = 3;
    progress = bingo_get_goal_progress(BINGO_GOAL_N_3_SHIELDS_3_TUNICS);
    assert(progress.current == 2);
    assert(progress.target == 2);
    assert(progress.complete);

    z64_file.kokiri_sword = 1;
    z64_file.master_sword = 1;
    z64_file.giants_knife = 1;
    global_counts[GCOUNT_BOOT] = 3;
    progress = bingo_get_goal_progress(BINGO_GOAL_N_3_SWORDS_TUNICS_BOOTS_AND_SHIELDS);
    assert(progress.current == 4);
    assert(progress.target == 4);
    assert(progress.complete);
}

static void test_trial_ends_are_distinct_from_beams(void) {
    static const bingo_goal_t goals[] = {
        BINGO_GOAL_GET_TO_THE_END_OF_LIGHT_TRIAL,
        BINGO_GOAL_GET_TO_THE_END_OF_FIRE_TRIAL,
        BINGO_GOAL_GET_TO_THE_END_OF_SPIRIT_TRIAL,
        BINGO_GOAL_GET_TO_THE_END_OF_FOREST_TRIAL,
        BINGO_GOAL_GET_TO_THE_END_OF_WATER_TRIAL,
        BINGO_GOAL_GET_TO_THE_END_OF_SHADOW_TRIAL,
    };

    global_counts[GCOUNT_TRIAL_BEAMS] = 6;
    for (uint8_t trial = 0; trial < 6; trial++) {
        assert(!bingo_goal_is_complete(goals[trial]));
        objectives[OBJECTIVE_TRIAL_LIGHT_END + trial] = true;
        assert(bingo_goal_is_complete(goals[trial]));
    }
}

static void test_upgrade_goals_require_exact_levels(void) {
    assert(!bingo_item_owned(Z64_ITEM_BOW));
    z64_file.bullet_bag = 2;
    assert(bingo_goal_is_complete(BINGO_GOAL_BULLET_BAG_40));
    assert(!bingo_goal_is_complete(BINGO_GOAL_BULLET_BAG_50));
    z64_file.bullet_bag = 3;
    assert(!bingo_goal_is_complete(BINGO_GOAL_BULLET_BAG_40));
    assert(bingo_goal_is_complete(BINGO_GOAL_BULLET_BAG_50));
    assert(!bingo_item_owned(Z64_ITEM_BULLET_BAG_40));
    assert(bingo_item_owned(Z64_ITEM_BULLET_BAG_50));

    z64_file.bomb_bag = 2;
    assert(bingo_goal_is_complete(BINGO_GOAL_BOMB_BAG_30));
    z64_file.bomb_bag = 3;
    assert(!bingo_goal_is_complete(BINGO_GOAL_BOMB_BAG_30));
    assert(!bingo_item_owned(Z64_ITEM_BOMB_BAG_30));
    assert(bingo_item_owned(Z64_ITEM_BOMB_BAG_40));

    z64_file.quiver = 2;
    assert(bingo_goal_is_complete(BINGO_GOAL_QUIVER_40));
    z64_file.quiver = 3;
    assert(!bingo_goal_is_complete(BINGO_GOAL_QUIVER_40));
    assert(!bingo_item_owned(Z64_ITEM_QUIVER_40));
    assert(bingo_item_owned(Z64_ITEM_QUIVER_50));

    z64_file.items[Z64_SLOT_BOW] = Z64_ITEM_NULL;
    z64_file.items[Z64_SLOT_SLINGSHOT] = Z64_ITEM_NULL;
    z64_file.items[Z64_SLOT_BOMB] = Z64_ITEM_NULL;
    z64_file.stick_upgrade = 3;
    z64_file.nut_upgrade = 3;
    assert(bingo_get_capacity_level(Z64_ITEM_BOW) == 3);
    assert(bingo_get_capacity_level(Z64_ITEM_SLINGSHOT) == 3);
    assert(bingo_get_capacity_level(Z64_ITEM_BOMB) == 3);
    assert(bingo_get_capacity_level(Z64_ITEM_STICK) == 3);
    assert(bingo_get_capacity_level(Z64_ITEM_NUT) == 3);
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
        assert(bingo_get_strength_level() == strength);
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
    test_boss_door_goals_use_scene_switches();
    reset_state();
    test_exact_upgrade_and_ammo_goals();
    reset_state();
    test_equipment_set_progress();
    reset_state();
    test_trial_ends_are_distinct_from_beams();
    reset_state();
    test_upgrade_goals_require_exact_levels();
    reset_state();
    reset_state();
    test_current_key_aggregate_rolls_back();
    reset_state();
    test_variant_rusty_switch_target();
    reset_state();
    test_strength_colors_are_exact();
    return 0;
}
