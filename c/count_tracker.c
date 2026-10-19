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
#define ACTOR_EN_BA 0x00DE
#define ACTOR_EN_IK 0x0113
#define ACTOR_EN_WF 0x01AF
#define GI_RUPEE_GOLD 0x56
#define GI_ICE_TRAP 0x7C
#define ENBOX_GET_ITEM_ID(actor) (((actor)->variable >> 5) & 0x7F)
#define ENBOX_GET_TREASURE_FLAG(actor) ((actor)->variable & 0x1F)
#define SILVER_ROOM_COUNT 22
#define ENEMY_ENCOUNTER_BASE 32
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
    ENEMY_TYPE_COUNT,
} enemy_type_t;

typedef struct {
    uint8_t type;
    uint8_t scene;
    int8_t room;
    int16_t x;
    int16_t y;
    int16_t z;
} enemy_encounter_t;

#define ENCOUNTER(type, scene, room, x, y, z) \
    {ENEMY_##type, scene, room, x, y, z}

static enemy_encounter_t vanilla_enemy_encounters[] = {
    ENCOUNTER(IRON_KNUCKLE, 0x06, 10, -1250, 1003, 1476),
    ENCOUNTER(IRON_KNUCKLE, 0x06, 20, 1515, 1003, 1473),
    ENCOUNTER(IRON_KNUCKLE, 0x0A, 4, -291, 520, 37),
    ENCOUNTER(IRON_KNUCKLE, 0x0A, 4, -290, 520, -32),
    ENCOUNTER(IRON_KNUCKLE, 0x17, 1, -1235, 0, 1268),
    ENCOUNTER(WHITE_WOLFOS, 0x09, 7, -1096, 282, 964),
    ENCOUNTER(WHITE_WOLFOS, 0x0B, 3, -1377, 159, -2795),
    ENCOUNTER(WHITE_WOLFOS, 0x0B, 3, -1675, 159, -2849),
    ENCOUNTER(LIZALFOS, 0x01, 3, 3969, 100, -2161),
    ENCOUNTER(LIZALFOS, 0x01, 3, 2829, 100, -1961),
    ENCOUNTER(LIZALFOS, 0x01, 3, 4261, 531, -1055),
    ENCOUNTER(LIZALFOS, 0x01, 3, 4442, 531, -1386),
    ENCOUNTER(LIZALFOS, 0x06, 4, -1074, 501, -717),
    ENCOUNTER(LIZALFOS, 0x06, 4, -730, 480, -736),
    ENCOUNTER(LIZALFOS, 0x06, 26, -545, 1743, -830),
    ENCOUNTER(LIZALFOS, 0x06, 26, -24, 1743, -304),
    ENCOUNTER(GIBDO, 0x07, 7, 3498, -543, 721),
    ENCOUNTER(GIBDO, 0x07, 7, 3212, -543, 726),
    ENCOUNTER(GIBDO, 0x07, 20, 4253, -1363, -1070),
    ENCOUNTER(GIBDO, 0x07, 20, 4527, -1363, -1081),
    ENCOUNTER(GIBDO, 0x08, 2, -2070, 0, -854),
    ENCOUNTER(TENTACLE, 0x02, 8, 1, 20, -5801),
    ENCOUNTER(TENTACLE, 0x02, 10, -1561, 20, -4681),
    ENCOUNTER(TENTACLE, 0x02, 11, 660, 20, -5680),
    ENCOUNTER(STALFOS, 0x03, 6, 273, 467, -3378),
    ENCOUNTER(STALFOS, 0x03, 6, -38, 467, -3382),
    ENCOUNTER(STALFOS, 0x06, 3, -1565, 30, -670),
    ENCOUNTER(STALFOS, 0x07, 9, 3752, -1143, 3413),
    ENCOUNTER(STALFOS, 0x07, 21, 2746, -991, -2011),
    ENCOUNTER(STALFOS, 0x07, 21, 1209, -974, -1982),
    ENCOUNTER(STALFOS, 0x0A, 2, -183, 260, -12),
    ENCOUNTER(STALFOS, 0x0A, 2, 161, 260, -7),
    ENCOUNTER(STALFOS, 0x0B, 1, -1629, -90, -19),
    ENCOUNTER(STALFOS, 0x0B, 1, -1870, -90, -357),
    ENCOUNTER(DEAD_HAND, 0x07, 4, -2424, -63, -413),
    ENCOUNTER(DEAD_HAND, 0x08, 4, 1281, -20, 211),
    ENCOUNTER(FLARE_DANCER, 0x04, 3, -361, 80, -1300),
    ENCOUNTER(FLARE_DANCER, 0x04, 24, -2928, 2920, 139),
};

static enemy_encounter_t mq_enemy_encounters[] = {
    ENCOUNTER(IRON_KNUCKLE, 0x04, 18, 338, 20, -1452),
    ENCOUNTER(IRON_KNUCKLE, 0x06, 10, -1250, 1003, 1476),
    ENCOUNTER(IRON_KNUCKLE, 0x06, 20, 1515, 1003, 1473),
    ENCOUNTER(IRON_KNUCKLE, 0x06, 22, 681, 1633, -1317),
    ENCOUNTER(IRON_KNUCKLE, 0x0A, 4, -291, 520, 37),
    ENCOUNTER(IRON_KNUCKLE, 0x0A, 4, -290, 520, -32),
    ENCOUNTER(IRON_KNUCKLE, 0x0B, 1, -1629, -90, -19),
    ENCOUNTER(IRON_KNUCKLE, 0x0B, 5, 1378, -65, -2758),
    ENCOUNTER(IRON_KNUCKLE, 0x0D, 0, 3, 150, 690),
    ENCOUNTER(IRON_KNUCKLE, 0x0D, 17, -945, 411, 822),
    ENCOUNTER(WHITE_WOLFOS, 0x09, 3, 271, 0, -401),
    ENCOUNTER(WHITE_WOLFOS, 0x09, 5, -1564, 41, -957),
    ENCOUNTER(WHITE_WOLFOS, 0x09, 5, -1156, 61, -683),
    ENCOUNTER(LIZALFOS, 0x01, 3, 3969, 100, -2161),
    ENCOUNTER(LIZALFOS, 0x01, 3, 2829, 100, -1961),
    ENCOUNTER(LIZALFOS, 0x01, 3, 4261, 531, -1055),
    ENCOUNTER(LIZALFOS, 0x01, 3, 4442, 531, -1386),
    ENCOUNTER(LIZALFOS, 0x01, 8, 1558, 16, -3540),
    ENCOUNTER(LIZALFOS, 0x02, 4, -1229, 632, -1446),
    ENCOUNTER(LIZALFOS, 0x02, 7, 0, -340, -4846),
    ENCOUNTER(LIZALFOS, 0x02, 14, 928, -1153, -3216),
    ENCOUNTER(LIZALFOS, 0x04, 4, 2351, 2520, -26),
    ENCOUNTER(LIZALFOS, 0x04, 5, 1777, 2800, 561),
    ENCOUNTER(LIZALFOS, 0x04, 5, 1900, 2800, -835),
    ENCOUNTER(LIZALFOS, 0x04, 5, 2181, 2800, -71),
    ENCOUNTER(LIZALFOS, 0x04, 5, 2263, 2800, 519),
    ENCOUNTER(LIZALFOS, 0x04, 25, -824, 2800, -1142),
    ENCOUNTER(LIZALFOS, 0x05, 10, -1024, 1084, 121),
    ENCOUNTER(LIZALFOS, 0x05, 18, 1720, 335, 283),
    ENCOUNTER(LIZALFOS, 0x05, 18, 1717, 335, 84),
    ENCOUNTER(LIZALFOS, 0x05, 20, 653, 706, 953),
    ENCOUNTER(LIZALFOS, 0x05, 20, 658, 992, 184),
    ENCOUNTER(LIZALFOS, 0x0B, 7, 1228, -80, -198),
    ENCOUNTER(GIBDO, 0x06, 2, -502, 30, -734),
    ENCOUNTER(GIBDO, 0x06, 2, -420, 30, -1333),
    ENCOUNTER(GIBDO, 0x06, 2, -417, 30, -1141),
    ENCOUNTER(GIBDO, 0x06, 21, 1889, 873, -636),
    ENCOUNTER(GIBDO, 0x07, 7, 3212, -543, 726),
    ENCOUNTER(GIBDO, 0x07, 7, 3498, -543, 721),
    ENCOUNTER(GIBDO, 0x07, 20, 4253, -1363, -1070),
    ENCOUNTER(GIBDO, 0x07, 20, 4527, -1363, -1081),
    ENCOUNTER(GIBDO, 0x08, 2, -1753, 0, -738),
    ENCOUNTER(GIBDO, 0x0D, 18, -1429, 150, 1045),
    ENCOUNTER(TENTACLE, 0x02, 8, 1, 20, -5801),
    ENCOUNTER(TENTACLE, 0x02, 9, 1561, 20, -4679),
    ENCOUNTER(TENTACLE, 0x02, 10, -1562, 20, -4683),
    ENCOUNTER(STALFOS, 0x03, 5, -1000, 383, -1440),
    ENCOUNTER(STALFOS, 0x04, 14, -2021, 4180, -1019),
    ENCOUNTER(STALFOS, 0x04, 14, -1625, 4120, -627),
    ENCOUNTER(STALFOS, 0x04, 15, 102, 0, -598),
    ENCOUNTER(STALFOS, 0x04, 15, 283, 0, -320),
    ENCOUNTER(STALFOS, 0x05, 3, -2023, 300, 765),
    ENCOUNTER(STALFOS, 0x05, 3, -2023, 300, 966),
    ENCOUNTER(STALFOS, 0x05, 6, -3114, 560, -382),
    ENCOUNTER(STALFOS, 0x05, 6, -3211, 560, -455),
    ENCOUNTER(STALFOS, 0x05, 6, -3213, 560, -355),
    ENCOUNTER(STALFOS, 0x05, 16, -1285, 108, -2986),
    ENCOUNTER(STALFOS, 0x05, 19, 1554, 780, 268),
    ENCOUNTER(STALFOS, 0x05, 19, 1557, 780, 107),
    ENCOUNTER(STALFOS, 0x05, 19, 1486, 780, 192),
    ENCOUNTER(STALFOS, 0x06, 15, 1161, 480, -1015),
    ENCOUNTER(STALFOS, 0x06, 15, 1163, 480, -678),
    ENCOUNTER(STALFOS, 0x06, 27, -1053, 150, -1799),
    ENCOUNTER(STALFOS, 0x07, 14, 1114, -1083, 1081),
    ENCOUNTER(STALFOS, 0x07, 14, 1489, -1323, 1083),
    ENCOUNTER(STALFOS, 0x07, 21, 1209, -974, -1982),
    ENCOUNTER(STALFOS, 0x07, 21, 2746, -991, -2011),
    ENCOUNTER(STALFOS, 0x09, 7, -1096, 282, 964),
    ENCOUNTER(STALFOS, 0x0A, 2, -183, 260, -12),
    ENCOUNTER(STALFOS, 0x0A, 2, 161, 260, -7),
    ENCOUNTER(STALFOS, 0x0B, 3, -1396, 159, -2611),
    ENCOUNTER(STALFOS, 0x0B, 3, -1505, 159, -2788),
    ENCOUNTER(STALFOS, 0x0D, 5, 1018, 110, 631),
    ENCOUNTER(STALFOS, 0x0D, 5, 757, 110, 792),
    ENCOUNTER(DEAD_HAND, 0x07, 4, -2434, -63, -405),
    ENCOUNTER(DEAD_HAND, 0x07, 17, -4306, -1363, -542),
    ENCOUNTER(DEAD_HAND, 0x08, 4, 1280, -16, 210),
    ENCOUNTER(FLARE_DANCER, 0x04, 3, -361, 80, -1300),
    ENCOUNTER(FLARE_DANCER, 0x04, 24, -2928, 2920, 139),
    ENCOUNTER(IRON_KNUCKLE, 0x17, 1, -1235, 0, 1268),
};

typedef struct {
    uint8_t scene;
    uint8_t vanilla_switch_flag;
    uint8_t mq_switch_flag;
} silver_room_t;

static silver_room_t silver_rooms[SILVER_ROOM_COUNT] = {
    {SCENE_DODONGOS_CAVERN, 0xFF, 31},
    {SCENE_ICE_CAVERN, 8, 8},
    {SCENE_ICE_CAVERN, 9, 9},
    {SCENE_BOTTOM_OF_THE_WELL, 31, 31},
    {SCENE_SHADOW_TEMPLE, 1, 1},
    {SCENE_SHADOW_TEMPLE, 0xFF, 3},
    {SCENE_SHADOW_TEMPLE, 9, 17},
    {SCENE_SHADOW_TEMPLE, 8, 8},
    {SCENE_GERUDO_TRAINING_GROUND, 28, 28},
    {SCENE_GERUDO_TRAINING_GROUND, 12, 12},
    {SCENE_GERUDO_TRAINING_GROUND, 27, 27},
    {SCENE_SPIRIT_TEMPLE, 5, 0xFF},
    {SCENE_SPIRIT_TEMPLE, 2, 0xFF},
    {SCENE_SPIRIT_TEMPLE, 0xFF, 31},
    {SCENE_SPIRIT_TEMPLE, 10, 0xFF},
    {SCENE_SPIRIT_TEMPLE, 0xFF, 0},
    {SCENE_INSIDE_GANONS_CASTLE, 11, 0xFF},
    {SCENE_INSIDE_GANONS_CASTLE, 18, 0xFF},
    {SCENE_INSIDE_GANONS_CASTLE, 9, 1},
    {SCENE_INSIDE_GANONS_CASTLE, 0xFF, 11},
    {SCENE_INSIDE_GANONS_CASTLE, 0xFF, 2},
    {SCENE_INSIDE_GANONS_CASTLE, 14, 0xFF},
};

static int16_t previous_scene = -1;
static int8_t previous_room = -1;
static uint32_t previous_chest_flags;
static uint8_t pending_enemy_encounters[COUNT_ENCOUNTER_BYTES];
static uint8_t pending_enemy_switches[COUNT_ENCOUNTER_CAPACITY];

#ifndef COUNT_TRACKER_TEST
typedef char assert_actor_health_offset[
    __builtin_offsetof(z64_actor_t, health) == 0xAF ? 1 : -1];
#endif

static void increment_saturating(uint8_t* count) {
    if (*count != UINT8_MAX) {
        (*count)++;
    }
}

static uint32_t get_current_chest_flags() {
    return z64_file.scene_flags[z64_game.scene_index].chest |
        z64_game.chest_flags;
}

static uint32_t get_current_switch_flags() {
    return z64_file.scene_flags[z64_game.scene_index].swch |
        z64_game.swch_flags |
        z64_game.temp_swch_flags;
}

static uint32_t get_current_clear_flags() {
    return z64_file.scene_flags[z64_game.scene_index].clear |
        z64_game.clear_flags |
        z64_game.temp_clear_flags;
}

static void track_opened_chests(uint32_t opened_flags) {
    z64_actor_t* actor = z64_link.incoming_item_actor;
    z64_actor_t* matching_actor = NULL;

    if (actor == NULL || actor->actor_id != ACTOR_EN_BOX ||
        !(opened_flags & (1 << ENBOX_GET_TREASURE_FLAG(actor)))) {
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
        }
    }
}

