#include <assert.h>
#include <stdint.h>
#include <string.h>

#define Z64_H
#define COUNT_TRACKER_TEST

enum {
    ACTORTYPE_SWITCH,
    ACTORTYPE_BG,
    ACTORTYPE_PLAYER,
    ACTORTYPE_EXPLOSIVES,
    ACTORTYPE_NPC,
    ACTORTYPE_ENEMY,
    ACTORTYPE_PROP,
    ACTORTYPE_ITEMACTION,
    ACTORTYPE_MISC,
    ACTORTYPE_BOSS,
    ACTORTYPE_DOOR,
    ACTORTYPE_CHEST,
};

typedef struct z64_actor_s z64_actor_t;
struct z64_actor_s {
    int16_t actor_id;
    int8_t room_index;
    uint16_t variable;
    struct {
        float x;
        float y;
        float z;
    } pos_1;
    uint8_t health;
    z64_actor_t* next;
};

typedef struct {
    z64_actor_t* incoming_item_actor;
} z64_link_t;

typedef struct {
    int32_t entrance_index;
    uint8_t master_quest;
    uint8_t spirit_medallion;
    uint8_t shadow_medallion;
    uint8_t items[24];
    struct {
        uint32_t chest;
        uint32_t swch;
        uint32_t clear;
        uint32_t collect;
        uint32_t unk_00_;
        uint32_t rooms_1;
        uint32_t rooms_2;
    } scene_flags[101];
    uint16_t event_chk_inf[14];
    uint16_t item_get_inf[4];
} tracker_file_t;

typedef struct {
    int16_t scene_index;
    int8_t room_index;
    int8_t scene_load_flag;
    uint32_t swch_flags;
    uint32_t temp_swch_flags;
    uint32_t chest_flags;
    uint32_t clear_flags;
    uint32_t temp_clear_flags;
    struct {
        uint32_t length;
        z64_actor_t* first;
    } actor_list[12];
} tracker_game_t;

static tracker_file_t z64_file;
static tracker_game_t z64_game;
static z64_link_t z64_link;

#include "../c/count_tracker.c"

extended_savecontext_static_t extended_savectx;
static uint8_t test_completed_encounters[COUNT_ENCOUNTER_BYTES];
static uint64_t test_completed_objectives;

bool count_encounter_is_complete(uint16_t encounter_id) {
    return encounter_id < COUNT_ENCOUNTER_CAPACITY &&
           (test_completed_encounters[encounter_id >> 3] & (1 << (encounter_id & 7)));
}

void count_encounter_complete(uint16_t encounter_id) {
    if (encounter_id < COUNT_ENCOUNTER_CAPACITY) {
        test_completed_encounters[encounter_id >> 3] |= 1 << (encounter_id & 7);
    }
}

bool count_objective_is_complete(uint8_t objective_id) {
    return objective_id < OBJECTIVE_COUNT && (test_completed_objectives & ((uint64_t)1 << objective_id));
}

void count_objective_complete(uint8_t objective_id) {
    if (objective_id < OBJECTIVE_COUNT) {
        test_completed_objectives |= (uint64_t)1 << objective_id;
    }
}

bool count_item_get_set(uint8_t flag) {
    return z64_file.item_get_inf[flag >> 4] & (1 << (flag & 0xF));
}

static uint8_t completed_objective_count(uint8_t first, uint8_t count) {
    uint8_t completed = 0;

    for (uint8_t i = 0; i < count; i++) {
        completed += count_objective_is_complete(first + i);
    }
    return completed;
}

