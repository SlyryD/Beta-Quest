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
    BEAN = 0x50
    TRAP = 0x51
    GOLD_RUPEE = 0x52
    # Enemies
    IRON_KNUCKLE = 0x60
    WHITE_WOLFOS = 0x61
    LIZALFOS = 0x62
    GIBDO = 0x63
    TENTACLE = 0x64
    STALFOS = 0x65
    DEAD_HANDS = 0x66
    FLARE_DANCER = 0x67