void count_tracker_update_chests() {
    if (z64_game.scene_index != previous_scene ||
        z64_game.room_index != previous_room) {
        return;
    }

    if (z64_game.scene_index != SCENE_TREASURE_BOX_SHOP) {
        track_opened_chests(
            get_current_chest_flags() & ~previous_chest_flags
        );
    }
    previous_chest_flags = get_current_chest_flags();
}

static void complete_silver_room(uint8_t encounter_id) {
    if (!count_encounter_is_complete(encounter_id)) {
        count_encounter_complete(encounter_id);
        if (extended_savectx.silver_rupee_rooms_cleared < SILVER_ROOM_COUNT) {
            extended_savectx.silver_rupee_rooms_cleared++;
        }
    }
}

static void track_silver_rooms(uint32_t switches) {
    for (uint8_t i = 0; i < SILVER_ROOM_COUNT; i++) {
        silver_room_t* room = &silver_rooms[i];
        uint8_t switch_flag = z64_file.master_quest == 1 ?
            room->mq_switch_flag : room->vanilla_switch_flag;

        if (room->scene == z64_game.scene_index &&
            switch_flag != 0xFF &&
            (switches & (1 << switch_flag))) {
            complete_silver_room(i);
        }
    }
}

static enemy_encounter_t* get_enemy_encounters(int* count) {
    if (z64_file.master_quest == 1) {
        *count = sizeof(mq_enemy_encounters) / sizeof(mq_enemy_encounters[0]);
        return mq_enemy_encounters;
    }

    *count = sizeof(vanilla_enemy_encounters) /
        sizeof(vanilla_enemy_encounters[0]);
    return vanilla_enemy_encounters;
}

