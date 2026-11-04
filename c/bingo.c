#include "bingo.h"

#include "area_info.h"
#include "count_tracker.h"
#include "counts.h"
#include "dungeon_info.h"
#include "save.h"
#include "z64.h"

#define DUNGEON_ITEM_BOSS_KEY 0x01
#define DUNGEON_ITEM_COMPASS 0x02
#define DUNGEON_ITEM_MAP 0x04

typedef enum {
    SWORDS = 1 << 0,
    SHIELDS = 1 << 1,
    TUNICS = 1 << 2,
    BOOTS = 1 << 3
} equipment_set_t;

static bingo_progress_t progress_at_least(uint16_t current, uint16_t target) {
    bingo_progress_t progress = { current, target, current >= target };
    return progress;
}

static bingo_progress_t progress_exactly(uint16_t current, uint16_t target) {
    bingo_progress_t progress = { current, target, current == target };
    return progress;
}

static bingo_progress_t progress_bool(bool complete) {
    return progress_at_least(complete ? 1 : 0, 1);
}

static uint8_t item_owned(uint8_t slot, uint8_t item) {
    return z64_file.items[slot] == item;
}

static uint8_t inventory_item_count(uint8_t item) {
    uint8_t count = 0;
    for (uint8_t slot = 0; slot < 24; slot++) {
        count += z64_file.items[slot] == item;
    }
    return count;
}

static uint8_t bottle_slot_count(void) {
    uint8_t count = 0;
    for (uint8_t slot = Z64_SLOT_BOTTLE_1; slot <= Z64_SLOT_BOTTLE_4; slot++) {
        count += z64_file.items[slot] != (uint8_t)Z64_ITEM_NULL;
    }
    return count;
}

static uint8_t sword_count(void) {
    return z64_file.kokiri_sword + z64_file.master_sword +
           (z64_file.giants_knife || z64_file.broken_giants_knife);
}

static uint8_t dungeon_item_owned(uint8_t scene, uint8_t item) {
    return !!(z64_file.dungeon_items[scene].items & item);
}

static uint8_t map_and_compass_count(uint8_t scene) {
    return dungeon_item_owned(scene, DUNGEON_ITEM_MAP) + dungeon_item_owned(scene, DUNGEON_ITEM_COMPASS);
}

static uint8_t dungeon_completed_count(void) {
    uint8_t count = 0;
    for (uint8_t scene = SCENE_DEKU_TREE; scene <= SCENE_SPIRIT_TEMPLE; scene++) {
        count += count_tracker_is_dungeon_completed(scene);
    }
    return count;
}

static uint8_t dungeon_boss_cleared(uint8_t scene) {
    return count_tracker_is_boss_cleared(scene);
}

static uint8_t area_tokens(uint8_t area) {
    uint16_t count = bingo_get_area_count(area, ACOUNT_TOKEN);
    return count == UINT16_MAX ? 0 : count;
}

static uint8_t area_hps(uint8_t area) {
    uint16_t count = bingo_get_area_count(area, ACOUNT_HEART_PIECE);
    return count == UINT16_MAX ? 0 : count;
}

static uint8_t dungeon_tokens(uint8_t scene) {
    uint16_t count = bingo_get_dungeon_count(scene, DCOUNT_TOKEN);
    return count == UINT16_MAX ? 0 : count;
}

static uint8_t total_keys_obtained(uint8_t scene) {
    uint16_t count = bingo_get_dungeon_count(scene, DCOUNT_SMALL_KEY);
    return count == UINT16_MAX ? 0 : count;
}

static uint8_t current_keys(uint8_t scene) {
    return z64_file.dungeon_keys[scene] < 0 ? 0 : z64_file.dungeon_keys[scene];
}

static uint8_t child_dungeons_with_tokens(void) {
    return (dungeon_tokens(SCENE_DEKU_TREE) > 0) + (dungeon_tokens(SCENE_DODONGOS_CAVERN) > 0) +
           (dungeon_tokens(SCENE_JABU_JABU) > 0);
}

static uint8_t adult_dungeons_with_tokens(void) {
    return (dungeon_tokens(SCENE_FOREST_TEMPLE) > 0) + (dungeon_tokens(SCENE_FIRE_TEMPLE) > 0) +
           (dungeon_tokens(SCENE_WATER_TEMPLE) > 0) + (dungeon_tokens(SCENE_SHADOW_TEMPLE) > 0) +
           (dungeon_tokens(SCENE_SPIRIT_TEMPLE) > 0);
}

static uint8_t adult_dungeons_with_current_keys(void) {
    return (current_keys(SCENE_FOREST_TEMPLE) > 0) + (current_keys(SCENE_FIRE_TEMPLE) > 0) +
           (current_keys(SCENE_WATER_TEMPLE) > 0) + (current_keys(SCENE_SHADOW_TEMPLE) > 0) +
           (current_keys(SCENE_SPIRIT_TEMPLE) > 0);
}

static bingo_progress_t equipment_set_progress(equipment_set_t sets) {
    uint8_t current = ((sets & SWORDS) && sword_count() >= 3) +
                      ((sets & SHIELDS) && get_global_count(GCOUNT_SHIELD) >= 3) +
                      ((sets & TUNICS) && get_global_count(GCOUNT_TUNIC) >= 3) +
                      ((sets & BOOTS) && get_global_count(GCOUNT_BOOT) >= 3);
    uint8_t target = !!(sets & SWORDS) + !!(sets & SHIELDS) + !!(sets & TUNICS) + !!(sets & BOOTS);
    return progress_at_least(current, target);
}

static bingo_progress_t trial_progress(trial_t trial) {
    return progress_bool(trial < TRIAL_COUNT && count_objective_is_complete(OBJECTIVE_TRIAL_LIGHT_END + trial));
}

uint16_t bingo_get_global_count(global_count_t count_index) {
    uint8_t count = get_global_count(count_index);
    return count == 0xFF ? UINT16_MAX : count;
}

uint16_t bingo_get_dungeon_count(uint8_t dungeon_index, dungeon_count_t count_index) {
    uint8_t count = get_dungeon_count(dungeon_index, count_index);
    return count == 0xFF ? UINT16_MAX : count;
}

uint16_t bingo_get_area_count(area_t area_index, area_count_t count_index) {
    uint8_t count = get_area_count(area_index, count_index);
    return count == 0xFF ? UINT16_MAX : count;
}

uint8_t bingo_get_current_area_index(void) {
    return get_current_area_index();
}

uint8_t bingo_get_inventory_item(uint8_t slot) {
    return slot < 24 ? z64_file.items[slot] : (uint8_t)Z64_ITEM_NULL;
}

uint8_t bingo_get_ammo(uint8_t slot) {
    return (slot < 15 && z64_file.ammo[slot] > 0) ? z64_file.ammo[slot] : 0;
}