static void reset_tracker(int16_t scene, int8_t room, uint8_t master_quest) {
    memset(&z64_game, 0, sizeof(z64_game));
    memset(&z64_link, 0, sizeof(z64_link));
    memset(&z64_file, 0, sizeof(z64_file));
    memset(&extended_savectx, 0, sizeof(extended_savectx));
    memset(test_completed_encounters, 0, sizeof(test_completed_encounters));
    test_completed_objectives = 0;
    memset(pending_enemy_encounters, 0, sizeof(pending_enemy_encounters));
    memset(pending_enemy_switches, 0, sizeof(pending_enemy_switches));
    z64_file.master_quest = master_quest;
    z64_game.scene_index = scene;
    z64_game.room_index = room;
    previous_scene = -1;
    previous_room = -1;
    previous_chest_flags = 0;
    previous_spirit_medallion = false;
    previous_shadow_medallion = false;
    count_tracker_update();
}

static z64_actor_t make_actor(int16_t actor_id, int8_t room, uint16_t variable, int16_t x, int16_t y, int16_t z) {
    z64_actor_t actor = {
        .actor_id = actor_id,
        .room_index = room,
        .variable = variable,
        .pos_1 = { x, y, z },
        .health = 1,
        .next = NULL,
    };
    return actor;
}

static void test_silver_room_transition_and_dedup() {
    reset_tracker(SCENE_SHADOW_TEMPLE, 9, 0);
    z64_game.swch_flags |= 1 << 9;
    count_tracker_update();

    assert(extended_savectx.silver_rupee_rooms_cleared == 1);
    assert(count_encounter_is_complete(4));

    count_tracker_update();
    assert(extended_savectx.silver_rupee_rooms_cleared == 1);

    reset_tracker(SCENE_SHADOW_TEMPLE, 9, 0);
    z64_file.scene_flags[SCENE_SHADOW_TEMPLE].swch |= 1 << 9;
    count_tracker_update();
    assert(extended_savectx.silver_rupee_rooms_cleared == 1);

    reset_tracker(SCENE_SHADOW_TEMPLE, 9, 0);
    z64_game.temp_swch_flags |= 1 << 9;
    count_tracker_update();
    assert(extended_savectx.silver_rupee_rooms_cleared == 1);
}

static void test_mq_silver_room_uses_mq_switch() {
    reset_tracker(SCENE_SHADOW_TEMPLE, 9, 1);
    z64_game.swch_flags |= 1 << 9;
    count_tracker_update();
    assert(extended_savectx.silver_rupee_rooms_cleared == 0);

    z64_game.swch_flags |= 1 << 17;
    count_tracker_update();
    assert(extended_savectx.silver_rupee_rooms_cleared == 1);
    assert(count_encounter_is_complete(3));
}

static void test_silver_room_sets_match_each_quest() {
    for (uint8_t i = 0; i < ENEMY_ENCOUNTER_BASE; i++) {
        count_encounter_complete(i);
    }

    z64_file.master_quest = 0;
    assert(count_tracker_get_silver_rooms_cleared() == 16);
    int room_count;
    assert(get_silver_rooms(&room_count) == vanilla_silver_rooms);
    assert(room_count == 16);
    uint32_t ids_seen = 0;
    for (uint8_t i = 0; i < room_count; i++) {
        uint8_t id = vanilla_silver_rooms[i].id;
        assert(id < ENEMY_ENCOUNTER_BASE);
        assert(!(ids_seen & (1u << id)));
        ids_seen |= 1u << id;
    }
    assert(ids_seen == 0xFFFF);

    z64_file.master_quest = 1;
    assert(count_tracker_get_silver_rooms_cleared() == 13);
    assert(get_silver_rooms(&room_count) == mq_silver_rooms);
    assert(room_count == 13);
    ids_seen = 0;
    for (uint8_t i = 0; i < room_count; i++) {
        uint8_t id = mq_silver_rooms[i].id;
        assert(id < ENEMY_ENCOUNTER_BASE);
        assert(!(ids_seen & (1u << id)));
        ids_seen |= 1u << id;
    }
    assert(ids_seen == 0x1FFF);
}

