#include "count_tracker.h"

#include <stdint.h>

#include "counts.h"
#include "save.h"
#include "z64.h"

#define ACTOR_EN_BOX 0x000A
#define ACTOR_EN_TEST 0x0002
#define ACTOR_EN_ZF 0x0025
#define ACTOR_EN_RD 0x0090
#define ACTOR_EN_FD 0x0099
#define ACTOR_EN_DH 0x00A4
#define ACTOR_EN_COW 0x00D5
#define ACTOR_EN_BA 0x00DE
#define ACTOR_EN_IK 0x0113
#define ACTOR_EN_SKJ 0x0115
#define ACTOR_EN_WF 0x01AF
#define GI_RUPEE_GOLD 0x56
#define GI_ICE_TRAP 0x7C
#define BOTTLE_SLOT_FIRST 18
#define BOTTLE_SLOT_COUNT 4
#define ITEM_MILK 0x1A
#define ENBOX_GET_ITEM_ID(actor) (((actor)->variable >> 5) & 0x7F)
#define ENBOX_GET_TREASURE_FLAG(actor) ((actor)->variable & 0x1F)
#define ACTOR_CATEGORY_COUNT 12
#define ROOM_CLEAR_FLAG_COUNT 32
#define BOSS_ENCOUNTER_BASE 16
#define DUNGEON_COMPLETION_BASE 24
#define ENEMY_ENCOUNTER_BASE 32
#define GANONDORF_ENCOUNTER_ID (COUNT_ENCOUNTER_CAPACITY - 1)
#define NABOORU_KNUCKLE_CLEAR_FLAG (1 << 1)

typedef enum {
    ENEMY_IRON_KNUCKLE,
    ENEMY_WHITE_WOLFOS,
    ENEMY_LIZALFOS,
    ENEMY_GIBDO,
    ENEMY_TENTACLE,
    ENEMY_STALFOS,
    ENEMY_DEAD_HAND,
    ENEMY_FLARE_DANCER,
    ENEMY_SKULL_KID,
    ENEMY_TYPE_COUNT,
} enemy_type_t;

typedef struct {
    uint8_t id;
    uint8_t type;
    uint8_t scene;
    int8_t room;
    int16_t x;
    int16_t y;
    int16_t z;
} enemy_encounter_t;

typedef struct {
    uint8_t dungeon;
    uint8_t boss_scene;
    uint32_t clear_flag;
} boss_entry_t;

static const boss_entry_t bosses[] = {
    { SCENE_DEKU_TREE, SCENE_GOHMA, 1 << 1 },
    { SCENE_DODONGOS_CAVERN, SCENE_KING_DODONGO, 1 << 1 },
    { SCENE_JABU_JABU, SCENE_BARINADE, 1 << 1 },
    { SCENE_FOREST_TEMPLE, SCENE_PHANTOM_GANON, 1 << 1 },
    { SCENE_FIRE_TEMPLE, SCENE_VOLVAGIA, 1 << 1 },
    { SCENE_WATER_TEMPLE, SCENE_MORPHA, 1 << 1 },
    { SCENE_SHADOW_TEMPLE, SCENE_BONGO_BONGO, 1 << 1 },
    { SCENE_SPIRIT_TEMPLE, SCENE_TWINROVA, 1 << 1 },
};