static int get_enemy_type(z64_actor_t* actor) {
    uint8_t actor_type = actor->variable & 0xFF;

    switch (actor->actor_id) {
        case ACTOR_EN_IK:
            return ENEMY_IRON_KNUCKLE;
        case ACTOR_EN_WF:
            return actor_type != 0 ? ENEMY_WHITE_WOLFOS : -1;
        case ACTOR_EN_ZF:
            return actor_type == 0 || actor_type == 1 ||
                    actor_type == 0x80 || actor_type == 0xFF ?
                ENEMY_LIZALFOS : -1;
        case ACTOR_EN_RD:
            return actor_type == 0xFD || actor_type == 0xFE ?
                ENEMY_GIBDO : -1;
        case ACTOR_EN_BA:
            return actor_type < 3 ? ENEMY_TENTACLE : -1;
        case ACTOR_EN_TEST:
            return ENEMY_STALFOS;
        case ACTOR_EN_DH:
            return ENEMY_DEAD_HAND;
        case ACTOR_EN_FD:
            return ENEMY_FLARE_DANCER;
        default:
            return -1;
    }
}

static int find_enemy_encounter(z64_actor_t* actor, int enemy_type) {
    int count;
    enemy_encounter_t* encounters = get_enemy_encounters(&count);

    for (int i = 0; i < count; i++) {
        enemy_encounter_t* encounter = &encounters[i];

        if (encounter->type == enemy_type &&
            encounter->scene == z64_game.scene_index &&
            encounter->room == actor->room_index &&
            encounter->x == (int16_t)actor->pos_1.x &&
            encounter->y == (int16_t)actor->pos_1.y &&
            encounter->z == (int16_t)actor->pos_1.z) {
            return ENEMY_ENCOUNTER_BASE + i;
        }
    }

    return -1;
}