static void test_clear_room_enemies_commit_together() {
    z64_actor_t first = make_actor(ACTOR_EN_IK, 4, 0xFF02, -291, 520, 37);
    z64_actor_t second = make_actor(ACTOR_EN_IK, 4, 0xFF03, -290, 520, -32);
    first.next = &second;

    reset_tracker(SCENE_GANONS_TOWER, 4, 0);
    z64_game.actor_list[ACTORTYPE_ENEMY].first = &first;
    first.health = 0;
    second.health = 0;
    count_tracker_update();
    assert(extended_savectx.enemy_counts[ENEMY_IRON_KNUCKLE] == 0);
    assert(count_tracker_get_enemy_count(ENEMY_IRON_KNUCKLE) == 2);

    z64_game.clear_flags |= 1 << 4;
    count_tracker_update();
    assert(extended_savectx.enemy_counts[ENEMY_IRON_KNUCKLE] == 2);
    assert(count_tracker_get_enemy_count(ENEMY_IRON_KNUCKLE) == 2);

    count_tracker_update();
    assert(extended_savectx.enemy_counts[ENEMY_IRON_KNUCKLE] == 2);
}

static void test_ganons_tower_stalfos_wait_for_their_room_clear() {
    z64_actor_t first = make_actor(ACTOR_EN_TEST, 2, 0x0001, -183, 260, -12);
    z64_actor_t second = make_actor(ACTOR_EN_TEST, 2, 0x0000, 161, 260, -7);
    first.next = &second;

    reset_tracker(SCENE_GANONS_TOWER, 2, 0);
    z64_game.clear_flags |= 1 << 1;
    z64_game.actor_list[ACTORTYPE_ENEMY].first = &first;
    first.health = 0;
    second.health = 0;
    count_tracker_update();
    assert(extended_savectx.enemy_counts[ENEMY_STALFOS] == 0);
    assert(count_tracker_get_enemy_count(ENEMY_STALFOS) == 2);

    z64_game.temp_clear_flags |= 1 << 2;
    count_tracker_update();
    assert(extended_savectx.enemy_counts[ENEMY_STALFOS] == 2);
    assert(count_tracker_get_enemy_count(ENEMY_STALFOS) == 2);

    count_tracker_update();
    assert(extended_savectx.enemy_counts[ENEMY_STALFOS] == 2);
}

static void test_abandoned_clear_room_does_not_count() {
    z64_actor_t stalfos = make_actor(ACTOR_EN_TEST, 6, 0xFFFF, 273, 467, -3378);

    reset_tracker(SCENE_FOREST_TEMPLE, 6, 0);
    z64_game.actor_list[ACTORTYPE_ENEMY].first = &stalfos;
    stalfos.health = 0;
    count_tracker_update();
    assert(extended_savectx.enemy_counts[ENEMY_STALFOS] == 0);
    assert(count_tracker_get_enemy_count(ENEMY_STALFOS) == 1);

    z64_game.actor_list[ACTORTYPE_ENEMY].first = NULL;
    z64_game.room_index = 7;
    count_tracker_update();
    assert(count_tracker_get_enemy_count(ENEMY_STALFOS) == 0);
    z64_game.room_index = 6;
    count_tracker_update();
    z64_game.clear_flags |= 1 << 6;
    count_tracker_update();

    assert(extended_savectx.enemy_counts[ENEMY_STALFOS] == 0);
}

static void test_iron_knuckle_waits_for_room_clear() {
    z64_actor_t iron_knuckle = make_actor(ACTOR_EN_IK, 10, 1, -1250, 1003, 1476);

    reset_tracker(SCENE_SPIRIT_TEMPLE, 10, 0);
    z64_game.actor_list[ACTORTYPE_ENEMY].first = &iron_knuckle;
    iron_knuckle.health = 0;
    count_tracker_update();
    assert(extended_savectx.enemy_counts[ENEMY_IRON_KNUCKLE] == 0);
    assert(count_tracker_get_enemy_count(ENEMY_IRON_KNUCKLE) == 1);

    z64_game.swch_flags |= 1 << 25;
    count_tracker_update();
    assert(extended_savectx.enemy_counts[ENEMY_IRON_KNUCKLE] == 0);
    assert(count_tracker_get_enemy_count(ENEMY_IRON_KNUCKLE) == 1);

    z64_game.clear_flags |= 1 << 10;
    count_tracker_update();
    assert(extended_savectx.enemy_counts[ENEMY_IRON_KNUCKLE] == 1);
    assert(count_tracker_get_enemy_count(ENEMY_IRON_KNUCKLE) == 1);
}