#define ENCOUNTER(id, type, scene, room, x, y, z) \
    { ENEMY_ENCOUNTER_BASE + id, ENEMY_##type, SCENE_##scene, room, x, y, z }

static enemy_encounter_t vanilla_enemy_encounters[] = {
    ENCOUNTER(0, IRON_KNUCKLE, SPIRIT_TEMPLE, 10, -1250, 1003, 1476),
    ENCOUNTER(1, IRON_KNUCKLE, SPIRIT_TEMPLE, 20, 1515, 1003, 1473),
    ENCOUNTER(2, IRON_KNUCKLE, GANONS_TOWER, 4, -291, 520, 37),
    ENCOUNTER(3, IRON_KNUCKLE, GANONS_TOWER, 4, -290, 520, -32),
    ENCOUNTER(4, IRON_KNUCKLE, TWINROVA, 1, -1235, 0, 1268),
    ENCOUNTER(5, IRON_KNUCKLE, CUTSCENE_MAP, 0, -173, 0, 100),
    ENCOUNTER(6, IRON_KNUCKLE, CUTSCENE_MAP, 0, 173, 0, 100),
    ENCOUNTER(7, IRON_KNUCKLE, CUTSCENE_MAP, 0, 0, 0, -200),
    ENCOUNTER(8, WHITE_WOLFOS, ICE_CAVERN, 7, -1096, 282, 964),
    ENCOUNTER(9, WHITE_WOLFOS, GERUDO_TRAINING_GROUND, 3, -1377, 159, -2795),
    ENCOUNTER(10, WHITE_WOLFOS, GERUDO_TRAINING_GROUND, 3, -1675, 159, -2849),
    ENCOUNTER(11, LIZALFOS, DODONGOS_CAVERN, 3, 3969, 100, -2161),
    ENCOUNTER(12, LIZALFOS, DODONGOS_CAVERN, 3, 2829, 100, -1961),
    ENCOUNTER(13, LIZALFOS, DODONGOS_CAVERN, 3, 4261, 531, -1055),
    ENCOUNTER(14, LIZALFOS, DODONGOS_CAVERN, 3, 4442, 531, -1386),
    ENCOUNTER(15, LIZALFOS, SPIRIT_TEMPLE, 4, -1074, 501, -717),
    ENCOUNTER(16, LIZALFOS, SPIRIT_TEMPLE, 4, -730, 480, -736),
    ENCOUNTER(17, LIZALFOS, SPIRIT_TEMPLE, 26, -545, 1743, -830),
    ENCOUNTER(18, LIZALFOS, SPIRIT_TEMPLE, 26, -24, 1743, -304),
    ENCOUNTER(19, GIBDO, SHADOW_TEMPLE, 7, 3498, -543, 721),
    ENCOUNTER(20, GIBDO, SHADOW_TEMPLE, 7, 3212, -543, 726),
    ENCOUNTER(21, GIBDO, SHADOW_TEMPLE, 20, 4253, -1363, -1070),
    ENCOUNTER(22, GIBDO, SHADOW_TEMPLE, 20, 4527, -1363, -1081),
    ENCOUNTER(23, GIBDO, BOTTOM_OF_THE_WELL, 2, -2070, 0, -854),
    ENCOUNTER(24, TENTACLE, JABU_JABU, 8, 1, 20, -5801),
    ENCOUNTER(25, TENTACLE, JABU_JABU, 10, -1561, 20, -4681),
    ENCOUNTER(26, TENTACLE, JABU_JABU, 11, 660, 20, -5680),
    ENCOUNTER(27, STALFOS, FOREST_TEMPLE, 6, 273, 467, -3378),
    ENCOUNTER(28, STALFOS, FOREST_TEMPLE, 6, -38, 467, -3382),
    ENCOUNTER(29, STALFOS, SPIRIT_TEMPLE, 3, -1565, 30, -670),
    ENCOUNTER(30, STALFOS, SHADOW_TEMPLE, 9, 3752, -1143, 3413),
    ENCOUNTER(31, STALFOS, SHADOW_TEMPLE, 21, 2746, -991, -2011),
    ENCOUNTER(32, STALFOS, SHADOW_TEMPLE, 21, 1209, -974, -1982),
    ENCOUNTER(33, STALFOS, GANONS_TOWER, 2, -183, 260, -12),
    ENCOUNTER(34, STALFOS, GANONS_TOWER, 2, 161, 260, -7),
    ENCOUNTER(35, STALFOS, GERUDO_TRAINING_GROUND, 1, -1629, -90, -19),
    ENCOUNTER(36, STALFOS, GERUDO_TRAINING_GROUND, 1, -1870, -90, -357),
    ENCOUNTER(37, DEAD_HAND, SHADOW_TEMPLE, 4, -2424, -63, -413),
    ENCOUNTER(38, DEAD_HAND, BOTTOM_OF_THE_WELL, 4, 1281, -20, 211),
    ENCOUNTER(39, FLARE_DANCER, FIRE_TEMPLE, 3, -361, 80, -1300),
    ENCOUNTER(40, FLARE_DANCER, FIRE_TEMPLE, 24, -2928, 2920, 139),
    ENCOUNTER(41, SKULL_KID, LOST_WOODS, 1, 1183, -72, 476),
    ENCOUNTER(42, SKULL_KID, LOST_WOODS, 1, 1265, -94, 420),
    ENCOUNTER(43, SKULL_KID, LOST_WOODS, 9, -1132, 80, -128),
};

static enemy_encounter_t mq_enemy_encounters[] = {
    ENCOUNTER(0, IRON_KNUCKLE, FIRE_TEMPLE, 18, 338, 20, -1452),
    ENCOUNTER(1, IRON_KNUCKLE, SPIRIT_TEMPLE, 10, -1250, 1003, 1476),
    ENCOUNTER(2, IRON_KNUCKLE, SPIRIT_TEMPLE, 20, 1515, 1003, 1473),
    ENCOUNTER(3, IRON_KNUCKLE, SPIRIT_TEMPLE, 22, 681, 1633, -1317),
    ENCOUNTER(4, IRON_KNUCKLE, GANONS_TOWER, 4, -291, 520, 37),
    ENCOUNTER(5, IRON_KNUCKLE, GANONS_TOWER, 4, -290, 520, -32),
    ENCOUNTER(6, IRON_KNUCKLE, GERUDO_TRAINING_GROUND, 1, -1629, -90, -19),
    ENCOUNTER(7, IRON_KNUCKLE, GERUDO_TRAINING_GROUND, 5, 1378, -65, -2758),
    ENCOUNTER(8, IRON_KNUCKLE, INSIDE_GANONS_CASTLE, 0, 3, 150, 690),
    ENCOUNTER(9, IRON_KNUCKLE, INSIDE_GANONS_CASTLE, 17, -945, 411, 822),
    ENCOUNTER(10, IRON_KNUCKLE, TWINROVA, 1, -1235, 0, 1268),
    ENCOUNTER(11, IRON_KNUCKLE, CUTSCENE_MAP, 0, -173, 0, 100),
    ENCOUNTER(12, IRON_KNUCKLE, CUTSCENE_MAP, 0, 173, 0, 100),
    ENCOUNTER(13, IRON_KNUCKLE, CUTSCENE_MAP, 0, 0, 0, -200),
    ENCOUNTER(14, WHITE_WOLFOS, ICE_CAVERN, 3, 271, 0, -401),
    ENCOUNTER(15, WHITE_WOLFOS, ICE_CAVERN, 5, -1564, 41, -957),
    ENCOUNTER(16, WHITE_WOLFOS, ICE_CAVERN, 5, -1156, 61, -683),
    ENCOUNTER(17, LIZALFOS, DODONGOS_CAVERN, 3, 3969, 100, -2161),
    ENCOUNTER(18, LIZALFOS, DODONGOS_CAVERN, 3, 2829, 100, -1961),
    ENCOUNTER(19, LIZALFOS, DODONGOS_CAVERN, 3, 4261, 531, -1055),
    ENCOUNTER(20, LIZALFOS, DODONGOS_CAVERN, 3, 4442, 531, -1386),
    ENCOUNTER(21, LIZALFOS, DODONGOS_CAVERN, 8, 1558, 16, -3540),
    ENCOUNTER(22, LIZALFOS, JABU_JABU, 4, -1229, 632, -1446),
    ENCOUNTER(23, LIZALFOS, JABU_JABU, 7, 0, -340, -4846),
    ENCOUNTER(24, LIZALFOS, JABU_JABU, 14, 928, -1153, -3216),
    ENCOUNTER(25, LIZALFOS, FIRE_TEMPLE, 4, 2351, 2520, -26),
    ENCOUNTER(26, LIZALFOS, FIRE_TEMPLE, 5, 1777, 2800, 561),
    ENCOUNTER(27, LIZALFOS, FIRE_TEMPLE, 5, 1900, 2800, -835),
    ENCOUNTER(28, LIZALFOS, FIRE_TEMPLE, 5, 2181, 2800, -71),
    ENCOUNTER(29, LIZALFOS, FIRE_TEMPLE, 5, 2263, 2800, 519),
    ENCOUNTER(30, LIZALFOS, FIRE_TEMPLE, 25, -824, 2800, -1142),
    ENCOUNTER(31, LIZALFOS, WATER_TEMPLE, 10, -1024, 1084, 121),
    ENCOUNTER(32, LIZALFOS, WATER_TEMPLE, 18, 1720, 335, 283),
    ENCOUNTER(33, LIZALFOS, WATER_TEMPLE, 18, 1717, 335, 84),
    ENCOUNTER(34, LIZALFOS, WATER_TEMPLE, 20, 653, 706, 953),
    ENCOUNTER(35, LIZALFOS, WATER_TEMPLE, 20, 658, 992, 184),
    ENCOUNTER(36, LIZALFOS, GERUDO_TRAINING_GROUND, 7, 1228, -80, -198),
    ENCOUNTER(37, GIBDO, SPIRIT_TEMPLE, 2, -502, 30, -734),
    ENCOUNTER(38, GIBDO, SPIRIT_TEMPLE, 2, -420, 30, -1333),
    ENCOUNTER(39, GIBDO, SPIRIT_TEMPLE, 2, -417, 30, -1141),
    ENCOUNTER(40, GIBDO, SPIRIT_TEMPLE, 21, 1889, 873, -636),
    ENCOUNTER(41, GIBDO, SHADOW_TEMPLE, 7, 3212, -543, 726),
    ENCOUNTER(42, GIBDO, SHADOW_TEMPLE, 7, 3498, -543, 721),
    ENCOUNTER(43, GIBDO, SHADOW_TEMPLE, 20, 4253, -1363, -1070),
    ENCOUNTER(44, GIBDO, SHADOW_TEMPLE, 20, 4527, -1363, -1081),
    ENCOUNTER(45, GIBDO, BOTTOM_OF_THE_WELL, 2, -1753, 0, -738),
    ENCOUNTER(46, GIBDO, INSIDE_GANONS_CASTLE, 18, -1429, 150, 1045),
    ENCOUNTER(47, TENTACLE, JABU_JABU, 8, 1, 20, -5801),
    ENCOUNTER(48, TENTACLE, JABU_JABU, 9, 1561, 20, -4679),
    ENCOUNTER(49, TENTACLE, JABU_JABU, 10, -1562, 20, -4683),
    ENCOUNTER(50, STALFOS, FOREST_TEMPLE, 5, -1000, 383, -1440),
    ENCOUNTER(51, STALFOS, FIRE_TEMPLE, 14, -2021, 4180, -1019),
    ENCOUNTER(52, STALFOS, FIRE_TEMPLE, 14, -1625, 4120, -627),
    ENCOUNTER(53, STALFOS, FIRE_TEMPLE, 15, 102, 0, -598),
    ENCOUNTER(54, STALFOS, FIRE_TEMPLE, 15, 283, 0, -320),
    ENCOUNTER(55, STALFOS, WATER_TEMPLE, 3, -2023, 300, 765),
    ENCOUNTER(56, STALFOS, WATER_TEMPLE, 3, -2023, 300, 966),
    ENCOUNTER(57, STALFOS, WATER_TEMPLE, 6, -3114, 560, -382),
    ENCOUNTER(58, STALFOS, WATER_TEMPLE, 6, -3211, 560, -455),
    ENCOUNTER(59, STALFOS, WATER_TEMPLE, 6, -3213, 560, -355),
    ENCOUNTER(60, STALFOS, WATER_TEMPLE, 16, -1285, 108, -2986),
    ENCOUNTER(61, STALFOS, WATER_TEMPLE, 19, 1554, 780, 268),
    ENCOUNTER(62, STALFOS, WATER_TEMPLE, 19, 1557, 780, 107),
    ENCOUNTER(63, STALFOS, WATER_TEMPLE, 19, 1486, 780, 192),
    ENCOUNTER(64, STALFOS, SPIRIT_TEMPLE, 15, 1161, 480, -1015),
    ENCOUNTER(65, STALFOS, SPIRIT_TEMPLE, 15, 1163, 480, -678),
    ENCOUNTER(66, STALFOS, SPIRIT_TEMPLE, 27, -1053, 150, -1799),
    ENCOUNTER(67, STALFOS, SHADOW_TEMPLE, 14, 1114, -1083, 1081),
    ENCOUNTER(68, STALFOS, SHADOW_TEMPLE, 14, 1489, -1323, 1083),
    ENCOUNTER(69, STALFOS, SHADOW_TEMPLE, 21, 1209, -974, -1982),
    ENCOUNTER(70, STALFOS, SHADOW_TEMPLE, 21, 2746, -991, -2011),
    ENCOUNTER(71, STALFOS, ICE_CAVERN, 7, -1096, 282, 964),
    ENCOUNTER(72, STALFOS, GANONS_TOWER, 2, -183, 260, -12),
    ENCOUNTER(73, STALFOS, GANONS_TOWER, 2, 161, 260, -7),
    ENCOUNTER(74, STALFOS, GERUDO_TRAINING_GROUND, 3, -1396, 159, -2611),
    ENCOUNTER(75, STALFOS, GERUDO_TRAINING_GROUND, 3, -1505, 159, -2788),
    ENCOUNTER(76, STALFOS, INSIDE_GANONS_CASTLE, 5, 1018, 110, 631),
    ENCOUNTER(77, STALFOS, INSIDE_GANONS_CASTLE, 5, 757, 110, 792),
    ENCOUNTER(78, DEAD_HAND, SHADOW_TEMPLE, 4, -2434, -63, -405),
    ENCOUNTER(79, DEAD_HAND, SHADOW_TEMPLE, 17, -4306, -1363, -542),
    ENCOUNTER(80, DEAD_HAND, BOTTOM_OF_THE_WELL, 4, 1280, -16, 210),
    ENCOUNTER(81, FLARE_DANCER, FIRE_TEMPLE, 3, -361, 80, -1300),
    ENCOUNTER(82, FLARE_DANCER, FIRE_TEMPLE, 24, -2928, 2920, 139),
    ENCOUNTER(83, SKULL_KID, LOST_WOODS, 1, 1183, -72, 476),
    ENCOUNTER(84, SKULL_KID, LOST_WOODS, 1, 1265, -94, 420),
    ENCOUNTER(85, SKULL_KID, LOST_WOODS, 9, -1132, 80, -128),
};

typedef char
    assert_enemy_encounters_fit    [ENEMY_ENCOUNTER_BASE + sizeof(mq_enemy_encounters) / sizeof(*mq_enemy_encounters) <= GANONDORF_ENCOUNTER_ID
                                    ? 1
                                    : -1];
typedef char assert_boss_encounters_fit
    [BOSS_ENCOUNTER_BASE + sizeof(bosses) / sizeof(*bosses) <= DUNGEON_COMPLETION_BASE ? 1 : -1];
typedef char assert_dungeon_completions_fit
    [DUNGEON_COMPLETION_BASE + sizeof(bosses) / sizeof(*bosses) <= ENEMY_ENCOUNTER_BASE ? 1 : -1];

typedef struct {
    uint8_t id;
    uint8_t scene;
    uint8_t switch_flag;
} silver_room_t;

#define SILVER_ROOM(id, scene, flag) { id, SCENE_##scene, flag }

static const silver_room_t vanilla_silver_rooms[] = {
    SILVER_ROOM(0, ICE_CAVERN, 8),
    SILVER_ROOM(1, ICE_CAVERN, 9),
    SILVER_ROOM(2, BOTTOM_OF_THE_WELL, 31),
    SILVER_ROOM(3, SHADOW_TEMPLE, 1),
    SILVER_ROOM(4, SHADOW_TEMPLE, 9),
    SILVER_ROOM(5, SHADOW_TEMPLE, 8),
    SILVER_ROOM(6, GERUDO_TRAINING_GROUND, 28),
    SILVER_ROOM(7, GERUDO_TRAINING_GROUND, 12),
    SILVER_ROOM(8, GERUDO_TRAINING_GROUND, 27),
    SILVER_ROOM(9, SPIRIT_TEMPLE, 5),
    SILVER_ROOM(10, SPIRIT_TEMPLE, 2),
    SILVER_ROOM(11, SPIRIT_TEMPLE, 10),
    SILVER_ROOM(12, INSIDE_GANONS_CASTLE, 11),
    SILVER_ROOM(13, INSIDE_GANONS_CASTLE, 18),
    SILVER_ROOM(14, INSIDE_GANONS_CASTLE, 9),
    SILVER_ROOM(15, INSIDE_GANONS_CASTLE, 14),
};

// clang-format off
static const silver_room_t mq_silver_rooms[] = {
    SILVER_ROOM(0, DODONGOS_CAVERN, 31),
    SILVER_ROOM(1, SHADOW_TEMPLE, 1),
    SILVER_ROOM(2, SHADOW_TEMPLE, 3),
    SILVER_ROOM(3, SHADOW_TEMPLE, 17),
    SILVER_ROOM(4, SHADOW_TEMPLE, 8),
    SILVER_ROOM(5, GERUDO_TRAINING_GROUND, 28),
    SILVER_ROOM(6, GERUDO_TRAINING_GROUND, 12),
    SILVER_ROOM(7, GERUDO_TRAINING_GROUND, 27),
    SILVER_ROOM(8, SPIRIT_TEMPLE, 31),
    SILVER_ROOM(9, SPIRIT_TEMPLE, 0),
    SILVER_ROOM(10, INSIDE_GANONS_CASTLE, 1),
    SILVER_ROOM(11, INSIDE_GANONS_CASTLE, 11),
    SILVER_ROOM(12, INSIDE_GANONS_CASTLE, 2),
};
// clang-format on

#undef SILVER_ROOM

typedef char
    assert_silver_room_encounters_fit[sizeof(vanilla_silver_rooms) / sizeof(*vanilla_silver_rooms) <=
                                                  ENEMY_ENCOUNTER_BASE &&
                                              sizeof(mq_silver_rooms) / sizeof(*mq_silver_rooms) <= ENEMY_ENCOUNTER_BASE
                                          ? 1
                                          : -1];

static const silver_room_t* get_silver_rooms(int* count) {
    if (z64_file.master_quest == 1) {
        *count = sizeof(mq_silver_rooms) / sizeof(mq_silver_rooms[0]);
        return mq_silver_rooms;
    }

    *count = sizeof(vanilla_silver_rooms) / sizeof(vanilla_silver_rooms[0]);
    return vanilla_silver_rooms;
}

uint8_t count_tracker_get_silver_rooms_cleared() {
    uint8_t count = 0;
    int room_count;
    const silver_room_t* rooms = get_silver_rooms(&room_count);

    for (uint8_t i = 0; i < room_count; i++) {
        if (count_encounter_is_complete(rooms[i].id)) {
            count++;
        }
    }
    return count;
}

static int16_t previous_scene = -1;
static int8_t previous_room = -1;
static uint32_t previous_chest_flags;
static uint8_t pending_enemy_encounters[COUNT_ENCOUNTER_BYTES];
static uint8_t pending_enemy_switches[COUNT_ENCOUNTER_CAPACITY];
static uint8_t previous_bottle_items[BOTTLE_SLOT_COUNT];
static bool sky_temple_active;

#ifndef COUNT_TRACKER_TEST
typedef char assert_actor_health_offset[__builtin_offsetof(z64_actor_t, health) == 0xAF ? 1 : -1];
#endif

static void increment_saturating(uint8_t* count) {
    if (*count != UINT8_MAX) {
        (*count)++;
    }
}

static void snapshot_bottle_items() {
    for (uint8_t i = 0; i < BOTTLE_SLOT_COUNT; i++) {
        previous_bottle_items[i] = z64_file.items[BOTTLE_SLOT_FIRST + i];
    }
}

static uint32_t get_current_chest_flags() {
    return z64_file.scene_flags[z64_game.scene_index].chest | z64_game.chest_flags;
}

static uint32_t get_current_switch_flags() {
    return z64_file.scene_flags[z64_game.scene_index].swch | z64_game.swch_flags | z64_game.temp_swch_flags;
}

static uint32_t get_current_clear_flags() {
    return z64_file.scene_flags[z64_game.scene_index].clear | z64_game.clear_flags | z64_game.temp_clear_flags;
}

static bool get_event_check(uint8_t flag) {
    return z64_file.event_chk_inf[flag >> 4] & (1 << (flag & 0xF));
}

bool count_tracker_is_dungeon_completed(uint8_t dungeon_index) {
    switch (dungeon_index) {
        case SCENE_DEKU_TREE:
            return get_event_check(0x09);
        case SCENE_DODONGOS_CAVERN:
            return get_event_check(0x25);
        case SCENE_JABU_JABU:
            return get_event_check(0x37);
        case SCENE_FOREST_TEMPLE:
            return get_event_check(0x48);
        case SCENE_FIRE_TEMPLE:
            return get_event_check(0x49);
        case SCENE_WATER_TEMPLE:
            return get_event_check(0x4A);
        case SCENE_SHADOW_TEMPLE:
            return count_encounter_is_complete(DUNGEON_COMPLETION_BASE + 6);
        case SCENE_SPIRIT_TEMPLE:
            return count_encounter_is_complete(DUNGEON_COMPLETION_BASE + 7);
        default:
            return false;
    }
}

bool count_tracker_is_boss_cleared(uint8_t dungeon_index) {
    for (uint8_t i = 0; i < sizeof(bosses) / sizeof(*bosses); i++) {
        const boss_entry_t* boss = &bosses[i];
        if (boss->dungeon != dungeon_index) {
            continue;
        }

        if (count_encounter_is_complete(BOSS_ENCOUNTER_BASE + i) ||
            count_tracker_is_dungeon_completed(dungeon_index)) {
            return true;
        }

        return z64_game.scene_index == boss->boss_scene && (get_current_clear_flags() & boss->clear_flag);
    }

    if (dungeon_index == SCENE_INSIDE_GANONS_CASTLE) {
        return count_encounter_is_complete(GANONDORF_ENCOUNTER_ID);
    }

    return false;
}

static void track_boss_clears(uint32_t clear_flags) {
    for (uint8_t i = 0; i < sizeof(bosses) / sizeof(*bosses); i++) {
        const boss_entry_t* boss = &bosses[i];
        if (z64_game.scene_index == boss->boss_scene && (clear_flags & boss->clear_flag)) {
            count_encounter_complete(BOSS_ENCOUNTER_BASE + i);
        }

        if (z64_game.scene_index == SCENE_GANONDORF && z64_game.scene_load_flag != 0) {
            count_encounter_complete(GANONDORF_ENCOUNTER_ID);
        }
    }
}

static bool previous_spirit_medallion;
static bool previous_shadow_medallion;

static void track_spirit_and_shadow_completion(bool detect_changes) {
    if (detect_changes && z64_game.scene_index == SCENE_TWINROVA && !previous_spirit_medallion &&
        z64_file.spirit_medallion) {
        count_encounter_complete(DUNGEON_COMPLETION_BASE + 7);
    }
    if (detect_changes && z64_game.scene_index == SCENE_BONGO_BONGO && !previous_shadow_medallion &&
        z64_file.shadow_medallion) {
        count_encounter_complete(DUNGEON_COMPLETION_BASE + 6);
    }

    previous_spirit_medallion = z64_file.spirit_medallion;
    previous_shadow_medallion = z64_file.shadow_medallion;
}

static void track_opened_chests(uint32_t opened_flags) {
    z64_actor_t* actor = z64_link.incoming_item_actor;
    z64_actor_t* matching_actor = NULL;

    if (actor == NULL || actor->actor_id != ACTOR_EN_BOX || !(opened_flags & (1 << ENBOX_GET_TREASURE_FLAG(actor)))) {
        actor = z64_game.actor_list[ACTORTYPE_CHEST].first;
        while (actor != NULL) {
            uint32_t flag = 1 << ENBOX_GET_TREASURE_FLAG(actor);

            if (actor->actor_id == ACTOR_EN_BOX && (opened_flags & flag)) {
                if (matching_actor != NULL) {
                    return;
                }
                matching_actor = actor;
            }
            actor = actor->next;
        }
        actor = matching_actor;
    }

    if (actor != NULL) {
        uint8_t get_item_id = ENBOX_GET_ITEM_ID(actor);

        if (get_item_id == GI_ICE_TRAP) {
            increment_saturating(&extended_savectx.trap_count);
        } else if (get_item_id == GI_RUPEE_GOLD) {
            increment_saturating(&extended_savectx.gold_rupee_count);
            if (!z64_file.master_quest && z64_game.scene_index == SCENE_FIRE_TEMPLE) {
                count_objective_complete(OBJECTIVE_FIRE_GOLD_RUPEE);
            }
        }
    }

    if (z64_game.scene_index == SCENE_SPIRIT_TEMPLE && (opened_flags & 1)) {
        count_objective_complete(OBJECTIVE_SPIRIT_BOMBCHU_CHEST);
    }
    if (sky_temple_active && opened_flags != 0) {
        count_objective_complete(OBJECTIVE_SKY_TEMPLE);
        sky_temple_active = false;
    }
}

void count_tracker_update_chests() {
    if (z64_game.scene_index != previous_scene || z64_game.room_index != previous_room) {
        return;
    }

    if (z64_game.scene_index != SCENE_TREASURE_BOX_SHOP) {
        track_opened_chests(get_current_chest_flags() & ~previous_chest_flags);
    }
    previous_chest_flags = get_current_chest_flags();
}

static void complete_silver_room(uint8_t encounter_id) {
    if (!count_encounter_is_complete(encounter_id)) {
        int room_count;

        get_silver_rooms(&room_count);
        count_encounter_complete(encounter_id);
        if (extended_savectx.silver_rupee_rooms_cleared < room_count) {
            extended_savectx.silver_rupee_rooms_cleared++;
        }
    }
}

static void track_silver_rooms(uint32_t switches) {
    int room_count;
    const silver_room_t* rooms = get_silver_rooms(&room_count);

    for (uint8_t i = 0; i < room_count; i++) {
        const silver_room_t* room = &rooms[i];

        if (room->scene == z64_game.scene_index && (switches & (1 << room->switch_flag))) {
            complete_silver_room(room->id);
        }
    }
}

static enemy_encounter_t* get_enemy_encounters(int* count) {
    if (z64_file.master_quest == 1) {
        *count = sizeof(mq_enemy_encounters) / sizeof(mq_enemy_encounters[0]);
        return mq_enemy_encounters;
    }

    *count = sizeof(vanilla_enemy_encounters) / sizeof(vanilla_enemy_encounters[0]);
    return vanilla_enemy_encounters;
}

uint8_t count_tracker_get_enemy_count(uint8_t enemy_type) {
    if (enemy_type >= ENEMY_TYPE_COUNT) {
        return 0xFF;
    }

    uint8_t enemy_count = extended_savectx.enemy_counts[enemy_type];
    int encounter_count;
    enemy_encounter_t* encounters = get_enemy_encounters(&encounter_count);

    for (int i = 0; i < encounter_count; i++) {
        enemy_encounter_t* encounter = &encounters[i];

        if (encounter->type == enemy_type &&
            (pending_enemy_encounters[encounter->id >> 3] & (1 << (encounter->id & 7)))) {
            increment_saturating(&enemy_count);
        }
    }

    return enemy_count;
}

static int get_enemy_type(z64_actor_t* actor) {
    uint8_t actor_type = actor->variable & 0xFF;

    switch (actor->actor_id) {
        case ACTOR_EN_IK:
            return ENEMY_IRON_KNUCKLE;
        case ACTOR_EN_WF:
            return actor_type != 0 ? ENEMY_WHITE_WOLFOS : -1;
        case ACTOR_EN_ZF:
            return (actor_type == 0 || actor_type == 1 || actor_type == 0x80 || actor_type == 0xFF) ? ENEMY_LIZALFOS
                                                                                                    : -1;
        case ACTOR_EN_RD:
            return (actor_type == 0xFD || actor_type == 0xFE) ? ENEMY_GIBDO : -1;
        case ACTOR_EN_BA:
            return actor_type < 3 ? ENEMY_TENTACLE : -1;
        case ACTOR_EN_TEST:
            return ENEMY_STALFOS;
        case ACTOR_EN_DH:
            return ENEMY_DEAD_HAND;
        case ACTOR_EN_FD:
            return ENEMY_FLARE_DANCER;
        case ACTOR_EN_SKJ:
            return ENEMY_SKULL_KID;
        default:
            return -1;
    }
}

static enemy_encounter_t* find_enemy_encounter(z64_actor_t* actor, int enemy_type) {
    int count;
    enemy_encounter_t* encounters = get_enemy_encounters(&count);

    for (int i = 0; i < count; i++) {
        enemy_encounter_t* encounter = &encounters[i];

        if (encounter->type == enemy_type && encounter->scene == z64_game.scene_index &&
            encounter->room == actor->room_index && encounter->x == (int16_t)actor->pos_1.x &&
            encounter->y == (int16_t)actor->pos_1.y && encounter->z == (int16_t)actor->pos_1.z) {
            return encounter;
        }
    }

    return NULL;
}

static void complete_enemy_encounter(enemy_encounter_t* encounter) {
    if (count_encounter_is_complete(encounter->id)) {
        return;
    }

    count_encounter_complete(encounter->id);
    increment_saturating(&extended_savectx.enemy_counts[encounter->type]);
}

static void track_nabooru_iron_knuckle() {
    if (z64_game.scene_index == SCENE_TWINROVA && z64_game.room_index == 1 &&
        (get_current_clear_flags() & NABOORU_KNUCKLE_CLEAR_FLAG)) {
        int count;
        enemy_encounter_t* encounters = get_enemy_encounters(&count);

        for (int i = 0; i < count; i++) {
            enemy_encounter_t* encounter = &encounters[i];

            if (encounter->type == ENEMY_IRON_KNUCKLE && encounter->scene == SCENE_TWINROVA && encounter->room == 1) {
                complete_enemy_encounter(encounter);
                count_objective_complete(OBJECTIVE_NABOORU_KNUCKLE);
                break;
            }
        }
    }
}

static bool enemy_encounters_complete(const uint8_t* ids, uint8_t count) {
    for (uint8_t i = 0; i < count; i++) {
        if (!count_encounter_is_complete(ENEMY_ENCOUNTER_BASE + ids[i])) {
            return false;
        }
    }

    return true;
}

static void track_objective_progress() {
    static const uint8_t vanilla_dodongo_lizalfos[] = { 11, 12, 13, 14 };
    static const uint8_t mq_dodongo_lizalfos[] = { 17, 18, 19, 20 };
    static const uint8_t vanilla_fire_flare_dancers[] = { 39, 40 };
    static const uint8_t mq_fire_flare_dancers[] = { 81, 82 };
    const uint8_t* dodongo_ids = z64_file.master_quest ? mq_dodongo_lizalfos : vanilla_dodongo_lizalfos;
    const uint8_t* flare_ids = z64_file.master_quest ? mq_fire_flare_dancers : vanilla_fire_flare_dancers;

    if (enemy_encounters_complete(dodongo_ids, 4)) {
        count_objective_complete(OBJECTIVE_DODONGO_LIZALFOS);
    }
    if (enemy_encounters_complete(flare_ids, 2)) {
        count_objective_complete(OBJECTIVE_FIRE_FLARE_DANCERS);
    }
    if (count_item_get_set(0x12)) {
        count_objective_complete(OBJECTIVE_BOMBCHU_BOWLING);
    }
}

static uint8_t get_cow_objective(z64_actor_t* cow) {
    if (z64_game.scene_index == SCENE_LINKS_HOUSE) {
        return OBJECTIVE_COW_0;
    }

    uint32_t hash = (uint16_t)z64_game.scene_index;
    hash = hash * 31 + (uint8_t)cow->room_index;
    hash = hash * 31 + (uint16_t)cow->pos_1.x;
    hash = hash * 31 + (uint16_t)cow->pos_1.y;
    hash = hash * 31 + (uint16_t)cow->pos_1.z;
    return OBJECTIVE_COW_1 + hash % 15;
}

static void track_cow_milk_awards() {
    z64_actor_t* cow = z64_link.incoming_item_actor;
    bool received_milk = false;

    for (uint8_t i = 0; i < BOTTLE_SLOT_COUNT; i++) {
        uint8_t current = z64_file.items[BOTTLE_SLOT_FIRST + i];
        if (previous_bottle_items[i] != ITEM_MILK && current == ITEM_MILK) {
            received_milk = true;
        }
        previous_bottle_items[i] = current;
    }

    if (received_milk && cow != NULL && cow->actor_id == ACTOR_EN_COW) {
        count_objective_complete(get_cow_objective(cow));
    }
}

static uint8_t get_enemy_completion_switch(z64_actor_t* actor, int enemy_type) {
    uint8_t switch_flag = actor->variable >> 8;

    if ((enemy_type == ENEMY_WHITE_WOLFOS || enemy_type == ENEMY_LIZALFOS) && switch_flag != 0xFF) {
        return switch_flag;
    }
    if (enemy_type == ENEMY_TENTACLE) {
        return switch_flag;
    }
    return 0xFF;
}

static void stage_enemy_encounter(uint8_t encounter_id, uint8_t completion_switch) {
    pending_enemy_encounters[encounter_id >> 3] |= 1 << (encounter_id & 7);
    pending_enemy_switches[encounter_id] = completion_switch;
}

static void track_enemy_deaths() {
    for (int category = 0; category < ACTOR_CATEGORY_COUNT; category++) {
        z64_actor_t* actor = z64_game.actor_list[category].first;

        while (actor != NULL) {
            int enemy_type = get_enemy_type(actor);

            if (enemy_type >= 0 && actor->health <= 0) {
                enemy_encounter_t* encounter = find_enemy_encounter(actor, enemy_type);

                if (encounter != NULL && !count_encounter_is_complete(encounter->id)) {
                    if (z64_game.scene_index <= SCENE_INSIDE_GANONS_CASTLE) {
                        stage_enemy_encounter(encounter->id, get_enemy_completion_switch(actor, enemy_type));
                    } else {
                        complete_enemy_encounter(encounter);
                    }
                }
            }
            actor = actor->next;
        }
    }
}

static void commit_pending_enemy_encounters(uint32_t switches, uint32_t clear) {
    int count;
    uint32_t room_clear = (z64_game.room_index >= 0 && z64_game.room_index < ROOM_CLEAR_FLAG_COUNT)
                              ? clear & (1u << z64_game.room_index)
                              : 0;

    enemy_encounter_t* encounters = get_enemy_encounters(&count);
    for (int i = 0; i < count; i++) {
        enemy_encounter_t* encounter = &encounters[i];
        uint8_t encounter_id = encounter->id;

        if (pending_enemy_encounters[encounter_id >> 3] & (1 << (encounter_id & 7)) &&
            ((pending_enemy_switches[encounter_id] == 0xFF && room_clear) ||
             (pending_enemy_switches[encounter_id] != 0xFF &&
              (switches & (1 << pending_enemy_switches[encounter_id]))))) {
            complete_enemy_encounter(encounter);
            pending_enemy_encounters[encounter_id >> 3] &= ~(1 << (encounter_id & 7));
        }
    }
}

void count_tracker_update() {
    int16_t scene = z64_game.scene_index;
    int8_t room = z64_game.room_index;

    if (scene != previous_scene || room != previous_room) {
        sky_temple_active = z64_file.entrance_index == 0x00A0;
        if (scene == SCENE_PHANTOM_GANON && previous_scene == SCENE_FOREST_TEMPLE) {
            count_objective_complete(OBJECTIVE_FOREST_BOSS_DOOR);
        }
        if (scene == SCENE_GANONDORF &&
            (previous_scene == SCENE_INSIDE_GANONS_CASTLE || previous_scene == SCENE_GANONS_TOWER)) {
            count_objective_complete(OBJECTIVE_GANON_BOSS_DOOR);
        }
        previous_scene = scene;
        previous_room = room;
        previous_chest_flags = get_current_chest_flags();
        snapshot_bottle_items();
        track_spirit_and_shadow_completion(false);
        for (int i = 0; i < COUNT_ENCOUNTER_BYTES; i++) {
            pending_enemy_encounters[i] = 0;
        }
        return;
    }

    count_tracker_update_chests();
    uint32_t switches = get_current_switch_flags();
    uint32_t clear = get_current_clear_flags();

    track_silver_rooms(switches);
    track_enemy_deaths();
    track_nabooru_iron_knuckle();
    track_boss_clears(clear);
    track_spirit_and_shadow_completion(true);
    commit_pending_enemy_encounters(switches, clear);
    track_objective_progress();
    track_cow_milk_awards();
}
