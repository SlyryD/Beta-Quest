#ifndef BINGO_H
#define BINGO_H

#include <stdbool.h>
#include <stdint.h>

#include "counts.h"

// Generated from ocarina_of_time_beta_quest_generator.js
// Source commit: d8068e992682834bb57713299d32c4d12ca5ff7b
typedef enum {
    BINGO_GOAL_BOTH_GERUDO_VALLEY_HPS = 0, // Both Gerudo Valley HPs
    BINGO_GOAL_KOKIRI_SWORD = 1, // Kokiri Sword
    BINGO_GOAL_MAP_COMPASS_IN_DEKU_TREE = 2, // Map & Compass in Deku Tree
    BINGO_GOAL_SILVER_SCALE = 3, // Silver Scale
    BINGO_GOAL_BOLERO_OF_FIRE = 4, // Bolero of Fire
    BINGO_GOAL_EXACTLY_20_DEKU_STICKS = 5, // Exactly 20 Deku Sticks
    BINGO_GOAL_MINUET_OF_FOREST = 6, // Minuet of Forest
    BINGO_GOAL_ZORA_TUNIC = 7, // Zora Tunic
    BINGO_GOAL_N_2_SKULLTULAS_IN_LON_LON_RANCH = 8, // 2 Skulltulas in Lon-Lon Ranch
    BINGO_GOAL_N_3_LAKE_HYLIA_SKULLTULAS = 9, // 3 Lake Hylia Skulltulas
    BINGO_GOAL_BLUE_FIRE = 10, // Blue Fire
    BINGO_GOAL_DEFEAT_A_SKULL_KID = 11, // Defeat a Skull Kid
    BINGO_GOAL_GORON_TUNIC = 12, // Goron Tunic
    BINGO_GOAL_PLANT_BEAN_IN_DEATH_MOUNTAIN_CRATER = 13, // Plant Bean in Death Mountain Crater
    BINGO_GOAL_RUTO_S_LETTER = 14, // Ruto's Letter
    BINGO_GOAL_BOTTLED_POE = 15, // Bottled Poe
    BINGO_GOAL_RED_POTION = 16, // Red Potion
    BINGO_GOAL_ADULT_S_WALLET = 17, // Adult's Wallet
    BINGO_GOAL_PLANT_BEAN_IN_GERUDO_VALLEY = 18, // Plant Bean in Gerudo Valley
    BINGO_GOAL_BOTTLED_FAIRY = 19, // Bottled Fairy
    BINGO_GOAL_DEFEAT_NABOORU_KNUCKLE = 20, // Defeat Nabooru-Knuckle
    BINGO_GOAL_FAIRY_SLINGSHOT = 21, // Fairy Slingshot
    BINGO_GOAL_LON_LON_RANCH_HP = 22, // Lon-Lon Ranch HP
    BINGO_GOAL_MIRROR_SHIELD = 23, // Mirror Shield
    BINGO_GOAL_REQUIEM_OF_SPIRIT = 24, // Requiem of Spirit
    BINGO_GOAL_FAIRY_OCARINA = 25, // Fairy Ocarina
    BINGO_GOAL_BLACK_GAUNTLETS = 26, // Black Gauntlets
    BINGO_GOAL_N_3_SKULLS_IN_DEKU_TREE = 27, // 3 Skulls in Deku Tree
    BINGO_GOAL_OPEN_MIDO_S_GREEN_RUPEE_CHEST = 28, // Open Mido's Green Rupee Chest
    BINGO_GOAL_N_5_HEARTS = 29, // 5 Hearts
    BINGO_GOAL_N_5_MAGIC_BEANS = 30, // 5 Magic Beans
    BINGO_GOAL_DEFEAT_BONGO_BONGO = 31, // Defeat Bongo-Bongo
    BINGO_GOAL_DEFEAT_QUEEN_GOHMA = 32, // Defeat Queen Gohma
    BINGO_GOAL_GIANT_S_KNIFE = 33, // Giant's Knife
    BINGO_GOAL_N_3_MAPS = 34, // 3 Maps
    BINGO_GOAL_N_3_TUNICS = 35, // 3 Tunics
    BINGO_GOAL_GANON_S_CASTLE_BOSS_KEY = 36, // Ganon's Castle Boss Key
    BINGO_GOAL_N_3_SONGS = 37, // 3 Songs
    BINGO_GOAL_N_37TH_HEART_PIECE_CHILD_FORTRESS = 38, // 37th Heart Piece (Child Fortress)
    BINGO_GOAL_BEAT_THE_DEKU_TREE = 39, // Beat the Deku Tree
    BINGO_GOAL_BEAT_THE_SHADOW_TEMPLE = 40, // Beat the Shadow Temple
    BINGO_GOAL_BULLET_BAG_40 = 41, // Bullet Bag (40)
    BINGO_GOAL_GOLDEN_SCALE = 42, // Golden Scale
    BINGO_GOAL_N_3_COMPASSES = 43, // 3 Compasses
    BINGO_GOAL_BEAT_2_DUNGEONS = 44, // Beat 2 Dungeons
    BINGO_GOAL_N_3_SKULLTULAS_IN_DODONGO_S_CAVERN = 45, // 3 Skulltulas in Dodongo's Cavern
    BINGO_GOAL_N_4_SONGS = 46, // 4 Songs
    BINGO_GOAL_N_30_DEKU_NUTS = 47, // 30 Deku Nuts
    BINGO_GOAL_ALL_3_KOKIRI_FOREST_AREA_SKULLTULAS = 48, // All 3 Kokiri Forest Area Skulltulas
    BINGO_GOAL_ICE_ARROWS = 49, // Ice Arrows
    BINGO_GOAL_MAP_COMPASS_IN_BOTTOM_OF_THE_WELL = 50, // Map & Compass in Bottom of the Well
    BINGO_GOAL_MAP_COMPASS_IN_DODONGO_S_CAVERN = 51, // Map & Compass in Dodongo's Cavern
    BINGO_GOAL_MAP_COMPASS_IN_WATER_TEMPLE = 52, // Map & Compass in Water Temple
    BINGO_GOAL_PLANT_3_MAGIC_BEANS = 53, // Plant 3 Magic Beans
    BINGO_GOAL_OBTAIN_ONE_HYRULE_FIELD_HP = 54, // Obtain one Hyrule Field HP
    BINGO_GOAL_N_4_UNUSED_KEYS_IN_GERUDO_TRAINING_GROUNDS = 55, // 4 Unused Keys in Gerudo Training Grounds
    BINGO_GOAL_N_6_HEARTS = 56, // 6 Hearts
    BINGO_GOAL_BOOMERANG = 57, // Boomerang
    BINGO_GOAL_DEFEAT_A_WHITE_WOLFOS = 58, // Defeat a White Wolfos
    BINGO_GOAL_DEFEAT_ALL_LIZALFOS_IN_DODONGO_S_CAVERN = 59, // Defeat all Lizalfos in Dodongo's Cavern
    BINGO_GOAL_GET_BOMBCHU_CHEST_IN_SPIRIT_TEMPLE = 60, // Get Bombchu Chest in Spirit Temple
    BINGO_GOAL_MAP_COMPASS_IN_SHADOW_TEMPLE = 61, // Map & Compass in Shadow Temple
    BINGO_GOAL_CLEAR_4_SILVER_RUPEE_ROOMS = 62, // Clear 4 Silver Rupee Rooms
    BINGO_GOAL_DESERT_COLOSSUS_HP = 63, // Desert Colossus HP
    BINGO_GOAL_N_7_KAKARIKO_AREA_SKULLTULAS = 64, // 7 Kakariko Area Skulltulas
    BINGO_GOAL_DEFEAT_5_GIBDOS = 65, // Defeat 5 Gibdos
    BINGO_GOAL_N_15_DIFFERENT_SKULLTULAS = 66, // 15 Different Skulltulas
    BINGO_GOAL_N_3_SHIELDS = 67, // 3 Shields
    BINGO_GOAL_BOMB_BAG_30 = 68, // Bomb Bag (30)
    BINGO_GOAL_BULLET_BAG_50 = 69, // Bullet Bag (50)
    BINGO_GOAL_DEFEAT_BIG_OCTO = 70, // Defeat Big Octo
    BINGO_GOAL_DEFEAT_KING_DODONGO = 71, // Defeat King Dodongo
    BINGO_GOAL_ICE_CAVERN_HP = 72, // Ice Cavern HP
    BINGO_GOAL_MAP_COMPASS_IN_ICE_CAVERN = 73, // Map & Compass in Ice Cavern
    BINGO_GOAL_MAP_COMPASS_IN_JABU_JABU = 74, // Map & Compass in Jabu-Jabu
    BINGO_GOAL_SILVER_GAUNTLETS = 75, // Silver Gauntlets
    BINGO_GOAL_GORON_CITY_HP = 76, // Goron City HP
    BINGO_GOAL_KILL_ALL_TENTACLES_IN_JABU_JABU = 77, // Kill All Tentacles in Jabu-Jabu
    BINGO_GOAL_N_5_SONGS = 78, // 5 Songs
    BINGO_GOAL_DIN_S_FIRE = 79, // Din's Fire
    BINGO_GOAL_FARORE_S_WIND = 80, // Farore's Wind
    BINGO_GOAL_KEATON_MASK = 81, // Keaton Mask
    BINGO_GOAL_N_2_BOSS_KEYS = 82, // 2 Boss Keys
    BINGO_GOAL_N_4_MAPS = 83, // 4 Maps
    BINGO_GOAL_ALL_4_LOST_WOODS_AREA_SKULLTULAS = 84, // All 4 Lost Woods Area Skulltulas
    BINGO_GOAL_BEAT_DODONGO_S_CAVERN = 85, // Beat Dodongo's Cavern
    BINGO_GOAL_BOTH_HPS_IN_DEATH_MOUNTAIN_CRATER = 86, // Both HPs in Death Mountain Crater
    BINGO_GOAL_DEFEAT_MORPHA = 87, // Defeat Morpha
    BINGO_GOAL_IRON_BOOTS = 88, // Iron Boots
    BINGO_GOAL_LENS_OF_TRUTH = 89, // Lens of Truth
    BINGO_GOAL_COJIRO = 90, // Cojiro
    BINGO_GOAL_N_3_SWORDS_3_TUNICS = 91, // 3 Swords & 3 Tunics
    BINGO_GOAL_NAYRU_S_LOVE = 92, // Nayru's Love
    BINGO_GOAL_BEAT_3_DUNGEONS = 93, // Beat 3 Dungeons
    BINGO_GOAL_PLANT_4_MAGIC_BEANS = 94, // Plant 4 Magic Beans
    BINGO_GOAL_OPEN_UPPER_GERUDO_FORTRESS_CHEST = 95, // Open Upper Gerudo Fortress Chest
    BINGO_GOAL_N_4_COMPASSES = 96, // 4 Compasses
    BINGO_GOAL_ALL_4_SKULLTULAS_IN_DEKU_TREE = 97, // All 4 Skulltulas in Deku Tree
    BINGO_GOAL_ALL_8_KAKARIKO_AREA_SKULLTULAS = 98, // All 8 Kakariko Area Skulltulas
    BINGO_GOAL_BEAT_THE_WATER_TEMPLE = 99, // Beat the Water Temple
    BINGO_GOAL_BOTH_GERUDO_S_FORTRESS_AREA_SKULLTULAS = 100, // Both Gerudo's Fortress Area Skulltulas
    BINGO_GOAL_FILL_ALL_4_BOTTLE_SLOTS = 101, // Fill All 4 Bottle Slots
    BINGO_GOAL_SARIA_S_SONG = 102, // Saria's Song
    BINGO_GOAL_SERENADE_OF_WATER = 103, // Serenade of Water
    BINGO_GOAL_N_4_LAKE_HYLIA_SKULLTULAS = 104, // 4 Lake Hylia Skulltulas
    BINGO_GOAL_ALL_4_SKULLTULAS_IN_JABU_JABU = 105, // All 4 Skulltulas in Jabu-Jabu
    BINGO_GOAL_DOUBLE_MAGIC = 106, // Double Magic
    BINGO_GOAL_GET_TO_THE_END_OF_LIGHT_TRIAL = 107, // Get to the End of Light Trial
    BINGO_GOAL_N_4_DEATH_MOUNTAIN_AREA_SKULLTULAS = 108, // 4 Death Mountain Area Skulltulas
    BINGO_GOAL_DEFEAT_6_DIFFERENT_STALFOS = 109, // Defeat 6 different Stalfos
    BINGO_GOAL_N_6_SONGS = 110, // 6 Songs
    BINGO_GOAL_BLUE_GAUNTLETS = 111, // Blue Gauntlets
    BINGO_GOAL_DEFEAT_PHANTOM_GANON = 112, // Defeat Phantom Ganon
    BINGO_GOAL_DOUBLE_DEFENSE = 113, // Double Defense
    BINGO_GOAL_FIRE_TEMPLE_BOSS_KEY = 114, // Fire Temple Boss Key
    BINGO_GOAL_SHADOW_TEMPLE_BOSS_KEY = 115, // Shadow Temple Boss Key
    BINGO_GOAL_SPIRIT_TEMPLE_BOSS_KEY = 116, // Spirit Temple Boss Key
    BINGO_GOAL_SKULL_MASK = 117, // Skull Mask
    BINGO_GOAL_LOWER_FINAL_PLATFORM_IN_SPIRIT_TEMPLE = 118, // Lower Final Platform in Spirit Temple
    BINGO_GOAL_FREE_3_CARPENTERS = 119, // Free 3 Carpenters
    BINGO_GOAL_DISPEL_3_TRIAL_BEAMS = 120, // Dispel 3 trial beams
    BINGO_GOAL_N_1_SKULLTULA_FROM_EACH_CHILD_DUNGEON = 121, // 1 Skulltula From Each Child Dungeon
    BINGO_GOAL_N_3_BOOTS = 122, // 3 Boots
    BINGO_GOAL_N_5_MAPS = 123, // 5 Maps
    BINGO_GOAL_N_7_HEARTS = 124, // 7 Hearts
    BINGO_GOAL_ALL_3_SKULLTULAS_IN_ICE_CAVERN = 125, // All 3 Skulltulas in Ice Cavern
    BINGO_GOAL_BEAT_THE_FOREST_TEMPLE = 126, // Beat the Forest Temple
    BINGO_GOAL_EPONA_S_SONG = 127, // Epona's Song
    BINGO_GOAL_WATER_TEMPLE_BOSS_KEY = 128, // Water Temple Boss Key
    BINGO_GOAL_ALL_4_CHILD_SKULLTULAS_IN_ZORA_S_DOMAIN_AREA = 129, // All 4 Child Skulltulas in Zora's Domain Area
    BINGO_GOAL_GOLDEN_RUPEE_CHEST_IN_FIRE_TEMPLE = 130, // Golden Rupee Chest in Fire Temple
    BINGO_GOAL_TWO_FAIRY_SPELLS = 131, // Two Fairy Spells
    BINGO_GOAL_N_3_SHIELDS_3_TUNICS = 132, // 3 Shields & 3 Tunics
    BINGO_GOAL_N_6_UNUSED_KEYS_IN_GERUDO_TRAINING_GROUNDS = 133, // 6 Unused Keys in Gerudo Training Grounds
    BINGO_GOAL_ALL_4_LON_LON_RANCH_AREA_SKULLTULAS = 134, // All 4 Lon-Lon Ranch Area Skulltulas
    BINGO_GOAL_DEFEAT_AMY_GREEN_POE = 135, // Defeat Amy (Green Poe)
    BINGO_GOAL_DEFEAT_TWINROVA = 136, // Defeat Twinrova
    BINGO_GOAL_GIANT_S_WALLET = 137, // Giant's Wallet
    BINGO_GOAL_MAP_COMPASS_IN_FIRE_TEMPLE = 138, // Map & Compass in Fire Temple
    BINGO_GOAL_WIN_BOMBCHU_BOWLING_PRIZE = 139, // Win Bombchu Bowling Prize
    BINGO_GOAL_DEFEAT_BOTH_DEAD_HANDS = 140, // Defeat Both Dead-Hands
    BINGO_GOAL_EXACTLY_30_DEKU_STICKS = 141, // Exactly 30 Deku Sticks
    BINGO_GOAL_N_4_UNUSED_KEYS_IN_FOREST_TEMPLE = 142, // 4 Unused Keys in Forest Temple
    BINGO_GOAL_N_6_MAGIC_BEANS = 143, // 6 Magic Beans
    BINGO_GOAL_N_6_DEATH_MOUNTAIN_AREA_SKULLTULAS = 144, // 6 Death Mountain Area Skulltulas
    BINGO_GOAL_N_3_BOSS_KEYS = 145, // 3 Boss Keys
    BINGO_GOAL_N_20_DIFFERENT_SKULLTULAS = 146, // 20 Different Skulltulas
    BINGO_GOAL_ALL_3_SKULLTULAS_IN_BOTTOM_OF_THE_WELL = 147, // All 3 Skulltulas in Bottom of the Well
    BINGO_GOAL_ALL_4_GERUDO_VALLEY_AREA_SKULLTULAS = 148, // All 4 Gerudo Valley Area Skulltulas
    BINGO_GOAL_FAIRY_BOW = 149, // Fairy Bow
    BINGO_GOAL_FOREST_TEMPLE_BOSS_KEY = 150, // Forest Temple Boss Key
    BINGO_GOAL_GERUDO_S_CARD = 151, // Gerudo's Card
    BINGO_GOAL_MILK = 152, // Milk
    BINGO_GOAL_GET_TO_THE_END_OF_FIRE_TRIAL = 153, // Get to the End of Fire Trial
    BINGO_GOAL_MAP_COMPASS_IN_FOREST_TEMPLE = 154, // Map & Compass in Forest Temple
    BINGO_GOAL_BEAT_THE_SKY_TEMPLE = 155, // Beat the Sky Temple
    BINGO_GOAL_DEFEAT_3_DIFFERENT_WHITE_WOLFOS = 156, // Defeat 3 Different White Wolfos
    BINGO_GOAL_DEFEAT_JOELLE_RED_POE_BETH_BLUE_POE = 157, // Defeat Joelle (Red Poe) & Beth (Blue Poe)
    BINGO_GOAL_OPEN_GANON_S_BOSS_DOOR = 158, // Open Ganon's Boss Door
    BINGO_GOAL_N_3_SWORDS_3_SHIELDS = 159, // 3 Swords & 3 Shields
    BINGO_GOAL_N_5_COMPASSES = 160, // 5 Compasses
    BINGO_GOAL_ALL_4_WASTELAND_COLOSSUS_AREA_SKULLTULAS = 161, // All 4 Wasteland/ Colossus Area Skulltulas
    BINGO_GOAL_ALL_5_SKULLTULAS_IN_DODONGO_S_CAVERN = 162, // All 5 Skulltulas in Dodongo's Cavern
    BINGO_GOAL_BEAT_THE_SPIRIT_TEMPLE = 163, // Beat the Spirit Temple
    BINGO_GOAL_GET_TO_THE_END_OF_SPIRIT_TRIAL = 164, // Get to the End of Spirit Trial
    BINGO_GOAL_OPEN_3_GOLD_RUPEE_CHESTS = 165, // Open 3 Gold Rupee Chests
    BINGO_GOAL_BOTTLED_BIG_POE = 166, // Bottled Big Poe
    BINGO_GOAL_PRELUDE_OF_LIGHT = 167, // Prelude of Light
    BINGO_GOAL_ALL_4_ADULT_SKULLTULAS_IN_ZORA_S_DOMAIN_AREA = 168, // All 4 Adult Skulltulas in Zora's Domain Area
    BINGO_GOAL_DEFEAT_4_DIFFERENT_IRON_KNUCKLES = 169, // Defeat 4 Different Iron Knuckles
    BINGO_GOAL_N_6_ZORA_S_DOMAIN_AREA_SKULLTULAS = 170, // 6 Zora's Domain Area Skulltulas
    BINGO_GOAL_OPEN_ALL_6_ICE_TRAP_CHESTS = 171, // Open all 6 Ice Trap Chests
    BINGO_GOAL_N_3_TUNICS_3_BOOTS = 172, // 3 Tunics & 3 Boots
    BINGO_GOAL_N_500_RUPEES = 173, // 500 Rupees
    BINGO_GOAL_N_7_MAGIC_BEANS = 174, // 7 Magic Beans
    BINGO_GOAL_N_7_SONGS = 175, // 7 Songs
    BINGO_GOAL_COW_IN_HOUSE = 176, // Cow in House
    BINGO_GOAL_DEFEAT_BARINADE = 177, // Defeat Barinade
    BINGO_GOAL_FIRE_ARROWS = 178, // Fire Arrows
    BINGO_GOAL_OBTAIN_ALL_5_SMALL_KEYS_IN_FOREST_TEMPLE = 179, // Obtain All 5 Small Keys in Forest Temple
    BINGO_GOAL_QUIVER_40 = 180, // Quiver (40)
    BINGO_GOAL_N_5_CHILD_DEATH_MOUNTAIN_AREA_SKULLTULAS = 181, // 5 Child Death Mountain Area Skulltulas
    BINGO_GOAL_N_3_SWORDS_3_BOOTS = 182, // 3 Swords & 3 Boots
    BINGO_GOAL_N_4_SKULLTULAS_IN_SHADOW_TEMPLE = 183, // 4 Skulltulas in Shadow Temple
    BINGO_GOAL_N_6_MAPS = 184, // 6 Maps
    BINGO_GOAL_N_8_HEARTS = 185, // 8 Hearts
    BINGO_GOAL_ALL_5_SKULLTULAS_IN_FOREST_TEMPLE = 186, // All 5 Skulltulas in Forest Temple
    BINGO_GOAL_BEAT_JABU_JABU_S_BELLY = 187, // Beat Jabu-Jabu's Belly
    BINGO_GOAL_STONE_OF_AGONY = 188, // Stone of Agony
    BINGO_GOAL_UNLOCK_PHANTOM_GANON_S_BOSS_DOOR = 189, // Unlock Phantom Ganon's Boss Door
    BINGO_GOAL_BEAT_4_DUNGEONS = 190, // Beat 4 Dungeons
    BINGO_GOAL_N_3_SHIELDS_3_BOOTS = 191, // 3 Shields & 3 Boots
    BINGO_GOAL_ALL_5_SKULLTULAS_IN_FIRE_TEMPLE = 192, // All 5 Skulltulas in Fire Temple
    BINGO_GOAL_DEFEAT_DARK_LINK = 193, // Defeat Dark Link
    BINGO_GOAL_GET_TO_THE_END_OF_FOREST_TRIAL = 194, // Get to the End of Forest Trial
    BINGO_GOAL_GORON_BRACELET = 195, // Goron Bracelet
    BINGO_GOAL_LONGSHOT = 196, // Longshot
    BINGO_GOAL_OPEN_50_RUPEE_CHEST_IN_WASTELAND = 197, // Open 50 Rupee Chest in Wasteland
    BINGO_GOAL_ALL_8_DEATH_MOUNTAIN_AREA_SKULLTULAS = 198, // All 8 Death Mountain Area Skulltulas
    BINGO_GOAL_DEFEAT_10_DIFFERENT_STALFOS = 199, // Defeat 10 different Stalfos
    BINGO_GOAL_N_6_COMPASSES = 200, // 6 Compasses
    BINGO_GOAL_N_8_SONGS = 201, // 8 Songs
    BINGO_GOAL_BOTH_HPS_IN_LOST_WOODS = 202, // Both HPs in Lost Woods
    BINGO_GOAL_DEFEAT_BOTH_FLARE_DANCERS = 203, // Defeat Both Flare Dancers
    BINGO_GOAL_MAP_COMPASS_IN_SPIRIT_TEMPLE = 204, // Map & Compass in Spirit Temple
    BINGO_GOAL_MEGATON_HAMMER = 205, // Megaton Hammer
    BINGO_GOAL_OBTAIN_ALL_8_SMALL_KEYS_IN_FIRE_TEMPLE = 206, // Obtain All 8 Small Keys in Fire Temple
    BINGO_GOAL_OPEN_4_SMALL_KEY_CHESTS_IN_SPIRIT_TEMPLE = 207, // Open 4 Small Key Chests in Spirit Temple
    BINGO_GOAL_BOTH_HYRULE_FIELD_HPS = 208, // Both Hyrule Field HPs
    BINGO_GOAL_MILK_4_COWS = 209, // Milk 4 Cows
    BINGO_GOAL_N_3_SKULLTULAS_IN_WATER_TEMPLE = 210, // 3 Skulltulas in Water Temple
    BINGO_GOAL_N_5_ZORA_AREA_HPS = 211, // 5 Zora area HPs
    BINGO_GOAL_ALL_8_ZORA_S_DOMAIN_AREA_SKULLTULAS = 212, // All 8 Zora's Domain Area Skulltulas
    BINGO_GOAL_FREE_ALL_9_GORONS_IN_FIRE_TEMPLE = 213, // Free All 9 Gorons in Fire Temple
    BINGO_GOAL_FROG_S_HP = 214, // Frog's HP
    BINGO_GOAL_CLEAR_8_SILVER_RUPEE_ROOMS = 215, // Clear 8 Silver Rupee Rooms
    BINGO_GOAL_N_4_BOSS_KEYS = 216, // 4 Boss Keys
    BINGO_GOAL_GOLDEN_GAUNTLETS = 217, // Golden Gauntlets
    BINGO_GOAL_OPEN_3_SMALL_KEY_CHESTS_IN_WATER_TEMPLE = 218, // Open 3 Small Key Chests in Water Temple
    BINGO_GOAL_N_3_SWORDS_TUNICS_BOOTS_AND_SHIELDS = 219, // 3 Swords, Tunics, Boots and Shields
    BINGO_GOAL_N_7_MAPS = 220, // 7 Maps
    BINGO_GOAL_N_9_HEARTS = 221, // 9 Hearts
    BINGO_GOAL_GREEN_GAUNTLETS = 222, // Green Gauntlets
    BINGO_GOAL_ALL_5_SKULLTULAS_IN_SPIRIT_TEMPLE = 223, // All 5 Skulltulas in Spirit Temple
    BINGO_GOAL_GET_TO_THE_END_OF_WATER_TRIAL = 224, // Get to the End of Water Trial
    BINGO_GOAL_DEFEAT_5_DIFFERENT_IRON_KNUCKLES = 225, // Defeat 5 Different Iron Knuckles
    BINGO_GOAL_DISPEL_5_TRIAL_BEAMS = 226, // Dispel 5 trial beams
    BINGO_GOAL_N_1_SKULLTULA_FROM_EACH_ADULT_DUNGEON = 227, // 1 Skulltula From Each Adult Dungeon
    BINGO_GOAL_N_8_DIFFERENT_UNUSED_KEYS_IN_GERUDO_TRAINING_GROUNDS = 228, // 8 Different Unused Keys in Gerudo Training Grounds
    BINGO_GOAL_DEFEAT_MEG_PURPLE_POE = 229, // Defeat Meg (Purple Poe)
    BINGO_GOAL_OPEN_ALL_5_SMALL_KEY_CHESTS_IN_SPIRIT_TEMPLE = 230, // Open All 5 Small Key Chests in Spirit Temple
    BINGO_GOAL_LIGHT_ARROWS = 231, // Light Arrows
    BINGO_GOAL_BEAT_5_DUNGEONS = 232, // Beat 5 Dungeons
    BINGO_GOAL_OBTAIN_ALL_3_SPIRITUAL_STONES = 233, // Obtain All 3 Spiritual Stones
    BINGO_GOAL_N_1_UNUSED_SMALL_KEY_IN_EACH_ADULT_DUNGEON = 234, // 1 Unused Small Key in Each Adult Dungeon
    BINGO_GOAL_N_7_COMPASSES = 235, // 7 Compasses
    BINGO_GOAL_N_9_SONGS = 236, // 9 Songs
    BINGO_GOAL_ALL_5_LAKE_HYLIA_SKULLTULAS = 237, // All 5 Lake Hylia Skulltulas
    BINGO_GOAL_DEFEAT_VOLVAGIA = 238, // Defeat Volvagia
    BINGO_GOAL_N_25_DIFFERENT_SKULLTULAS = 239, // 25 Different Skulltulas
    BINGO_GOAL_ALL_3_ELEMENTAL_ARROWS = 240, // All 3 Elemental Arrows
    BINGO_GOAL_ALL_5_SKULLTULAS_IN_SHADOW_TEMPLE = 241, // All 5 Skulltulas in Shadow Temple
    BINGO_GOAL_BEAT_THE_FIRE_TEMPLE = 242, // Beat the Fire Temple
    BINGO_GOAL_BOTH_HYRULE_FIELD_AREA_SKULLTULAS = 243, // Both Hyrule Field Area Skulltulas
    BINGO_GOAL_OBTAIN_ALL_5_SMALL_KEYS_IN_SHADOW_TEMPLE = 244, // Obtain All 5 Small Keys in Shadow Temple
    BINGO_GOAL_ALL_9_SOFT_SOIL_SKULLTULAS = 245, // All 9 Soft Soil Skulltulas
    BINGO_GOAL_N_10_SONGS = 246, // 10 Songs
    BINGO_GOAL_ALL_4_MARKET_AREA_SKULLTULAS = 247, // All 4 Market Area Skulltulas
    BINGO_GOAL_BLUE_POTION = 248, // Blue Potion
    BINGO_GOAL_GET_TO_THE_END_OF_SHADOW_TRIAL = 249, // Get to the End of Shadow Trial
    BINGO_GOAL_BOTH_RUSTY_SWITCHES_IN_SPIRIT_TEMPLE = 250, // Both Rusty Switches in Spirit Temple
    BINGO_GOAL_ALL_5_SKULLTULAS_IN_WATER_TEMPLE = 251, // All 5 Skulltulas in Water Temple
    BINGO_GOAL_N_30_DIFFERENT_SKULLTULAS = 252, // 30 Different Skulltulas
    BINGO_GOAL_N_6_ZORA_AREA_HPS = 253, // 6 Zora area HPs
    BINGO_GOAL_DEFEAT_GANONDORF = 254, // Defeat Ganondorf
    BINGO_GOAL_COUNT = 255
} bingo_goal_t;

typedef struct {
    uint16_t current;
    uint16_t target;
    bool complete;
} bingo_progress_t;

bingo_progress_t bingo_get_goal_progress(bingo_goal_t goal);
bool bingo_goal_is_complete(bingo_goal_t goal);
uint16_t bingo_get_global_count(global_count_t count_index);
uint16_t bingo_get_dungeon_count(uint8_t dungeon_index, dungeon_count_t count_index);
uint16_t bingo_get_area_count(area_t area_index, area_count_t count_index);
uint8_t bingo_get_current_area_index(void);
uint8_t bingo_get_inventory_item(uint8_t slot);
uint8_t bingo_get_ammo(uint8_t slot);
uint8_t bingo_get_magic_level(void);
uint8_t bingo_get_strength_level(void);
uint8_t bingo_get_scale_level(void);
uint8_t bingo_get_capacity_level(uint8_t item);

bool bingo_item_owned(uint8_t item);

#endif