static void test_nabooru_iron_knuckle_uses_clear_flags() {
    for (uint8_t master_quest = 0; master_quest <= 1; master_quest++) {
        reset_tracker(0x17, 1, master_quest);
        z64_file.scene_flags[SCENE_TWINROVA].clear |= NABOORU_KNUCKLE_CLEAR_FLAG;

        count_tracker_update();
        count_tracker_update();

        assert(extended_savectx.enemy_counts[ENEMY_IRON_KNUCKLE] == 1);

        reset_tracker(0x17, 1, master_quest);
        z64_game.clear_flags |= NABOORU_KNUCKLE_CLEAR_FLAG;

        count_tracker_update();
        count_tracker_update();

        assert(extended_savectx.enemy_counts[ENEMY_IRON_KNUCKLE] == 1);

        reset_tracker(0x17, 1, master_quest);
        z64_game.temp_clear_flags |= NABOORU_KNUCKLE_CLEAR_FLAG;

        count_tracker_update();
        count_tracker_update();

        assert(extended_savectx.enemy_counts[ENEMY_IRON_KNUCKLE] == 1);
    }
}

static void test_cutscene_map_iron_knuckles_are_counted() {
    z64_actor_t first = make_actor(ACTOR_EN_IK, 0, 0xFF03, -173, 0, 100);
    z64_actor_t second = make_actor(ACTOR_EN_IK, 0, 0xFF03, 173, 0, 100);
    z64_actor_t third = make_actor(ACTOR_EN_IK, 0, 0xFF02, 0, 0, -200);
    first.next = &second;
    second.next = &third;

    for (uint8_t master_quest = 0; master_quest <= 1; master_quest++) {
        reset_tracker(SCENE_CUTSCENE_MAP, 0, master_quest);
        z64_game.actor_list[ACTORTYPE_ENEMY].first = &first;
        first.health = 0;
        second.health = 0;
        third.health = 0;

        count_tracker_update();
        count_tracker_update();

        assert(extended_savectx.enemy_counts[ENEMY_IRON_KNUCKLE] == 3);
    }
}

static void test_enemy_encounters_are_grouped_with_monotonic_ids() {
    for (uint8_t master_quest = 0; master_quest <= 1; master_quest++) {
        int count;
        z64_file.master_quest = master_quest;
        enemy_encounter_t* encounters = get_enemy_encounters(&count);

        for (int i = 0; i < count; i++) {
            assert(encounters[i].id == ENEMY_ENCOUNTER_BASE + i);
            if (i > 0) {
                assert(encounters[i - 1].type <= encounters[i].type);
            }
        }
    }
}

static void test_child_and_adult_skull_kids_are_counted_once() {
    static const uint16_t skull_kid_types[] = { 1, 63 };

    for (int i = 0; i < 2; i++) {
        z64_actor_t skull_kid = make_actor(ACTOR_EN_SKJ, 1, skull_kid_types[i], 1183, -72, 476);

        reset_tracker(SCENE_LOST_WOODS, 1, 0);
        z64_game.actor_list[i == 0 ? ACTORTYPE_NPC : ACTORTYPE_ENEMY].first = &skull_kid;
        count_tracker_update();

        skull_kid.health = 0;
        count_tracker_update();
        count_tracker_update();

        assert(extended_savectx.enemy_counts[ENEMY_SKULL_KID] == 1);
        assert(count_tracker_get_enemy_count(ENEMY_SKULL_KID) == 1);
    }
}