static void complete_enemy_encounter(uint8_t encounter_id) {
    int count;
    enemy_encounter_t* encounters = get_enemy_encounters(&count);
    int encounter_index = encounter_id - ENEMY_ENCOUNTER_BASE;

    if (encounter_index < 0 || encounter_index >= count ||
        count_encounter_is_complete(encounter_id)) {
        return;
    }

    count_encounter_complete(encounter_id);
    increment_saturating(
        &extended_savectx.enemy_counts[encounters[encounter_index].type]
    );
}

static void track_nabooru_iron_knuckle() {
    if (z64_game.scene_index == SCENE_TWINROVA &&
        z64_game.room_index == 1 &&
        (get_current_clear_flags() & NABOORU_KNUCKLE_CLEAR_FLAG)) {
        int count;
        enemy_encounter_t* encounters = get_enemy_encounters(&count);

        for (int i = 0; i < count; i++) {
            enemy_encounter_t* encounter = &encounters[i];

            if (encounter->type == ENEMY_IRON_KNUCKLE &&
                encounter->scene == 0x17 &&
                encounter->room == 1) {
                complete_enemy_encounter(ENEMY_ENCOUNTER_BASE + i);
                break;
            }
        }
    }
}

static uint8_t get_enemy_completion_switch(
    z64_actor_t* actor,
    int enemy_type
) {
    uint8_t switch_flag = actor->variable >> 8;

    if ((enemy_type == ENEMY_WHITE_WOLFOS ||
         enemy_type == ENEMY_LIZALFOS) &&
        switch_flag != 0xFF) {
        return switch_flag;
    }
    if (enemy_type == ENEMY_TENTACLE) {
        return switch_flag;
    }
    return 0xFF;
}