uint8_t bingo_get_magic_level(void) {
    return z64_file.magic_capacity >= 2 ? 2 : (z64_file.magic_acquired ? 1 : 0);
}

uint8_t bingo_get_strength_level(void) {
    return z64_file.strength_upgrade;
}

uint8_t bingo_get_scale_level(void) {
    return z64_file.diving_upgrade;
}

uint8_t bingo_get_capacity_level(uint8_t item) {
    switch (item) {
        case Z64_ITEM_BOW:
            return z64_file.quiver;
        case Z64_ITEM_SLINGSHOT:
            return z64_file.bullet_bag;
        case Z64_ITEM_BOMB:
            return z64_file.bomb_bag;
        case Z64_ITEM_STICK:
            return z64_file.stick_upgrade;
        case Z64_ITEM_NUT:
            return z64_file.nut_upgrade;
        default:
            return 0;
    }
}

bool bingo_item_owned(uint8_t item) {
    switch (item) {
        case Z64_ITEM_BOW:
            return item_owned(Z64_SLOT_BOW, Z64_ITEM_BOW);
        case Z64_ITEM_SLINGSHOT:
            return item_owned(Z64_SLOT_SLINGSHOT, Z64_ITEM_SLINGSHOT);
        case Z64_ITEM_BOMB:
            return item_owned(Z64_SLOT_BOMB, Z64_ITEM_BOMB);
        case Z64_ITEM_KOKIRI_SWORD:
            return z64_file.kokiri_sword;
        case Z64_ITEM_MASTER_SWORD:
            return z64_file.master_sword;
        case Z64_ITEM_BIGGORON_SWORD:
            return z64_file.giants_knife || z64_file.broken_giants_knife;
        case Z64_ITEM_DEKU_SHIELD:
            return z64_file.deku_shield;
        case Z64_ITEM_HYLIAN_SHIELD:
            return z64_file.hylian_shield;
        case Z64_ITEM_MIRROR_SHIELD:
            return z64_file.mirror_shield;
        case Z64_ITEM_KOKIRI_TUNIC:
            return z64_file.kokiri_tunic;
        case Z64_ITEM_GORON_TUNIC:
            return z64_file.goron_tunic;
        case Z64_ITEM_ZORA_TUNIC:
            return z64_file.zora_tunic;
        case Z64_ITEM_KOKIRI_BOOTS:
            return z64_file.kokiri_boots;
        case Z64_ITEM_IRON_BOOTS:
            return z64_file.iron_boots;
        case Z64_ITEM_HOVER_BOOTS:
            return z64_file.hover_boots;
        case Z64_ITEM_BULLET_BAG_40:
            return z64_file.bullet_bag == 2;
        case Z64_ITEM_BULLET_BAG_50:
            return z64_file.bullet_bag == 3;
        case Z64_ITEM_QUIVER_40:
            return z64_file.quiver == 2;
        case Z64_ITEM_QUIVER_50:
            return z64_file.quiver == 3;
        case Z64_ITEM_BOMB_BAG_30:
            return z64_file.bomb_bag == 2;
        case Z64_ITEM_BOMB_BAG_40:
            return z64_file.bomb_bag == 3;
        case Z64_ITEM_MINUET:
            return z64_file.minuet_of_forest;
        case Z64_ITEM_BOLERO:
            return z64_file.bolero_of_fire;
        case Z64_ITEM_SERENADE:
            return z64_file.serenade_of_water;
        case Z64_ITEM_REQUIEM:
            return z64_file.requiem_of_spirit;
        case Z64_ITEM_NOCTURNE:
            return z64_file.nocturne_of_shadow;
        case Z64_ITEM_PRELUDE:
            return z64_file.prelude_of_light;
        case Z64_ITEM_ZELDAS_LULLABY:
            return z64_file.zeldas_lullaby;
        case Z64_ITEM_EPONAS_SONG:
            return z64_file.eponas_song;
        case Z64_ITEM_SARIAS_SONG:
            return z64_file.sarias_song;
        case Z64_ITEM_SUNS_SONG:
            return z64_file.suns_song;
        case Z64_ITEM_SONG_OF_TIME:
            return z64_file.song_of_time;
        case Z64_ITEM_SONG_OF_STORMS:
            return z64_file.song_of_storms;
        default:
            return false;
    }
}