static void test_chest_counts_once_and_excludes_minigame() {
    z64_actor_t trap = make_actor(ACTOR_EN_BOX, 0, (GI_ICE_TRAP << 5) | 2, 0, 0, 0);

    reset_tracker(SCENE_KOKIRI_FOREST, 0, 0);
    z64_game.actor_list[ACTORTYPE_CHEST].first = &trap;
    z64_link.incoming_item_actor = &trap;
    z64_game.chest_flags |= 1 << 2;
    count_tracker_update_chests();
    count_tracker_update_chests();
    assert(extended_savectx.trap_count == 1);

    reset_tracker(SCENE_KOKIRI_FOREST, 0, 0);
    z64_game.actor_list[ACTORTYPE_CHEST].first = &trap;
    z64_link.incoming_item_actor = &trap;
    z64_file.scene_flags[SCENE_KOKIRI_FOREST].chest |= 1 << 2;
    count_tracker_update_chests();
    assert(extended_savectx.trap_count == 1);

    z64_actor_t gold = make_actor(ACTOR_EN_BOX, 0, (GI_RUPEE_GOLD << 5) | 3, 0, 0, 0);
    reset_tracker(SCENE_GROTTOS, 0, 0);
    z64_game.actor_list[ACTORTYPE_CHEST].first = &gold;
    z64_link.incoming_item_actor = &gold;
    z64_game.chest_flags |= 1 << 3;
    count_tracker_update_chests();
    count_tracker_update_chests();
    assert(extended_savectx.gold_rupee_count == 1);

    reset_tracker(SCENE_TREASURE_BOX_SHOP, 0, 0);
    z64_game.actor_list[ACTORTYPE_CHEST].first = &gold;
    z64_link.incoming_item_actor = &gold;
    z64_game.chest_flags |= 1 << 3;
    count_tracker_update_chests();
    assert(extended_savectx.gold_rupee_count == 0);
}

static void test_variant_objective_encounter_sets() {
    static const uint8_t vanilla_dodongo[] = { 11, 12, 13, 14 };
    static const uint8_t mq_dodongo[] = { 17, 18, 19, 20 };
    static const uint8_t vanilla_flare[] = { 39, 40 };
    static const uint8_t mq_flare[] = { 81, 82 };

    for (uint8_t master_quest = 0; master_quest <= 1; master_quest++) {
        reset_tracker(SCENE_DODONGOS_CAVERN, 3, master_quest);
        const uint8_t* dodongo = master_quest ? mq_dodongo : vanilla_dodongo;
        const uint8_t* flare = master_quest ? mq_flare : vanilla_flare;

        for (uint8_t i = 0; i < 3; i++) {
            count_encounter_complete(ENEMY_ENCOUNTER_BASE + dodongo[i]);
        }
        track_objective_progress();
        assert(!count_objective_is_complete(OBJECTIVE_DODONGO_LIZALFOS));

        count_encounter_complete(ENEMY_ENCOUNTER_BASE + dodongo[3]);
        for (uint8_t i = 0; i < 2; i++) {
            count_encounter_complete(ENEMY_ENCOUNTER_BASE + flare[i]);
        }
        track_objective_progress();
        assert(count_objective_is_complete(OBJECTIVE_DODONGO_LIZALFOS));
        assert(count_objective_is_complete(OBJECTIVE_FIRE_FLARE_DANCERS));
    }
}

