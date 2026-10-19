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
    uint8_t master_quest;
    struct {
        uint32_t chest;
        uint32_t swch;
        uint32_t clear;
        uint32_t collect;
        uint32_t unk_00_;
        uint32_t rooms_1;
        uint32_t rooms_2;
    } scene_flags[101];
} tracker_file_t;

typedef struct {
    int16_t scene_index;
    int8_t room_index;
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

bool count_encounter_is_complete(uint16_t encounter_id) {
    return encounter_id < COUNT_ENCOUNTER_CAPACITY &&
        (test_completed_encounters[encounter_id >> 3] &
         (1 << (encounter_id & 7)));
}

void count_encounter_complete(uint16_t encounter_id) {
    if (encounter_id < COUNT_ENCOUNTER_CAPACITY) {
        test_completed_encounters[encounter_id >> 3] |=
            1 << (encounter_id & 7);
    }
}

static void reset_tracker(
    int16_t scene,
    int8_t room,
    uint8_t master_quest
) {
    memset(&z64_game, 0, sizeof(z64_game));
    memset(&z64_link, 0, sizeof(z64_link));
    memset(&z64_file, 0, sizeof(z64_file));
    memset(&extended_savectx, 0, sizeof(extended_savectx));
    memset(test_completed_encounters, 0, sizeof(test_completed_encounters));
    memset(pending_enemy_encounters, 0, sizeof(pending_enemy_encounters));
    memset(pending_enemy_switches, 0, sizeof(pending_enemy_switches));
    z64_file.master_quest = master_quest;
    z64_game.scene_index = scene;
    z64_game.room_index = room;
    previous_scene = -1;
    previous_room = -1;
    previous_chest_flags = 0;
    count_tracker_update();
}

static z64_actor_t make_actor(
    int16_t actor_id,
    int8_t room,
    uint16_t variable,
    int16_t x,
    int16_t y,
    int16_t z
) {
    z64_actor_t actor = {
        .actor_id = actor_id,
        .room_index = room,
        .variable = variable,
        .pos_1 = {x, y, z},
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
    assert(count_encounter_is_complete(6));

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
    assert(count_encounter_is_complete(6));
}

static void test_clear_room_enemies_commit_together() {
    z64_actor_t first = make_actor(
        ACTOR_EN_IK, 4, 0xFF02, -291, 520, 37
    );
    z64_actor_t second = make_actor(
        ACTOR_EN_IK, 4, 0xFF03, -290, 520, -32
    );
    first.next = &second;

    reset_tracker(SCENE_GANONS_TOWER, 4, 0);
    z64_game.actor_list[ACTORTYPE_ENEMY].first = &first;
    first.health = 0;
    second.health = 0;
    count_tracker_update();
    assert(extended_savectx.enemy_counts[ENEMY_IRON_KNUCKLE] == 0);

    z64_game.clear_flags |= 1 << 4;
    count_tracker_update();
    assert(extended_savectx.enemy_counts[ENEMY_IRON_KNUCKLE] == 2);

    count_tracker_update();
    assert(extended_savectx.enemy_counts[ENEMY_IRON_KNUCKLE] == 2);
}

static void test_ganons_tower_stalfos_wait_for_their_room_clear() {
    z64_actor_t first = make_actor(
        ACTOR_EN_TEST, 2, 0x0001, -183, 260, -12
    );
    z64_actor_t second = make_actor(
        ACTOR_EN_TEST, 2, 0x0000, 161, 260, -7
    );
    first.next = &second;

    reset_tracker(SCENE_GANONS_TOWER, 2, 0);
    z64_game.clear_flags |= 1 << 1;
    z64_game.actor_list[ACTORTYPE_ENEMY].first = &first;
    first.health = 0;
    second.health = 0;
    count_tracker_update();
    assert(extended_savectx.enemy_counts[ENEMY_STALFOS] == 0);

    z64_game.temp_clear_flags |= 1 << 2;
    count_tracker_update();
    assert(extended_savectx.enemy_counts[ENEMY_STALFOS] == 2);

    count_tracker_update();
    assert(extended_savectx.enemy_counts[ENEMY_STALFOS] == 2);
}

static void test_abandoned_clear_room_does_not_count() {
    z64_actor_t stalfos = make_actor(
        ACTOR_EN_TEST, 6, 0xFFFF, 273, 467, -3378
    );

    reset_tracker(SCENE_FOREST_TEMPLE, 6, 0);
    z64_game.actor_list[ACTORTYPE_ENEMY].first = &stalfos;
    stalfos.health = 0;
    count_tracker_update();

    z64_game.actor_list[ACTORTYPE_ENEMY].first = NULL;
    z64_game.room_index = 7;
    count_tracker_update();
    z64_game.room_index = 6;
    count_tracker_update();
    z64_game.clear_flags |= 1 << 6;
    count_tracker_update();

    assert(extended_savectx.enemy_counts[ENEMY_STALFOS] == 0);
}

static void test_iron_knuckle_waits_for_room_clear() {
    z64_actor_t iron_knuckle = make_actor(
        ACTOR_EN_IK, 10, 1, -1250, 1003, 1476
    );

    reset_tracker(SCENE_SPIRIT_TEMPLE, 10, 0);
    z64_game.actor_list[ACTORTYPE_ENEMY].first = &iron_knuckle;
    iron_knuckle.health = 0;
    count_tracker_update();
    assert(extended_savectx.enemy_counts[ENEMY_IRON_KNUCKLE] == 0);

    z64_game.swch_flags |= 1 << 25;
    count_tracker_update();
    assert(extended_savectx.enemy_counts[ENEMY_IRON_KNUCKLE] == 0);

    z64_game.clear_flags |= 1 << 10;
    count_tracker_update();
    assert(extended_savectx.enemy_counts[ENEMY_IRON_KNUCKLE] == 1);
}

static void test_nabooru_iron_knuckle_uses_clear_flags() {
    for (uint8_t master_quest = 0; master_quest <= 1; master_quest++) {
        reset_tracker(0x17, 1, master_quest);
        z64_file.scene_flags[SCENE_TWINROVA].clear |=
            NABOORU_KNUCKLE_CLEAR_FLAG;

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

static void test_chest_counts_once_and_excludes_minigame() {
    z64_actor_t trap = make_actor(
        ACTOR_EN_BOX, 0, (GI_ICE_TRAP << 5) | 2, 0, 0, 0
    );

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

    z64_actor_t gold = make_actor(
        ACTOR_EN_BOX, 0, (GI_RUPEE_GOLD << 5) | 3, 0, 0, 0
    );
    reset_tracker(SCENE_TREASURE_BOX_SHOP, 0, 0);
    z64_game.actor_list[ACTORTYPE_CHEST].first = &gold;
    z64_link.incoming_item_actor = &gold;
    z64_game.chest_flags |= 1 << 3;
    count_tracker_update_chests();
    assert(extended_savectx.gold_rupee_count == 0);
}

int main() {
    test_silver_room_transition_and_dedup();
    test_mq_silver_room_uses_mq_switch();
    test_clear_room_enemies_commit_together();
    test_ganons_tower_stalfos_wait_for_their_room_clear();
    test_abandoned_clear_room_does_not_count();
    test_iron_knuckle_waits_for_room_clear();
    test_nabooru_iron_knuckle_uses_clear_flags();
    test_chest_counts_once_and_excludes_minigame();
    return 0;
}