bingo_progress_t bingo_get_goal_progress(bingo_goal_t goal) {
    switch (goal) {
        case BINGO_GOAL_BOTH_GERUDO_VALLEY_HPS:
            return progress_at_least(area_hps(AREA_GERUDO_VALLEY), 2);
        case BINGO_GOAL_KOKIRI_SWORD:
            return progress_bool(z64_file.kokiri_sword);
        case BINGO_GOAL_MAP_COMPASS_IN_DEKU_TREE:
            return progress_at_least(map_and_compass_count(SCENE_DEKU_TREE), 2);
        case BINGO_GOAL_SILVER_SCALE:
            return progress_exactly(z64_file.diving_upgrade, 1);
        case BINGO_GOAL_BOLERO_OF_FIRE:
            return progress_bool(z64_file.bolero_of_fire);
        case BINGO_GOAL_EXACTLY_20_DEKU_STICKS:
            return progress_exactly(z64_file.ammo[Z64_SLOT_STICK], 20);
        case BINGO_GOAL_MINUET_OF_FOREST:
            return progress_bool(z64_file.minuet_of_forest);
        case BINGO_GOAL_ZORA_TUNIC:
            return progress_bool(z64_file.zora_tunic);
        case BINGO_GOAL_N_2_SKULLTULAS_IN_LON_LON_RANCH:
            return progress_at_least(area_tokens(AREA_LON_LON_RANCH), 2);
        case BINGO_GOAL_N_3_LAKE_HYLIA_SKULLTULAS:
            return progress_at_least(area_tokens(AREA_LAKE_HYLIA), 3);
        case BINGO_GOAL_BLUE_FIRE:
            return progress_at_least(inventory_item_count(Z64_ITEM_BLUE_FIRE), 1);
        case BINGO_GOAL_DEFEAT_A_SKULL_KID:
            return progress_at_least(get_global_count(GCOUNT_SKULL_KID), 1);
        case BINGO_GOAL_GORON_TUNIC:
            return progress_bool(z64_file.goron_tunic);
        case BINGO_GOAL_PLANT_BEAN_IN_DEATH_MOUNTAIN_CRATER:
            return progress_bool(count_scene_switch_set(SCENE_DEATH_MOUNTAIN_CRATER, 0x03));
        case BINGO_GOAL_RUTO_S_LETTER:
            return progress_at_least(inventory_item_count(Z64_ITEM_LETTER), 1);
        case BINGO_GOAL_BOTTLED_POE:
            return progress_at_least(inventory_item_count(Z64_ITEM_POE), 1);
        case BINGO_GOAL_RED_POTION:
            return progress_at_least(inventory_item_count(Z64_ITEM_RED_POTION), 1);
        case BINGO_GOAL_ADULT_S_WALLET:
            return progress_exactly(z64_file.wallet, 1);
        case BINGO_GOAL_PLANT_BEAN_IN_GERUDO_VALLEY:
            return progress_bool(count_scene_switch_set(SCENE_GERUDO_VALLEY, 0x03));
        case BINGO_GOAL_BOTTLED_FAIRY:
            return progress_at_least(inventory_item_count(Z64_ITEM_FAIRY), 1);
        case BINGO_GOAL_DEFEAT_NABOORU_KNUCKLE:
            return progress_bool(count_objective_is_complete(OBJECTIVE_NABOORU_KNUCKLE));
        case BINGO_GOAL_FAIRY_SLINGSHOT:
            return progress_bool(item_owned(Z64_SLOT_SLINGSHOT, Z64_ITEM_SLINGSHOT));
        case BINGO_GOAL_LON_LON_RANCH_HP:
            return progress_at_least(area_hps(AREA_LON_LON_RANCH), 1);
        case BINGO_GOAL_MIRROR_SHIELD:
            return progress_bool(z64_file.mirror_shield);
        case BINGO_GOAL_REQUIEM_OF_SPIRIT:
            return progress_bool(z64_file.requiem_of_spirit);
        case BINGO_GOAL_FAIRY_OCARINA:
            return progress_bool(item_owned(Z64_SLOT_OCARINA, Z64_ITEM_FAIRY_OCARINA));
        case BINGO_GOAL_BLACK_GAUNTLETS:
            return progress_exactly(z64_file.strength_upgrade, 4);
        case BINGO_GOAL_N_3_SKULLS_IN_DEKU_TREE:
            return progress_at_least(dungeon_tokens(SCENE_DEKU_TREE), 3);
        case BINGO_GOAL_OPEN_MIDO_S_GREEN_RUPEE_CHEST:
            return progress_at_least(get_global_count(GCOUNT_MIDOS_GREEN_RUPEE), 1);
        case BINGO_GOAL_N_5_HEARTS:
            return progress_at_least(get_global_count(GCOUNT_HEART), 5);
        case BINGO_GOAL_N_5_MAGIC_BEANS:
            return progress_exactly(z64_file.ammo[Z64_SLOT_BEANS], 5);
        case BINGO_GOAL_DEFEAT_BONGO_BONGO:
            return progress_bool(dungeon_boss_cleared(SCENE_SHADOW_TEMPLE));
        case BINGO_GOAL_DEFEAT_QUEEN_GOHMA:
            return progress_bool(dungeon_boss_cleared(SCENE_DEKU_TREE));
        case BINGO_GOAL_GIANT_S_KNIFE:
            return progress_bool(z64_file.giants_knife || z64_file.broken_giants_knife);
        case BINGO_GOAL_N_3_MAPS:
            return progress_at_least(get_global_count(GCOUNT_MAP), 3);
        case BINGO_GOAL_N_3_TUNICS:
            return progress_at_least(get_global_count(GCOUNT_TUNIC), 3);
        case BINGO_GOAL_GANON_S_CASTLE_BOSS_KEY:
            return progress_bool(dungeon_item_owned(SCENE_INSIDE_GANONS_CASTLE, DUNGEON_ITEM_BOSS_KEY));
        case BINGO_GOAL_N_3_SONGS:
            return progress_at_least(get_global_count(GCOUNT_SONG), 3);
        case BINGO_GOAL_N_37TH_HEART_PIECE_CHILD_FORTRESS:
            return progress_bool(count_scene_collect_set(SCENE_GERUDOS_FORTRESS, 0x01));
        case BINGO_GOAL_BEAT_THE_DEKU_TREE:
            return progress_bool(count_tracker_is_dungeon_completed(SCENE_DEKU_TREE));
        case BINGO_GOAL_BEAT_THE_SHADOW_TEMPLE:
            return progress_bool(count_tracker_is_dungeon_completed(SCENE_SHADOW_TEMPLE));
        case BINGO_GOAL_BULLET_BAG_40:
            return progress_exactly(z64_file.bullet_bag, 2);
        case BINGO_GOAL_GOLDEN_SCALE:
            return progress_exactly(z64_file.diving_upgrade, 2);
        case BINGO_GOAL_N_3_COMPASSES:
            return progress_at_least(get_global_count(GCOUNT_COMPASS), 3);
        case BINGO_GOAL_BEAT_2_DUNGEONS:
            return progress_at_least(dungeon_completed_count(), 2);
        case BINGO_GOAL_N_3_SKULLTULAS_IN_DODONGO_S_CAVERN:
            return progress_at_least(dungeon_tokens(SCENE_DODONGOS_CAVERN), 3);
        case BINGO_GOAL_N_4_SONGS:
            return progress_at_least(get_global_count(GCOUNT_SONG), 4);
        case BINGO_GOAL_N_30_DEKU_NUTS:
            return progress_exactly(z64_file.ammo[Z64_SLOT_NUT], 30);
        case BINGO_GOAL_ALL_3_KOKIRI_FOREST_AREA_SKULLTULAS:
            return progress_at_least(area_tokens(AREA_KOKIRI_FOREST), 3);
        case BINGO_GOAL_ICE_ARROWS:
            return progress_bool(item_owned(Z64_SLOT_ICE_ARROW, Z64_ITEM_ICE_ARROW));
        case BINGO_GOAL_MAP_COMPASS_IN_BOTTOM_OF_THE_WELL:
            return progress_at_least(map_and_compass_count(SCENE_BOTTOM_OF_THE_WELL), 2);
        case BINGO_GOAL_MAP_COMPASS_IN_DODONGO_S_CAVERN:
            return progress_at_least(map_and_compass_count(SCENE_DODONGOS_CAVERN), 2);
        case BINGO_GOAL_MAP_COMPASS_IN_WATER_TEMPLE:
            return progress_at_least(map_and_compass_count(SCENE_WATER_TEMPLE), 2);
        case BINGO_GOAL_PLANT_3_MAGIC_BEANS:
            return progress_at_least(get_global_count(GCOUNT_BEANS_PLANTED), 3);
        case BINGO_GOAL_OBTAIN_ONE_HYRULE_FIELD_HP:
            return progress_at_least(area_hps(AREA_HYRULE_FIELD), 1);
        case BINGO_GOAL_N_4_UNUSED_KEYS_IN_GERUDO_TRAINING_GROUNDS:
            return progress_at_least(current_keys(SCENE_GERUDO_TRAINING_GROUND), 4);
        case BINGO_GOAL_N_6_HEARTS:
            return progress_at_least(get_global_count(GCOUNT_HEART), 6);
        case BINGO_GOAL_BOOMERANG:
            return progress_bool(item_owned(Z64_SLOT_BOOMERANG, Z64_ITEM_BOOMERANG));
        case BINGO_GOAL_DEFEAT_A_WHITE_WOLFOS:
            return progress_at_least(get_global_count(GCOUNT_WHITE_WOLFOS), 1);
        case BINGO_GOAL_DEFEAT_ALL_LIZALFOS_IN_DODONGO_S_CAVERN:
            return progress_bool(count_objective_is_complete(OBJECTIVE_DODONGO_LIZALFOS));
        case BINGO_GOAL_GET_BOMBCHU_CHEST_IN_SPIRIT_TEMPLE:
            return progress_bool(count_objective_is_complete(OBJECTIVE_SPIRIT_BOMBCHU_CHEST));
        case BINGO_GOAL_MAP_COMPASS_IN_SHADOW_TEMPLE:
            return progress_at_least(map_and_compass_count(SCENE_SHADOW_TEMPLE), 2);
        case BINGO_GOAL_CLEAR_4_SILVER_RUPEE_ROOMS:
            return progress_at_least(get_global_count(GCOUNT_SILVER_RUPEE_ROOMS), 4);
        case BINGO_GOAL_DESERT_COLOSSUS_HP:
            return progress_at_least(area_hps(AREA_WASTELAND), 1);
        case BINGO_GOAL_N_7_KAKARIKO_AREA_SKULLTULAS:
            return progress_at_least(area_tokens(AREA_KAKARIKO), 7);
        case BINGO_GOAL_DEFEAT_5_GIBDOS:
            return progress_at_least(get_global_count(GCOUNT_GIBDO), 5);
        case BINGO_GOAL_N_15_DIFFERENT_SKULLTULAS:
            return progress_at_least(z64_file.gs_tokens, 15);
        case BINGO_GOAL_N_3_SHIELDS:
            return progress_at_least(get_global_count(GCOUNT_SHIELD), 3);
        case BINGO_GOAL_BOMB_BAG_30:
            return progress_exactly(z64_file.bomb_bag, 2);
        case BINGO_GOAL_BULLET_BAG_50:
            return progress_exactly(z64_file.bullet_bag, 3);
        case BINGO_GOAL_DEFEAT_BIG_OCTO:
            return progress_bool(count_objective_is_complete(OBJECTIVE_BIG_OCTO));
        case BINGO_GOAL_DEFEAT_KING_DODONGO:
            return progress_bool(dungeon_boss_cleared(SCENE_DODONGOS_CAVERN));
        case BINGO_GOAL_ICE_CAVERN_HP:
            return progress_at_least(bingo_get_dungeon_count(SCENE_ICE_CAVERN, DCOUNT_HEART_PIECE), 1);
        case BINGO_GOAL_MAP_COMPASS_IN_ICE_CAVERN:
            return progress_at_least(map_and_compass_count(SCENE_ICE_CAVERN), 2);
        case BINGO_GOAL_MAP_COMPASS_IN_JABU_JABU:
            return progress_at_least(map_and_compass_count(SCENE_JABU_JABU), 2);
        case BINGO_GOAL_SILVER_GAUNTLETS:
            return progress_exactly(z64_file.strength_upgrade, 2);
        case BINGO_GOAL_GORON_CITY_HP:
            return progress_bool(count_scene_collect_set(SCENE_GORON_CITY, 0x1F));
        case BINGO_GOAL_KILL_ALL_TENTACLES_IN_JABU_JABU:
            return progress_at_least(get_global_count(GCOUNT_TENTACLE), 3);
        case BINGO_GOAL_N_5_SONGS:
            return progress_at_least(get_global_count(GCOUNT_SONG), 5);
        case BINGO_GOAL_DIN_S_FIRE:
            return progress_bool(item_owned(Z64_SLOT_DINS_FIRE, Z64_ITEM_DINS_FIRE));
        case BINGO_GOAL_FARORE_S_WIND:
            return progress_bool(item_owned(Z64_SLOT_FARORES_WIND, Z64_ITEM_FARORES_WIND));
        case BINGO_GOAL_KEATON_MASK:
            return progress_bool(item_owned(Z64_SLOT_CHILD_TRADE, Z64_ITEM_KEATON_MASK));
        case BINGO_GOAL_N_2_BOSS_KEYS:
            return progress_at_least(get_global_count(GCOUNT_BOSS_KEY), 2);
        case BINGO_GOAL_N_4_MAPS:
            return progress_at_least(get_global_count(GCOUNT_MAP), 4);
        case BINGO_GOAL_ALL_4_LOST_WOODS_AREA_SKULLTULAS:
            return progress_at_least(area_tokens(AREA_LOST_WOODS), 4);
        case BINGO_GOAL_BEAT_DODONGO_S_CAVERN:
            return progress_bool(count_tracker_is_dungeon_completed(SCENE_DODONGOS_CAVERN));
        case BINGO_GOAL_BOTH_HPS_IN_DEATH_MOUNTAIN_CRATER:
            return progress_at_least(count_scene_collect_set(SCENE_DEATH_MOUNTAIN_CRATER, 0x08) +
                                         count_scene_collect_set(SCENE_DEATH_MOUNTAIN_CRATER, 0x02),
                                     2);
        case BINGO_GOAL_DEFEAT_MORPHA:
            return progress_bool(dungeon_boss_cleared(SCENE_WATER_TEMPLE));
        case BINGO_GOAL_IRON_BOOTS:
            return progress_bool(z64_file.iron_boots);
        case BINGO_GOAL_LENS_OF_TRUTH:
            return progress_bool(item_owned(Z64_SLOT_LENS, Z64_ITEM_LENS));
        case BINGO_GOAL_COJIRO:
            return progress_bool(item_owned(Z64_SLOT_ADULT_TRADE, Z64_ITEM_COJIRO));
        case BINGO_GOAL_N_3_SWORDS_3_TUNICS:
            return equipment_set_progress(SWORDS | TUNICS);
        case BINGO_GOAL_NAYRU_S_LOVE:
            return progress_bool(item_owned(Z64_SLOT_NAYRUS_LOVE, Z64_ITEM_NAYRUS_LOVE));
        case BINGO_GOAL_BEAT_3_DUNGEONS:
            return progress_at_least(dungeon_completed_count(), 3);
        case BINGO_GOAL_PLANT_4_MAGIC_BEANS:
            return progress_at_least(get_global_count(GCOUNT_BEANS_PLANTED), 4);
        case BINGO_GOAL_OPEN_UPPER_GERUDO_FORTRESS_CHEST:
            return progress_bool(count_scene_chest_opened(SCENE_GERUDOS_FORTRESS, 0x00));
        case BINGO_GOAL_N_4_COMPASSES:
            return progress_at_least(get_global_count(GCOUNT_COMPASS), 4);
        case BINGO_GOAL_ALL_4_SKULLTULAS_IN_DEKU_TREE:
            return progress_at_least(dungeon_tokens(SCENE_DEKU_TREE), 4);
        case BINGO_GOAL_ALL_8_KAKARIKO_AREA_SKULLTULAS:
            return progress_at_least(area_tokens(AREA_KAKARIKO), 8);
        case BINGO_GOAL_BEAT_THE_WATER_TEMPLE:
            return progress_bool(count_tracker_is_dungeon_completed(SCENE_WATER_TEMPLE));
        case BINGO_GOAL_BOTH_GERUDO_S_FORTRESS_AREA_SKULLTULAS:
            return progress_at_least(area_tokens(AREA_FORTRESS), 2);
        case BINGO_GOAL_FILL_ALL_4_BOTTLE_SLOTS:
            return progress_at_least(bottle_slot_count(), 4);
        case BINGO_GOAL_SARIA_S_SONG:
            return progress_bool(z64_file.sarias_song);
        case BINGO_GOAL_SERENADE_OF_WATER:
            return progress_bool(z64_file.serenade_of_water);
        case BINGO_GOAL_N_4_LAKE_HYLIA_SKULLTULAS:
            return progress_at_least(area_tokens(AREA_LAKE_HYLIA), 4);
        case BINGO_GOAL_ALL_4_SKULLTULAS_IN_JABU_JABU:
            return progress_at_least(dungeon_tokens(SCENE_JABU_JABU), 4);
        case BINGO_GOAL_DOUBLE_MAGIC:
            return progress_at_least(z64_file.magic_capacity, 2);
        case BINGO_GOAL_GET_TO_THE_END_OF_LIGHT_TRIAL:
            return trial_progress(TRIAL_LIGHT);
        case BINGO_GOAL_N_4_DEATH_MOUNTAIN_AREA_SKULLTULAS:
            return progress_at_least(area_tokens(AREA_DEATH_MOUNTAIN), 4);
        case BINGO_GOAL_DEFEAT_6_DIFFERENT_STALFOS:
            return progress_at_least(get_global_count(GCOUNT_STALFOS), 6);
        case BINGO_GOAL_N_6_SONGS:
            return progress_at_least(get_global_count(GCOUNT_SONG), 6);
        case BINGO_GOAL_BLUE_GAUNTLETS:
            return progress_exactly(z64_file.strength_upgrade, 6);
        case BINGO_GOAL_DEFEAT_PHANTOM_GANON:
            return progress_bool(dungeon_boss_cleared(SCENE_FOREST_TEMPLE));
        case BINGO_GOAL_DOUBLE_DEFENSE:
            return progress_bool(z64_file.double_defense);
        case BINGO_GOAL_FIRE_TEMPLE_BOSS_KEY:
            return progress_bool(dungeon_item_owned(SCENE_FIRE_TEMPLE, DUNGEON_ITEM_BOSS_KEY));
        case BINGO_GOAL_SHADOW_TEMPLE_BOSS_KEY:
            return progress_bool(dungeon_item_owned(SCENE_SHADOW_TEMPLE, DUNGEON_ITEM_BOSS_KEY));
        case BINGO_GOAL_SPIRIT_TEMPLE_BOSS_KEY:
            return progress_bool(dungeon_item_owned(SCENE_SPIRIT_TEMPLE, DUNGEON_ITEM_BOSS_KEY));
        case BINGO_GOAL_SKULL_MASK:
            return progress_bool(item_owned(Z64_SLOT_CHILD_TRADE, Z64_ITEM_SKULL_MASK));
        case BINGO_GOAL_LOWER_FINAL_PLATFORM_IN_SPIRIT_TEMPLE:
            return progress_bool(count_objective_is_complete(OBJECTIVE_SPIRIT_PLATFORM));
        case BINGO_GOAL_FREE_3_CARPENTERS:
            return progress_at_least(get_global_count(GCOUNT_CARPENTER), 3);
        case BINGO_GOAL_DISPEL_3_TRIAL_BEAMS:
            return progress_at_least(get_global_count(GCOUNT_TRIAL_BEAMS), 3);
        case BINGO_GOAL_N_1_SKULLTULA_FROM_EACH_CHILD_DUNGEON:
            return progress_at_least(child_dungeons_with_tokens(), 3);
        case BINGO_GOAL_N_3_BOOTS:
            return progress_at_least(get_global_count(GCOUNT_BOOT), 3);
        case BINGO_GOAL_N_5_MAPS:
            return progress_at_least(get_global_count(GCOUNT_MAP), 5);
        case BINGO_GOAL_N_7_HEARTS:
            return progress_at_least(get_global_count(GCOUNT_HEART), 7);
        case BINGO_GOAL_ALL_3_SKULLTULAS_IN_ICE_CAVERN:
            return progress_at_least(dungeon_tokens(SCENE_ICE_CAVERN), 3);
        case BINGO_GOAL_BEAT_THE_FOREST_TEMPLE:
            return progress_bool(count_tracker_is_dungeon_completed(SCENE_FOREST_TEMPLE));
        case BINGO_GOAL_EPONA_S_SONG:
            return progress_bool(z64_file.eponas_song);
        case BINGO_GOAL_WATER_TEMPLE_BOSS_KEY:
            return progress_bool(dungeon_item_owned(SCENE_WATER_TEMPLE, DUNGEON_ITEM_BOSS_KEY));
        case BINGO_GOAL_ALL_4_CHILD_SKULLTULAS_IN_ZORA_S_DOMAIN_AREA:
            return progress_at_least(get_tokens_with_mask(GS_FLAGS_ZORA, 0x0F), 4);
        case BINGO_GOAL_GOLDEN_RUPEE_CHEST_IN_FIRE_TEMPLE:
            return progress_bool(count_objective_is_complete(OBJECTIVE_FIRE_GOLD_RUPEE));
        case BINGO_GOAL_TWO_FAIRY_SPELLS:
            return progress_at_least(get_global_count(GCOUNT_FAIRY_SPELL), 2);
        case BINGO_GOAL_N_3_SHIELDS_3_TUNICS:
            return equipment_set_progress(SHIELDS | TUNICS);
        case BINGO_GOAL_N_6_UNUSED_KEYS_IN_GERUDO_TRAINING_GROUNDS:
            return progress_at_least(current_keys(SCENE_GERUDO_TRAINING_GROUND), 6);
        case BINGO_GOAL_ALL_4_LON_LON_RANCH_AREA_SKULLTULAS:
            return progress_at_least(area_tokens(AREA_LON_LON_RANCH), 4);
        case BINGO_GOAL_DEFEAT_AMY_GREEN_POE:
            return progress_bool(count_objective_is_complete(OBJECTIVE_POE_AMY));
        case BINGO_GOAL_DEFEAT_TWINROVA:
            return progress_bool(dungeon_boss_cleared(SCENE_SPIRIT_TEMPLE));
        case BINGO_GOAL_GIANT_S_WALLET:
            return progress_exactly(z64_file.wallet, 2);
        case BINGO_GOAL_MAP_COMPASS_IN_FIRE_TEMPLE:
            return progress_at_least(map_and_compass_count(SCENE_FIRE_TEMPLE), 2);
        case BINGO_GOAL_WIN_BOMBCHU_BOWLING_PRIZE:
            return progress_bool(count_objective_is_complete(OBJECTIVE_BOMBCHU_BOWLING));
        case BINGO_GOAL_DEFEAT_BOTH_DEAD_HANDS:
            return progress_at_least(get_global_count(GCOUNT_DEAD_HANDS), 2);
        case BINGO_GOAL_EXACTLY_30_DEKU_STICKS:
            return progress_exactly(z64_file.ammo[Z64_SLOT_STICK], 30);
        case BINGO_GOAL_N_4_UNUSED_KEYS_IN_FOREST_TEMPLE:
            return progress_at_least(current_keys(SCENE_FOREST_TEMPLE), 4);
        case BINGO_GOAL_N_6_MAGIC_BEANS:
            return progress_exactly(z64_file.ammo[Z64_SLOT_BEANS], 6);
        case BINGO_GOAL_N_6_DEATH_MOUNTAIN_AREA_SKULLTULAS:
            return progress_at_least(area_tokens(AREA_DEATH_MOUNTAIN), 6);
        case BINGO_GOAL_N_3_BOSS_KEYS:
            return progress_at_least(get_global_count(GCOUNT_BOSS_KEY), 3);
        case BINGO_GOAL_N_20_DIFFERENT_SKULLTULAS:
            return progress_at_least(z64_file.gs_tokens, 20);
        case BINGO_GOAL_ALL_3_SKULLTULAS_IN_BOTTOM_OF_THE_WELL:
            return progress_at_least(dungeon_tokens(SCENE_BOTTOM_OF_THE_WELL), 3);
        case BINGO_GOAL_ALL_4_GERUDO_VALLEY_AREA_SKULLTULAS:
            return progress_at_least(area_tokens(AREA_GERUDO_VALLEY), 4);
        case BINGO_GOAL_FAIRY_BOW:
            return progress_bool(item_owned(Z64_SLOT_BOW, Z64_ITEM_BOW));
        case BINGO_GOAL_FOREST_TEMPLE_BOSS_KEY:
            return progress_bool(dungeon_item_owned(SCENE_FOREST_TEMPLE, DUNGEON_ITEM_BOSS_KEY));
        case BINGO_GOAL_GERUDO_S_CARD:
            return progress_bool(z64_file.gerudos_card);
        case BINGO_GOAL_MILK:
            return progress_at_least(inventory_item_count(Z64_ITEM_MILK), 1);
        case BINGO_GOAL_GET_TO_THE_END_OF_FIRE_TRIAL:
            return trial_progress(TRIAL_FIRE);
        case BINGO_GOAL_MAP_COMPASS_IN_FOREST_TEMPLE:
            return progress_at_least(map_and_compass_count(SCENE_FOREST_TEMPLE), 2);
        case BINGO_GOAL_BEAT_THE_SKY_TEMPLE:
            return progress_bool(count_objective_is_complete(OBJECTIVE_SKY_TEMPLE));
        case BINGO_GOAL_DEFEAT_3_DIFFERENT_WHITE_WOLFOS:
            return progress_at_least(get_global_count(GCOUNT_WHITE_WOLFOS), 3);
        case BINGO_GOAL_DEFEAT_JOELLE_RED_POE_BETH_BLUE_POE:
            return progress_at_least(count_objective_is_complete(OBJECTIVE_POE_JOELLE) +
                                         count_objective_is_complete(OBJECTIVE_POE_BETH),
                                     2);
        case BINGO_GOAL_OPEN_GANON_S_BOSS_DOOR:
            return progress_bool(count_scene_switch_set(SCENE_GANONS_TOWER, BOSS_DOOR_SWITCH_FLAG));
        case BINGO_GOAL_N_3_SWORDS_3_SHIELDS:
            return equipment_set_progress(SWORDS | SHIELDS);
        case BINGO_GOAL_N_5_COMPASSES:
            return progress_at_least(get_global_count(GCOUNT_COMPASS), 5);
        case BINGO_GOAL_ALL_4_WASTELAND_COLOSSUS_AREA_SKULLTULAS:
            return progress_at_least(area_tokens(AREA_WASTELAND), 4);
        case BINGO_GOAL_ALL_5_SKULLTULAS_IN_DODONGO_S_CAVERN:
            return progress_at_least(dungeon_tokens(SCENE_DODONGOS_CAVERN), 5);
        case BINGO_GOAL_BEAT_THE_SPIRIT_TEMPLE:
            return progress_bool(count_tracker_is_dungeon_completed(SCENE_SPIRIT_TEMPLE));
        case BINGO_GOAL_GET_TO_THE_END_OF_SPIRIT_TRIAL:
            return trial_progress(TRIAL_SPIRIT);
        case BINGO_GOAL_OPEN_3_GOLD_RUPEE_CHESTS:
            return progress_at_least(get_global_count(GCOUNT_GOLD_RUPEE), 3);
        case BINGO_GOAL_BOTTLED_BIG_POE:
            return progress_at_least(inventory_item_count(Z64_ITEM_BIG_POE), 1);
        case BINGO_GOAL_PRELUDE_OF_LIGHT:
            return progress_bool(z64_file.prelude_of_light);
        case BINGO_GOAL_ALL_4_ADULT_SKULLTULAS_IN_ZORA_S_DOMAIN_AREA:
            return progress_at_least(get_tokens_with_mask(GS_FLAGS_ZORA, 0xF0), 4);
        case BINGO_GOAL_DEFEAT_4_DIFFERENT_IRON_KNUCKLES:
            return progress_at_least(get_global_count(GCOUNT_IRON_KNUCKLE), 4);
        case BINGO_GOAL_N_6_ZORA_S_DOMAIN_AREA_SKULLTULAS:
            return progress_at_least(area_tokens(AREA_ZORA), 6);
        case BINGO_GOAL_OPEN_ALL_6_ICE_TRAP_CHESTS:
            return progress_at_least(get_global_count(GCOUNT_TRAP), 6);
        case BINGO_GOAL_N_3_TUNICS_3_BOOTS:
            return equipment_set_progress(TUNICS | BOOTS);
        case BINGO_GOAL_N_500_RUPEES:
            return progress_exactly(z64_file.rupees, 500);
        case BINGO_GOAL_N_7_MAGIC_BEANS:
            return progress_exactly(z64_file.ammo[Z64_SLOT_BEANS], 7);
        case BINGO_GOAL_N_7_SONGS:
            return progress_at_least(get_global_count(GCOUNT_SONG), 7);
        case BINGO_GOAL_COW_IN_HOUSE:
            return progress_bool(count_objective_is_complete(OBJECTIVE_COW_0));
        case BINGO_GOAL_DEFEAT_BARINADE:
            return progress_bool(dungeon_boss_cleared(SCENE_JABU_JABU));
        case BINGO_GOAL_FIRE_ARROWS:
            return progress_bool(item_owned(Z64_SLOT_FIRE_ARROW, Z64_ITEM_FIRE_ARROW));
        case BINGO_GOAL_OBTAIN_ALL_5_SMALL_KEYS_IN_FOREST_TEMPLE:
            return progress_at_least(total_keys_obtained(SCENE_FOREST_TEMPLE), 5);
        case BINGO_GOAL_QUIVER_40:
            return progress_exactly(z64_file.quiver, 2);
        case BINGO_GOAL_N_5_CHILD_DEATH_MOUNTAIN_AREA_SKULLTULAS:
            return progress_at_least(get_tokens_with_mask(GS_FLAGS_DEATH_MOUNTAIN, 0x1F), 5);
        case BINGO_GOAL_N_3_SWORDS_3_BOOTS:
            return equipment_set_progress(SWORDS | BOOTS);
        case BINGO_GOAL_N_4_SKULLTULAS_IN_SHADOW_TEMPLE:
            return progress_at_least(dungeon_tokens(SCENE_SHADOW_TEMPLE), 4);
        case BINGO_GOAL_N_6_MAPS:
            return progress_at_least(get_global_count(GCOUNT_MAP), 6);
        case BINGO_GOAL_N_8_HEARTS:
            return progress_at_least(get_global_count(GCOUNT_HEART), 8);
        case BINGO_GOAL_ALL_5_SKULLTULAS_IN_FOREST_TEMPLE:
            return progress_at_least(dungeon_tokens(SCENE_FOREST_TEMPLE), 5);
        case BINGO_GOAL_BEAT_JABU_JABU_S_BELLY:
            return progress_bool(count_tracker_is_dungeon_completed(SCENE_JABU_JABU));
        case BINGO_GOAL_STONE_OF_AGONY:
            return progress_bool(z64_file.stone_of_agony);
        case BINGO_GOAL_UNLOCK_PHANTOM_GANON_S_BOSS_DOOR:
            return progress_bool(count_scene_switch_set(SCENE_FOREST_TEMPLE, BOSS_DOOR_SWITCH_FLAG));
        case BINGO_GOAL_BEAT_4_DUNGEONS:
            return progress_at_least(dungeon_completed_count(), 4);
        case BINGO_GOAL_N_3_SHIELDS_3_BOOTS:
            return equipment_set_progress(SHIELDS | BOOTS);
        case BINGO_GOAL_ALL_5_SKULLTULAS_IN_FIRE_TEMPLE:
            return progress_at_least(dungeon_tokens(SCENE_FIRE_TEMPLE), 5);
        case BINGO_GOAL_DEFEAT_DARK_LINK:
            return progress_bool(count_objective_is_complete(OBJECTIVE_DARK_LINK));
        case BINGO_GOAL_GET_TO_THE_END_OF_FOREST_TRIAL:
            return trial_progress(TRIAL_FOREST);
        case BINGO_GOAL_GORON_BRACELET:
            return progress_exactly(z64_file.strength_upgrade, 1);
        case BINGO_GOAL_LONGSHOT:
            return progress_bool(item_owned(Z64_SLOT_HOOKSHOT, Z64_ITEM_LONGSHOT));
        case BINGO_GOAL_OPEN_50_RUPEE_CHEST_IN_WASTELAND:
            return progress_at_least(count_scene_chest_opened(SCENE_HAUNTED_WASTELAND, 0x00), 1);
        case BINGO_GOAL_ALL_8_DEATH_MOUNTAIN_AREA_SKULLTULAS:
            return progress_at_least(area_tokens(AREA_DEATH_MOUNTAIN), 8);
        case BINGO_GOAL_DEFEAT_10_DIFFERENT_STALFOS:
            return progress_at_least(get_global_count(GCOUNT_STALFOS), 10);
        case BINGO_GOAL_N_6_COMPASSES:
            return progress_at_least(get_global_count(GCOUNT_COMPASS), 6);
        case BINGO_GOAL_N_8_SONGS:
            return progress_at_least(get_global_count(GCOUNT_SONG), 8);
        case BINGO_GOAL_BOTH_HPS_IN_LOST_WOODS:
            return progress_at_least(area_hps(AREA_LOST_WOODS), 2);
        case BINGO_GOAL_DEFEAT_BOTH_FLARE_DANCERS:
            return progress_bool(count_objective_is_complete(OBJECTIVE_FIRE_FLARE_DANCERS));
        case BINGO_GOAL_MAP_COMPASS_IN_SPIRIT_TEMPLE:
            return progress_at_least(map_and_compass_count(SCENE_SPIRIT_TEMPLE), 2);
        case BINGO_GOAL_MEGATON_HAMMER:
            return progress_bool(item_owned(Z64_SLOT_HAMMER, Z64_ITEM_HAMMER));
        case BINGO_GOAL_OBTAIN_ALL_8_SMALL_KEYS_IN_FIRE_TEMPLE:
            return progress_at_least(total_keys_obtained(SCENE_FIRE_TEMPLE), 8);
        case BINGO_GOAL_OPEN_4_SMALL_KEY_CHESTS_IN_SPIRIT_TEMPLE:
            return progress_at_least(total_keys_obtained(SCENE_SPIRIT_TEMPLE), 4);
        case BINGO_GOAL_BOTH_HYRULE_FIELD_HPS:
            return progress_at_least(area_hps(AREA_HYRULE_FIELD), 2);
        case BINGO_GOAL_MILK_4_COWS:
            return progress_at_least(get_global_count(GCOUNT_COW), 4);
        case BINGO_GOAL_N_3_SKULLTULAS_IN_WATER_TEMPLE:
            return progress_at_least(dungeon_tokens(SCENE_WATER_TEMPLE), 3);
        case BINGO_GOAL_N_5_ZORA_AREA_HPS:
            return progress_at_least(area_hps(AREA_ZORA), 5);
        case BINGO_GOAL_ALL_8_ZORA_S_DOMAIN_AREA_SKULLTULAS:
            return progress_at_least(area_tokens(AREA_ZORA), 8);
        case BINGO_GOAL_FREE_ALL_9_GORONS_IN_FIRE_TEMPLE:
            return progress_at_least(get_global_count(GCOUNT_FIRE_GORON), get_fire_goron_total());
        case BINGO_GOAL_FROG_S_HP:
            return progress_bool(count_event_check_set(0xD0));
        case BINGO_GOAL_CLEAR_8_SILVER_RUPEE_ROOMS:
            return progress_at_least(get_global_count(GCOUNT_SILVER_RUPEE_ROOMS), 8);
        case BINGO_GOAL_N_4_BOSS_KEYS:
            return progress_at_least(get_global_count(GCOUNT_BOSS_KEY), 4);
        case BINGO_GOAL_GOLDEN_GAUNTLETS:
            return progress_exactly(z64_file.strength_upgrade, 3);
        case BINGO_GOAL_OPEN_3_SMALL_KEY_CHESTS_IN_WATER_TEMPLE:
            return progress_at_least(total_keys_obtained(SCENE_WATER_TEMPLE), 3);
        case BINGO_GOAL_N_3_SWORDS_TUNICS_BOOTS_AND_SHIELDS:
            return equipment_set_progress(SWORDS | SHIELDS | TUNICS | BOOTS);
        case BINGO_GOAL_N_7_MAPS:
            return progress_at_least(get_global_count(GCOUNT_MAP), 7);
        case BINGO_GOAL_N_9_HEARTS:
            return progress_at_least(get_global_count(GCOUNT_HEART), 9);
        case BINGO_GOAL_GREEN_GAUNTLETS:
            return progress_exactly(z64_file.strength_upgrade, 5);
        case BINGO_GOAL_ALL_5_SKULLTULAS_IN_SPIRIT_TEMPLE:
            return progress_at_least(dungeon_tokens(SCENE_SPIRIT_TEMPLE), 5);
        case BINGO_GOAL_GET_TO_THE_END_OF_WATER_TRIAL:
            return trial_progress(TRIAL_WATER);
        case BINGO_GOAL_DEFEAT_5_DIFFERENT_IRON_KNUCKLES:
            return progress_at_least(get_global_count(GCOUNT_IRON_KNUCKLE), 5);
        case BINGO_GOAL_DISPEL_5_TRIAL_BEAMS:
            return progress_at_least(get_global_count(GCOUNT_TRIAL_BEAMS), 5);
        case BINGO_GOAL_N_1_SKULLTULA_FROM_EACH_ADULT_DUNGEON:
            return progress_at_least(adult_dungeons_with_tokens(), 5);
        case BINGO_GOAL_N_8_DIFFERENT_UNUSED_KEYS_IN_GERUDO_TRAINING_GROUNDS:
            return progress_at_least(current_keys(SCENE_GERUDO_TRAINING_GROUND), 8);
        case BINGO_GOAL_DEFEAT_MEG_PURPLE_POE:
            return progress_bool(count_objective_is_complete(OBJECTIVE_POE_MEG));
        case BINGO_GOAL_OPEN_ALL_5_SMALL_KEY_CHESTS_IN_SPIRIT_TEMPLE:
            return progress_at_least(total_keys_obtained(SCENE_SPIRIT_TEMPLE), 5);
        case BINGO_GOAL_LIGHT_ARROWS:
            return progress_bool(item_owned(Z64_SLOT_LIGHT_ARROW, Z64_ITEM_LIGHT_ARROW));
        case BINGO_GOAL_BEAT_5_DUNGEONS:
            return progress_at_least(dungeon_completed_count(), 5);
        case BINGO_GOAL_OBTAIN_ALL_3_SPIRITUAL_STONES:
            return progress_at_least(get_global_count(GCOUNT_STONE), 3);
        case BINGO_GOAL_N_1_UNUSED_SMALL_KEY_IN_EACH_ADULT_DUNGEON:
            return progress_at_least(adult_dungeons_with_current_keys(), 5);
        case BINGO_GOAL_N_7_COMPASSES:
            return progress_at_least(get_global_count(GCOUNT_COMPASS), 7);
        case BINGO_GOAL_N_9_SONGS:
            return progress_at_least(get_global_count(GCOUNT_SONG), 9);
        case BINGO_GOAL_ALL_5_LAKE_HYLIA_SKULLTULAS:
            return progress_at_least(area_tokens(AREA_LAKE_HYLIA), 5);
        case BINGO_GOAL_DEFEAT_VOLVAGIA:
            return progress_bool(dungeon_boss_cleared(SCENE_FIRE_TEMPLE));
        case BINGO_GOAL_N_25_DIFFERENT_SKULLTULAS:
            return progress_at_least(z64_file.gs_tokens, 25);
        case BINGO_GOAL_ALL_3_ELEMENTAL_ARROWS:
            return progress_at_least(get_global_count(GCOUNT_ELEMENTAL_ARROW), 3);
        case BINGO_GOAL_ALL_5_SKULLTULAS_IN_SHADOW_TEMPLE:
            return progress_at_least(dungeon_tokens(SCENE_SHADOW_TEMPLE), 5);
        case BINGO_GOAL_BEAT_THE_FIRE_TEMPLE:
            return progress_bool(count_tracker_is_dungeon_completed(SCENE_FIRE_TEMPLE));
        case BINGO_GOAL_BOTH_HYRULE_FIELD_AREA_SKULLTULAS:
            return progress_at_least(area_tokens(AREA_HYRULE_FIELD), 2);
        case BINGO_GOAL_OBTAIN_ALL_5_SMALL_KEYS_IN_SHADOW_TEMPLE:
            return progress_at_least(total_keys_obtained(SCENE_SHADOW_TEMPLE), 5);
        case BINGO_GOAL_ALL_9_SOFT_SOIL_SKULLTULAS:
            return progress_at_least(get_global_count(GCOUNT_SOIL_SKULLTULA), 9);
        case BINGO_GOAL_N_10_SONGS:
            return progress_at_least(get_global_count(GCOUNT_SONG), 10);
        case BINGO_GOAL_ALL_4_MARKET_AREA_SKULLTULAS:
            return progress_at_least(area_tokens(AREA_MARKET), 4);
        case BINGO_GOAL_BLUE_POTION:
            return progress_at_least(inventory_item_count(Z64_ITEM_BLUE_POTION), 1);
        case BINGO_GOAL_GET_TO_THE_END_OF_SHADOW_TRIAL:
            return trial_progress(TRIAL_SHADOW);
        case BINGO_GOAL_BOTH_RUSTY_SWITCHES_IN_SPIRIT_TEMPLE:
            return progress_at_least(count_tracker_get_spirit_rusty_switch_count(), z64_file.master_quest ? 4 : 2);
        case BINGO_GOAL_ALL_5_SKULLTULAS_IN_WATER_TEMPLE:
            return progress_at_least(dungeon_tokens(SCENE_WATER_TEMPLE), 5);
        case BINGO_GOAL_N_30_DIFFERENT_SKULLTULAS:
            return progress_at_least(z64_file.gs_tokens, 30);
        case BINGO_GOAL_N_6_ZORA_AREA_HPS:
            return progress_at_least(area_hps(AREA_ZORA), 6);
        case BINGO_GOAL_DEFEAT_GANONDORF:
            return progress_bool(dungeon_boss_cleared(SCENE_INSIDE_GANONS_CASTLE));
        case BINGO_GOAL_COUNT:
        default: {
            bingo_progress_t invalid = { 0, 0, false };
            return invalid;
        }
    }
}

bool bingo_goal_is_complete(bingo_goal_t goal) {
    return bingo_get_goal_progress(goal).complete;
}