static void test_variant_chest_objectives() {
    z64_actor_t gold = make_actor(ACTOR_EN_BOX, 0, (GI_RUPEE_GOLD << 5) | 3, 0, 0, 0);

    reset_tracker(SCENE_FIRE_TEMPLE, 0, 0);
    z64_link.incoming_item_actor = &gold;
    z64_game.chest_flags |= 1 << 3;
    count_tracker_update_chests();
    assert(count_objective_is_complete(OBJECTIVE_FIRE_GOLD_RUPEE));

    reset_tracker(SCENE_FIRE_TEMPLE, 0, 1);
    z64_link.incoming_item_actor = &gold;
    z64_game.chest_flags |= 1 << 3;
    count_tracker_update_chests();
    assert(!count_objective_is_complete(OBJECTIVE_FIRE_GOLD_RUPEE));

    reset_tracker(SCENE_SPIRIT_TEMPLE, 0, 0);
    z64_game.chest_flags |= 1;
    count_tracker_update_chests();
    assert(count_objective_is_complete(OBJECTIVE_SPIRIT_BOMBCHU_CHEST));
}

static void test_boss_door_transitions() {
    reset_tracker(SCENE_FOREST_TEMPLE, 0, 0);
    z64_game.scene_index = SCENE_PHANTOM_GANON;
    count_tracker_update();
    assert(count_objective_is_complete(OBJECTIVE_FOREST_BOSS_DOOR));

    reset_tracker(SCENE_GANONS_TOWER, 0, 0);
    z64_game.scene_index = SCENE_GANONDORF;
    count_tracker_update();
    assert(count_objective_is_complete(OBJECTIVE_GANON_BOSS_DOOR));
}

static void test_sky_temple_end_chest() {
    reset_tracker(SCENE_KOKIRI_FOREST, 0, 0);
    z64_file.entrance_index = 0x00A0;
    z64_game.scene_index = SCENE_GROTTOS;
    count_tracker_update();
    assert(!count_objective_is_complete(OBJECTIVE_SKY_TEMPLE));

    z64_game.chest_flags |= 1;
    count_tracker_update_chests();
    assert(count_objective_is_complete(OBJECTIVE_SKY_TEMPLE));
}

static void test_unique_cow_milk_awards() {
    z64_actor_t cow = make_actor(ACTOR_EN_COW, 0, 0, 100, 20, -50);

    reset_tracker(SCENE_LINKS_HOUSE, 0, 0);
    z64_file.items[BOTTLE_SLOT_FIRST] = 0x14;
    snapshot_bottle_items();
    z64_link.incoming_item_actor = &cow;
    z64_file.items[BOTTLE_SLOT_FIRST] = ITEM_MILK;
    count_tracker_update();
    assert(count_objective_is_complete(OBJECTIVE_COW_0));

    z64_file.items[BOTTLE_SLOT_FIRST] = 0x14;
    count_tracker_update();
    z64_file.items[BOTTLE_SLOT_FIRST] = ITEM_MILK;
    count_tracker_update();
    assert(completed_objective_count(OBJECTIVE_COW_0, 16) == 1);

    reset_tracker(SCENE_GROTTOS, 2, 0);
    z64_file.items[BOTTLE_SLOT_FIRST] = 0x14;
    snapshot_bottle_items();
    z64_link.incoming_item_actor = &cow;
    z64_file.items[BOTTLE_SLOT_FIRST] = ITEM_MILK;
    count_tracker_update();
    assert(completed_objective_count(OBJECTIVE_COW_0, 16) == 1);

    reset_tracker(SCENE_GROTTOS, 2, 0);
    z64_file.items[BOTTLE_SLOT_FIRST] = 0x14;
    snapshot_bottle_items();
    z64_file.items[BOTTLE_SLOT_FIRST] = ITEM_MILK;
    count_tracker_update();
    assert(completed_objective_count(OBJECTIVE_COW_0, 16) == 0);
}