static void stage_enemy_encounter(
    uint8_t encounter_id,
    uint8_t completion_switch
) {
    pending_enemy_encounters[encounter_id >> 3] |=
        1 << (encounter_id & 7);
    pending_enemy_switches[encounter_id] = completion_switch;
}

static void track_enemy_deaths() {
    for (int category = 0; category < 12; category++) {
        z64_actor_t* actor = z64_game.actor_list[category].first;

        while (actor != NULL) {
            int enemy_type = get_enemy_type(actor);

            if (enemy_type >= 0 && actor->health <= 0) {
                int encounter_id = find_enemy_encounter(actor, enemy_type);

                if (encounter_id >= 0 &&
                    !count_encounter_is_complete(encounter_id)) {
                    if (z64_game.scene_index <= SCENE_INSIDE_GANONS_CASTLE) {
                        stage_enemy_encounter(
                            encounter_id,
                            get_enemy_completion_switch(actor, enemy_type)
                        );
                    } else {
                        complete_enemy_encounter(encounter_id);
                    }
                }
            }
            actor = actor->next;
        }
    }
}

static void commit_pending_enemy_encounters(
    uint32_t switches,
    uint32_t clear
) {
    int count;
    uint32_t room_clear =
        z64_game.room_index >= 0 && z64_game.room_index < 32 ?
        clear & (1u << z64_game.room_index) : 0;

    get_enemy_encounters(&count);
    for (int i = 0; i < count; i++) {
        uint8_t encounter_id = ENEMY_ENCOUNTER_BASE + i;

        if (pending_enemy_encounters[encounter_id >> 3] &
            (1 << (encounter_id & 7)) &&
            ((pending_enemy_switches[encounter_id] == 0xFF && room_clear) ||
             (pending_enemy_switches[encounter_id] != 0xFF &&
              (switches &
               (1 << pending_enemy_switches[encounter_id]))))) {
            complete_enemy_encounter(encounter_id);
            pending_enemy_encounters[encounter_id >> 3] &=
                ~(1 << (encounter_id & 7));
        }
    }
}

void count_tracker_update() {
    int16_t scene = z64_game.scene_index;
    int8_t room = z64_game.room_index;

    if (scene != previous_scene || room != previous_room) {
        previous_scene = scene;
        previous_room = room;
        previous_chest_flags = get_current_chest_flags();
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
    commit_pending_enemy_encounters(switches, clear);
}
