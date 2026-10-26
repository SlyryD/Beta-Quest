from enum import IntEnum

# Overworld areas
class Area(IntEnum):
    WASTELAND = 0x00
    FORTRESS = 0x01
    GERUDO_VALLEY = 0x02
    LAKE_HYLIA = 0x03
    LON_LON_RANCH = 0x04
    MARKET = 0x05
    HYRULE_FIELD = 0x06
    DEATH_MOUNTAIN = 0x07
    KAKARIKO = 0x08
    LOST_WOODS = 0x09
    KOKIRI_FOREST = 0x0A
    ZORA = 0x0B


class Dungeon(IntEnum):
    DEKU_TREE = 0x00
    DODONGOS_CAVERN = 0x01
    JABU_JABUS_BELLY = 0x02
    FOREST_TEMPLE = 0x03
    FIRE_TEMPLE = 0x04
    WATER_TEMPLE = 0x05
    SPIRIT_TEMPLE = 0x06
    SHADOW_TEMPLE = 0x07
    BOTTOM_OF_THE_WELL = 0x08
    ICE_CAVERN = 0x09
    GERUDO_TRAINING_GROUND = 0x0B
    THIEVES_HIDEOUT = 0x0C
    GANONS_CASTLE = 0x0D


# DungeonCounts are counted per dungeon
class DungeonCount(IntEnum):
    # Quest
    TOKEN = 0x00
    HEART_PIECE = 0x01
    # Dungeon
    SMALL_KEY = 0x10


# AreaCounts are counted per overworld area
class AreaCount(IntEnum):
    # Quest
    TOKEN = 0x00
    HEART_PIECE = 0x01


# GlobalCounts are counted globally
class GlobalCount(IntEnum):
    # Equipment
    SWORD = 0x00
    SHIELD = 0x01
    TUNIC = 0x02
    BOOT = 0x03
    # Quest
    SONG = 0x10
    MEDALLION = 0x11
    STONE = 0x12
    REWARD = 0x13
    TOKEN = 0x14
    # Dungeon
    BOSS_KEY = 0x20
    COMPASS = 0x21
    MAP = 0x22
    # Inventory
    ELEMENTAL_ARROW = 0x30
    FAIRY_SPELL = 0x31
    BOTTLE_SLOT = 0x32
    # Flags
    CARPENTER = 0x40
    # Items not present in menu (count stored in save context)
    BEANS_PLANTED = 0x50
    TRAP = 0x51
    GOLD_RUPEE = 0x52
    SILVER_RUPEE_ROOMS = 0x53
    # Enemies
    IRON_KNUCKLE = 0x60
    WHITE_WOLFOS = 0x61
    LIZALFOS = 0x62
    GIBDO = 0x63
    TENTACLE = 0x64
    STALFOS = 0x65
    DEAD_HANDS = 0x66
    FLARE_DANCER = 0x67