static void test_boss_room_one_clears_are_persisted() {
    static const uint8_t dungeons[] = {
        SCENE_DEKU_TREE,   SCENE_DODONGOS_CAVERN, SCENE_JABU_JABU,    SCENE_FOREST_TEMPLE,
        SCENE_FIRE_TEMPLE, SCENE_WATER_TEMPLE,    SCENE_SPIRIT_TEMPLE, SCENE_SHADOW_TEMPLE,
    };
    static const uint8_t boss_scenes[] = {
        SCENE_GOHMA,    SCENE_KING_DODONGO, SCENE_BARINADE, SCENE_PHANTOM_GANON,
        SCENE_VOLVAGIA, SCENE_MORPHA,       SCENE_TWINROVA, SCENE_BONGO_BONGO,
    };

    for (uint8_t i = 0; i < sizeof(dungeons); i++) {
        reset_tracker(boss_scenes[i], 0, 0);
        assert(!count_tracker_is_boss_cleared(dungeons[i]));

        z64_game.clear_flags = 1 << 1;
        count_tracker_update();

        z64_game.scene_index = dungeons[i];
        z64_game.clear_flags = 0;
        assert(count_tracker_is_boss_cleared(dungeons[i]));
    }
}

static void test_ganondorf_transition_persists_boss_clear() {
    reset_tracker(SCENE_GANONDORF, 0, 0);
    assert(!count_tracker_is_boss_cleared(SCENE_INSIDE_GANONS_CASTLE));

    z64_game.scene_load_flag = 0x14;
    count_tracker_update();

    z64_game.scene_index = SCENE_GANONS_CASTLE_COLLAPSE;
    z64_game.scene_load_flag = 0;
    assert(count_tracker_is_boss_cleared(SCENE_INSIDE_GANONS_CASTLE));
}

static void test_blue_warp_flags_imply_completion_and_boss_clear() {
    reset_tracker(SCENE_DODONGOS_CAVERN, 0, 0);
    z64_file.event_chk_inf[0x25 >> 4] |= 1 << (0x25 & 0xF);

    assert(count_tracker_is_dungeon_completed(SCENE_DODONGOS_CAVERN));
    assert(count_tracker_is_boss_cleared(SCENE_DODONGOS_CAVERN));
}

static void test_existing_spirit_reward_does_not_imply_completion() {
    reset_tracker(SCENE_SPIRIT_TEMPLE, 0, 0);
    z64_game.scene_index = SCENE_TWINROVA;
    z64_file.spirit_medallion = 1;
    count_tracker_update();
    count_tracker_update();

    assert(!count_tracker_is_dungeon_completed(SCENE_SPIRIT_TEMPLE));
}

static void test_spirit_reward_transition_marks_completion() {
    reset_tracker(SCENE_TWINROVA, 0, 0);
    count_tracker_update();
    z64_file.spirit_medallion = 1;
    count_tracker_update();

    assert(count_tracker_is_dungeon_completed(SCENE_SPIRIT_TEMPLE));
    assert(count_tracker_is_boss_cleared(SCENE_SPIRIT_TEMPLE));
}

int main() {
    test_silver_room_transition_and_dedup();
    test_mq_silver_room_uses_mq_switch();
    test_silver_room_sets_match_each_quest();
    test_clear_room_enemies_commit_together();
    test_ganons_tower_stalfos_wait_for_their_room_clear();
    test_abandoned_clear_room_does_not_count();
    test_iron_knuckle_waits_for_room_clear();
    test_nabooru_iron_knuckle_uses_clear_flags();
    test_cutscene_map_iron_knuckles_are_counted();
    test_enemy_encounters_are_grouped_with_monotonic_ids();
    test_child_and_adult_skull_kids_are_counted_once();
    test_chest_counts_once_and_excludes_minigame();
    test_variant_objective_encounter_sets();
    test_variant_chest_objectives();
    test_boss_door_transitions();
    test_sky_temple_end_chest();
    test_unique_cow_milk_awards();
    test_boss_room_one_clears_are_persisted();
    test_ganondorf_transition_persists_boss_clear();
    test_blue_warp_flags_imply_completion_and_boss_clear();
    test_existing_spirit_reward_does_not_imply_completion();
    test_spirit_reward_transition_marks_completion();
    return 0;
}
