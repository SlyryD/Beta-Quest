# text details: https://wiki.cloudmodding.com/oot/Text_Format

from __future__ import annotations
from collections.abc import Callable
from enum import IntEnum
from typing import TYPE_CHECKING, Literal, Optional, Any

from Utils import find_last
from Counts import Area, AreaCount, Dungeon, DungeonCount, GlobalCount

if TYPE_CHECKING:
    from Rom import Rom

ENG_TEXT_START: int = 0x92D000
JPN_TEXT_START: int = 0x8EB000
ENG_TEXT_SIZE_LIMIT: int = 0x39000
JPN_TEXT_SIZE_LIMIT: int = 0x3B000

JPN_TABLE_START: int = 0xB808AC
ENG_TABLE_START: int = 0xB849EC
CREDITS_TABLE_START: int = 0xB88C0C

JPN_TABLE_SIZE: int = ENG_TABLE_START - JPN_TABLE_START
ENG_TABLE_SIZE: int = CREDITS_TABLE_START - ENG_TABLE_START

EXTENDED_TABLE_START: int = JPN_TABLE_START  # start writing entries to the jp table instead of english for more space
EXTENDED_TABLE_SIZE: int = JPN_TABLE_SIZE + ENG_TABLE_SIZE  # 0x8360 bytes, 4204 entries

EXTENDED_TEXT_SIZE_LIMIT: int = JPN_TEXT_SIZE_LIMIT + ENG_TEXT_SIZE_LIMIT  # 0x74000 bytes

# name of type, followed by number of additional bytes to read, follwed by a function that prints the code
CONTROL_CODES: dict[int, tuple[str, int, Callable[[Any], str]]] = {
    0x00: ('pad', 0, lambda _: '<pad>' ),
    0x01: ('line-break', 0, lambda _: '\n' ),
    0x02: ('end', 0, lambda _: '' ),
    0x04: ('box-break', 0, lambda _: '\n▼\n' ),
    0x05: ('color', 1, lambda d: '<color ' + "{:02x}".format(d) + '>' ),
    0x06: ('gap', 1, lambda d: '<' + str(d) + 'px gap>' ),
    0x07: ('goto', 2, lambda d: '<goto ' + "{:04x}".format(d) + '>' ),
    0x08: ('instant', 0, lambda _: '<allow instant text>' ),
    0x09: ('un-instant', 0, lambda _: '<disallow instant text>' ),
    0x0A: ('keep-open', 0, lambda _: '<keep open>' ),
    0x0B: ('event', 0, lambda _: '<event>' ),
    0x0C: ('box-break-delay', 1, lambda d: '\n▼<wait ' + str(d) + ' frames>\n' ),
    0x0E: ('fade-out', 1, lambda d: '<fade after ' + str(d) + ' frames?>' ),
    0x0F: ('name', 0, lambda _: '<name>' ),
    0x10: ('ocarina', 0, lambda _: '<ocarina>' ),
    0x12: ('sound', 2, lambda d: '<play SFX ' + "{:04x}".format(d) + '>' ),
    0x13: ('icon', 1, lambda d: '<icon ' + "{:02x}".format(d) + '>' ),
    0x14: ('speed', 1, lambda d: '<delay each character by ' + str(d) + ' frames>' ),
    0x15: ('background', 3, lambda d: '<set background to ' + "{:06x}".format(d) + '>' ),
    0x16: ('marathon', 0, lambda _: '<marathon time>' ),
    0x17: ('race', 0, lambda _: '<race time>' ),
    0x18: ('points', 0, lambda _: '<points>' ),
    0x19: ('skulltula', 0, lambda _: '<skulltula count>' ),
    0x1A: ('unskippable', 0, lambda _: '<text is unskippable>' ),
    0x1B: ('two-choice', 0, lambda _: '<start two choice>' ),
    0x1C: ('three-choice', 0, lambda _: '<start three choice>' ),
    0x1D: ('fish', 0, lambda _: '<fish weight>' ),
    0x1E: ('high-score', 1, lambda d: '<high-score ' + "{:02x}".format(d) + '>' ),
    0x1F: ('time', 0, lambda _: '<current time>' ),
    # 0xF0: ('silver_rupee', 1, lambda d: '<silver rupee count ' + "{:02x}".format(d) + '>' ),
    # dungeon_count format is \xF1\xXX\xYY where XX is the dungeon id and YY is the item id
    0xF1: ('dungeon_count', 2, lambda d: '<dungeon count ' + "{:02x}".format((d >> 8) & 0xFF) + " {:02x}".format(d & 0xFF) + '>' ),
    # 0xF2: ('outgoing_item_filename', 0, lambda _: '<outgoing item filename>' ),
    0xF3: ('farores_wind_destination', 0, lambda _: '<farores wind destination>' ),
    # area_count format is \xF4\xXX\xYY where XX is the area id and YY is the item id
    0xF4: ('area_count', 2, lambda d: '<area count ' + "{:02x}".format((d >> 8) & 0xFF) + " {:02x}".format(d & 0xFF) + '>' ),
    # global_count format is \xF5\xXX where XX is the item id
    0xF5: ('global_count', 1, lambda d: '<global count ' + "{:02x}".format(d) + '>' ),
}


# Maps unicode characters to corresponding bytes in OOTR's character set.
CHARACTER_MAP: dict[str, int] = {
    'Ⓐ': 0x9F,
    'Ⓑ': 0xA0,
    'Ⓒ': 0xA1,
    'Ⓛ': 0xA2,
    'Ⓡ': 0xA3,
    'Ⓩ': 0xA4,
    '⯅': 0xA5,
    '⯆': 0xA6,
    '⯇': 0xA7,
    '⯈': 0xA8,
    chr(0xA9): 0xA9,  # Down arrow   -- not sure what best supports this
    chr(0xAA): 0xAA,  # Analog stick -- not sure what best supports this
}
# Support other ways of directly specifying controller inputs in OOTR's character set.
# (This is backwards-compatibility support for ShadowShine57's previous patch.)
CHARACTER_MAP.update(tuple((chr(v), v) for v in CHARACTER_MAP.values()))

# Characters 0x20 thru 0x7D map perfectly.  range() excludes the last element.
CHARACTER_MAP.update((chr(c), c) for c in range(0x20, 0x7e))

# Other characters, source: https://wiki.cloudmodding.com/oot/Text_Format
CHARACTER_MAP.update((c, ix) for ix, c in enumerate(
        (
            '\u203e'             # 0x7f
            'ÀîÂÄÇÈÉÊËÏÔÖÙÛÜß'   # 0x80 .. #0x8f
            'àáâäçèéêëïôöùûü'    # 0x90 .. #0x9e
        ),
        start=0x7f
))

SPECIAL_CHARACTERS: dict[int, str] = {
    0x9F: '[A]',
    0xA0: '[B]',
    0xA1: '[C]',
    0xA2: '[L]',
    0xA3: '[R]',
    0xA4: '[Z]',
    0xA5: '[C Up]',
    0xA6: '[C Down]',
    0xA7: '[C Left]',
    0xA8: '[C Right]',
    0xA9: '[Triangle]',
    0xAA: '[Control Stick]',
}

REVERSE_MAP: list[str] = list(chr(x) for x in range(256))

for char, byte in CHARACTER_MAP.items():
    SPECIAL_CHARACTERS.setdefault(byte, char)
    REVERSE_MAP[byte] = char

new_messages = [] # Used to keep track of new/updated messages to prevent duplicates. Clear it at the start of patches

class MessageColor(IntEnum):
    WHITE = 0x40
    RED = 0x41
    GREEN = 0x42
    BLUE = 0x43
    LIGHT_BLUE = 0x44
    PINK = 0x45
    YELLOW = 0x46
    BLACK = 0x47


def message_color(color: MessageColor) -> str:
    return f"\x05{color:c}"


TEXT_WHITE = message_color(MessageColor.WHITE)
PLAYER_NAME = "\xF3"


def colored(
    text: str,
    color: MessageColor,
    reset: Literal[MessageColor.WHITE] | None = MessageColor.WHITE,
) -> str:
    reset_code = "" if reset is None else TEXT_WHITE
    return f"{message_color(color)}{text}{reset_code}"


def red(text: str, reset: Literal[MessageColor.WHITE] | None = MessageColor.WHITE) -> str:
    return colored(text, MessageColor.RED, reset)


def green(text: str, reset: Literal[MessageColor.WHITE] | None = MessageColor.WHITE) -> str:
    return colored(text, MessageColor.GREEN, reset)


def blue(text: str, reset: Literal[MessageColor.WHITE] | None = MessageColor.WHITE) -> str:
    return colored(text, MessageColor.BLUE, reset)


def light_blue(text: str, reset: Literal[MessageColor.WHITE] | None = MessageColor.WHITE) -> str:
    return colored(text, MessageColor.LIGHT_BLUE, reset)


def pink(text: str, reset: Literal[MessageColor.WHITE] | None = MessageColor.WHITE) -> str:
    return colored(text, MessageColor.PINK, reset)


def yellow(text: str, reset: Literal[MessageColor.WHITE] | None = MessageColor.WHITE) -> str:
    return colored(text, MessageColor.YELLOW, reset)


def dungeon_count(dungeon: Dungeon, count: DungeonCount) -> str:
    return colored(f"\xF1{dungeon:c}{count:c}", MessageColor.RED)


def area_count(area: Area, count: AreaCount) -> str:
    return colored(f"\xF4{area:c}{count:c}", MessageColor.RED)


def global_count(count: GlobalCount) -> str:
    return colored(f"\xF5{count:c}", MessageColor.RED)


AREA_MESSAGE_INFO: tuple[tuple[Area, str, MessageColor], ...] = (
    (Area.WASTELAND, "Wasteland Area", MessageColor.YELLOW),
    (Area.FORTRESS, "Fortress Area", MessageColor.YELLOW),
    (Area.GERUDO_VALLEY, "Gerudo Valley", MessageColor.YELLOW),
    (Area.LAKE_HYLIA, "Lake Hylia", MessageColor.BLUE),
    (Area.LON_LON_RANCH, "Lon Lon Ranch", MessageColor.LIGHT_BLUE),
    (Area.MARKET, "Market Area", MessageColor.LIGHT_BLUE),
    (Area.HYRULE_FIELD, "Hyrule Field", MessageColor.LIGHT_BLUE),
    (Area.DEATH_MOUNTAIN, "Death Mountain", MessageColor.RED),
    (Area.KAKARIKO, "Kakariko Area", MessageColor.PINK),
    (Area.LOST_WOODS, "Lost Woods", MessageColor.GREEN),
    (Area.KOKIRI_FOREST, "Kokiri Forest", MessageColor.GREEN),
    (Area.ZORA, "Zora Area", MessageColor.BLUE),
)

DUNGEON_TOKEN_MESSAGE_INFO: tuple[tuple[Dungeon, str, MessageColor], ...] = (
    (Dungeon.DEKU_TREE, "Deku Tree", MessageColor.GREEN),
    (Dungeon.DODONGOS_CAVERN, "Dodongo's Cavern", MessageColor.RED),
    (Dungeon.JABU_JABUS_BELLY, "Jabu Jabu's Belly", MessageColor.BLUE),
    (Dungeon.FOREST_TEMPLE, "Forest Temple", MessageColor.GREEN),
    (Dungeon.FIRE_TEMPLE, "Fire Temple", MessageColor.RED),
    (Dungeon.WATER_TEMPLE, "Water Temple", MessageColor.BLUE),
    (Dungeon.SPIRIT_TEMPLE, "Spirit Temple", MessageColor.YELLOW),
    (Dungeon.SHADOW_TEMPLE, "Shadow Temple", MessageColor.PINK),
    (Dungeon.BOTTOM_OF_THE_WELL, "Bottom of the Well", MessageColor.PINK),
    (Dungeon.ICE_CAVERN, "Ice Cavern", MessageColor.BLUE),
)

# messages for shorter item messages
ITEM_MESSAGES: list[tuple[int, str]] = [
    (
        0x000C,
        f"""\x13\x3DYou got the {red("Biggoron's Sword")}!\x01"""
        f"""This blade was forged by a \x01"""
        f"""master smith and won't break!\x01"""
        f"""That's {global_count(GlobalCount.SWORD)} sword(s)!""",
    ),
    (
        0x0030,
        f"\x13\x06You found the {red('Fairy Slingshot')}!",
    ),
    (
        0x0031,
        f"\x13\x03You found the {red('Fairy Bow')}!",
    ),
    (
        0x0035,
        f"\x13\x0EYou found the {red('Boomerang')}!",
    ),
    (
        0x0036,
        f"\x13\x0AYou found the {red('Hookshot')}!\x01"
        f"It's a spring-loaded chain that\x01"
        f"you can cast out to hook things.",
    ),
    (
        0x0039,
        f"\x13\x0FYou found the {red('Lens of Truth')}!\x01"
        f"Mysterious things are hidden\x01"
        f"everywhere!",
    ),
    (
        0x003C,
        f"\x13\x67You received the {red('Fire', reset=None)}\x01"
        f"{red('Medallion')}! Darunia awakens as a\x01"
        f"Sage and adds his power to yours!\x01"
        f"That's {global_count(GlobalCount.MEDALLION)} medallion(s), {global_count(GlobalCount.REWARD)} reward(s)!",
    ),
    (
        0x003D,
        f"\x13\x68You received the {blue('Water', reset=None)}\x01"
        f"{blue('Medallion')}! Ruto awakens as a\x01"
        f"Sage and adds her power to yours!\x01"
        f"That's {global_count(GlobalCount.MEDALLION)} medallion(s), {global_count(GlobalCount.REWARD)} reward(s)!",
    ),
    (
        0x003E,
        f"\x13\x66You received the {green('Forest', reset=None)}\x01"
        f"{green('Medallion')}! Saria awakens as a\x01"
        f"Sage and adds her power to yours!\x01"
        f"That's {global_count(GlobalCount.MEDALLION)} medallion(s), {global_count(GlobalCount.REWARD)} reward(s)!",
    ),
    (
        0x003F,
        f"\x13\x69You received the {yellow('Spirit', reset=None)}\x01"
        f"{yellow('Medallion')}! Nabooru awakens as a\x01"
        f"Sage and adds her power to yours!\x01"
        f"That's {global_count(GlobalCount.MEDALLION)} medallion(s), {global_count(GlobalCount.REWARD)} reward(s)!",
    ),
    (
        0x0040,
        f"\x13\x6BYou received the {light_blue('Light', reset=None)}\x01"
        f"{light_blue('Medallion')}! Rauru the Sage adds\x01"
        f"his power to yours!\x01"
        f"That's {global_count(GlobalCount.MEDALLION)} medallion(s), {global_count(GlobalCount.REWARD)} reward(s)!",
    ),
    (
        0x0041,
        f"\x13\x6AYou received the {pink('Shadow', reset=None)}\x01"
        f"{pink('Medallion')}! Impa awakens as a\x01"
        f"Sage and adds her power to yours!\x01"
        f"That's {global_count(GlobalCount.MEDALLION)} medallion(s), {global_count(GlobalCount.REWARD)} reward(s)!",
    ),
    (
        0x0048,
        f"\x13\x10You got a {red('Magic Bean')}!\x01"
        f"Find a suitable spot for a garden\x01"
        f"and plant it.\x01"
        f"That's {global_count(GlobalCount.BEANS_PLANTED)} magic bean(s)!",
    ),
    (
        0x004A,
        f"\x13\x07You received the {red('Fairy Ocarina')}!\x01"
        f"This is a memento from Saria.",
    ),
    (
        0x004B,
        f"""\x13\x3DYou got the {green("Giant's Knife")}!\x01"""
        f"""Hold it with both hands to attack!\x01"""
        f"""It's too long to use with a {light_blue('shield')}.\x01"""
        f"""That's {global_count(GlobalCount.SWORD)} sword(s)!""",
    ),
    (
        0x004C,
        f"\x13\x3EYou got a {light_blue('Deku Shield')}!\x01"
        f"That's {global_count(GlobalCount.SHIELD)} shield(s)!",
    ),
    (
        0x004D,
        f"\x13\x3FYou got a {light_blue('Hylian Shield')}!\x01"
        f"That's {global_count(GlobalCount.SHIELD)} shield(s)!",
    ),
    (
        0x004E,
        f"\x13\x40You found the {light_blue('Mirror Shield')}!\x01"
        f"The shield's polished surface can\x01"
        f"reflect light or energy.\x01"
        f"That's {global_count(GlobalCount.SHIELD)} shield(s)!",
    ),
    (
        0x0050,
        f"\x13\x42You got a {red('Goron Tunic')}!\x01"
        f"Going to a hot place? No worry!\x01"
        f"That's {global_count(GlobalCount.TUNIC)} tunic(s)!",
    ),
    (
        0x0051,
        f"\x13\x43You got a {blue('Zora Tunic')}!\x01"
        f"Wear it, and you won't drown\x01"
        f"underwater.\x01"
        f"That's {global_count(GlobalCount.TUNIC)} tunic(s)!",
    ),
    (
        0x0053,
        f"\x13\x45You got the {red('Iron Boots')}!\x01"
        f"So heavy, you can't run.\x01"
        f"So heavy, you can't float.\x01"
        f"That's {global_count(GlobalCount.BOOT)} boot(s)!",
    ),
    (
        0x0054,
        f"\x13\x46You got the {red('Hover Boots')}!\x01"
        f"With these mysterious boots\x01"
        f"you can hover above the ground.\x01"
        f"That's {global_count(GlobalCount.BOOT)} boot(s)!",
    ),
    (
        0x0058,
        f"\x13\x4DYou found a {red('Bomb Bag')}!\x01"
        f"You found {red('20 Bombs')} inside!",
    ),
    (
        0x005B,
        f"\x13\x51You found the {blue('Silver Gauntlets')}!\x01"
        f"You feel the power to lift\x01"
        f"big things with it!",
    ),
    (
        0x0066,
        f"\x13\x76You found the {red('Dungeon Map')}!\x01"
        f"It's the map to this dungeon.\x01"
        f"That's {global_count(GlobalCount.MAP)} map(s)!",
    ),
    (
        0x0067,
        f"\x13\x75You found the {red('Compass')}!\x01"
        f"Now you can see many hidden\x01"
        f"things in this dungeon!\x01"
        f"That's {global_count(GlobalCount.COMPASS)} compass(es)!",
    ),
    (
        0x0069,
        f"""\x13\x23You received {red("Zelda's Letter")}!\x01"""
        f"""Wow! This letter has Princess\x01"""
        f"""Zelda's autograph!""",
    ),
    (
        0x0070,
        f"\x13\x04"
        f"You got the {red('Fire Arrow')}!\x01"
        f"If you hit your target,\x01"
        f"it will catch fire.\x01"
        f"That's {global_count(GlobalCount.ELEMENTAL_ARROW)} elemental arrow(s)!",
    ),
    (
        0x0071,
        f"\x13\x0CYou got the {blue('Ice Arrow')}!\x01"
        f"If you hit your target,\x01"
        f"it will freeze.\x01"
        f"That's {global_count(GlobalCount.ELEMENTAL_ARROW)} elemental arrow(s)!",
    ),
    (
        0x0072,
        f"\x13\x12You got the {light_blue('Light Arrow')}!\x01"
        f"The light of justice\x01"
        f"will smite evil!\x01"
        f"That's {global_count(GlobalCount.ELEMENTAL_ARROW)} elemental arrow(s)!",
    ),
    (
        0x0073,
        f"\x06\x28You have learned the\x01"
        f"\x06\x2F{green('Minuet of Forest')}!\x01"
        f"\x06\x28That's {global_count(GlobalCount.SONG)} song(s)!",
    ),
    (
        0x0074,
        f"\x06\x28You have learned the\x01"
        f"\x06\x37{red('Bolero of Fire')}!\x01"
        f"\x06\x28That's {global_count(GlobalCount.SONG)} song(s)!",
    ),
    (
        0x0075,
        f"\x06\x28You have learned the\x01"
        f"\x06\x29{blue('Serenade of Water')}!\x01"
        f"\x06\x28That's {global_count(GlobalCount.SONG)} song(s)!",
    ),
    (
        0x0076,
        f"\x06\x28You have learned the\x01"
        f"\x06\x2D{yellow('Requiem of Spirit')}!\x01"
        f"\x06\x28That's {global_count(GlobalCount.SONG)} song(s)!",
    ),
    (
        0x0077,
        f"\x06\x28You have learned the\x01"
        f"\x06\x28{pink('Nocturne of Shadow')}!\x01"
        f"\x06\x28That's {global_count(GlobalCount.SONG)} song(s)!",
    ),
    (
        0x0078,
        f"\x06\x28You have learned the\x01"
        f"\x06\x32{light_blue('Prelude of Light')}!\x01"
        f"\x06\x28That's {global_count(GlobalCount.SONG)} song(s)!",
    ),
    (
        0x0080,
        f"""\x13\x6CYou got the {green("Kokiri's Emerald")}!\x01"""
        f"""It's the Spiritual Stone of Forest,\x01"""
        f"""passed down by the Great Deku Tree.\x01"""
        f"""That's {global_count(GlobalCount.STONE)} stone(s), {global_count(GlobalCount.REWARD)} reward(s)!""",
    ),
    (
        0x0081,
        f"""\x13\x6DYou obtained the {red("Goron's Ruby")}!\x01"""
        f"""This is the Spiritual Stone of \x01"""
        f"""Fire passed down by the Gorons!\x01"""
        f"""That's {global_count(GlobalCount.STONE)} stone(s) and {global_count(GlobalCount.REWARD)} reward(s)!""",
    ),
    (
        0x0082,
        f"""\x13\x6EYou obtained {blue("Zora's Sapphire")}!\x01"""
        f"""It's the Spiritual Stone of Water,\x01"""
        f"""passed down by the Zoras!\x01"""
        f"""That's {global_count(GlobalCount.STONE)} stone(s), {global_count(GlobalCount.REWARD)} reward(s)!""",
    ),
    (
        0x0098,
        f"\x13\x1AYou got {red('Lon Lon Milk')}!\x01"
        f"This milk is very nutritious!\x01"
        f"There are two drinks in it.",
    ),
    (
        0x0099,
        f"\x13\x1BYou got an {red('Empty Bottle')}!\x01"
        f"Put something inside... What?\x01"
        f"Something's already inside!?\x01"
        f"That's {global_count(GlobalCount.BOTTLE_SLOT)} bottle slot(s)!",
    ),
    (
        0x009A,
        f"\x13\x21You got a {red('Weird Egg')}!\x01"
        f"Feels like there's something\x01"
        f"moving inside!",
    ),
    (
        0x00A4,
        f"\x13\x3BYou got the {green('Kokiri Sword')}!\x01"
        f"This is a hidden treasure of\x01"
        f"the Kokiri.\x01"
        f"That's {global_count(GlobalCount.SWORD)} sword(s)!",
    ),
    (
        0x00AD,
        f"""\x13\x05You got {red("Din's Fire")}!\x01"""
        f"""Its fireball engulfs everything!\x01"""
        f"""That's {global_count(GlobalCount.FAIRY_SPELL)} fairy spell(s)!""",
    ),
    (
        0x00AE,
        f"""\x13\x0DYou got {green("Farore's Wind")}!\x01"""
        f"""This is warp magic you can use!\x01"""
        f"""That's {global_count(GlobalCount.FAIRY_SPELL)} fairy spell(s)!""",
    ),
    (
        0x00AF,
        f"""\x13\x13You got {blue("Nayru's Love")}!\x01"""
        f"""Cast this to create a powerful\x01"""
        f"""protective barrier.\x01"""
        f"""That's {global_count(GlobalCount.FAIRY_SPELL)} fairy spell(s)!""",
    ),
    (
        0x00C7,
        f"\x13\x74You got the {red('Boss Key')}!\x01"
        f"Now you can get inside the \x01"
        f"chamber where the Boss lurks.\x01"
        f"That's {global_count(GlobalCount.BOSS_KEY)} boss key(s)!",
    ),
    (
        0x00D1,
        f"""\x06\x14You've learned {green("Saria's Song")}!\x01"""
        f"""\x06\x14That's {global_count(GlobalCount.SONG)} song(s)!""",
    ),
    (
        0x00D2,
        f"""\x06\x11You've learned {red("Epona's Song")}!\x01"""
        f"""\x06\x11That's {global_count(GlobalCount.SONG)} song(s)!""",
    ),
    (
        0x00D3,
        f"""\x06\x0BYou've learned the {yellow("Sun's Song")}!\x01"""
        f"""\x06\x0BThat's {global_count(GlobalCount.SONG)} song(s)!""",
    ),
    (
        0x00D4,
        f"""\x06\x15You've learned {blue("Zelda's Lullaby")}!\x01"""
        f"""\x06\x15That's {global_count(GlobalCount.SONG)} song(s)!""",
    ),
    (
        0x00D5,
        f"\x06\x05You've learned the {light_blue('Song of Time')}!\x01"
        f"\x06\x05That's {global_count(GlobalCount.SONG)} song(s)!",
    ),
    (
        0x00D6,
        f"You've learned the {pink('Song of Storms')}!\x01"
        f"That's {global_count(GlobalCount.SONG)} song(s)!",
    ),
    (
        0x00DD,
        f"You mastered the secret sword\x01"
        f"technique of the {red('Spin Attack')}!",
    ),
    (
        0x00F2,
        f"You got a {yellow('Huge Rupee')}!\x01"
        f"This Rupee is worth a whopping\x01"
        f"{yellow('two hundred Rupees')}!\x01"
        f"That's {global_count(GlobalCount.GOLD_RUPEE)} gold rupee chests!",
    ),
]

DUNGEON_MESSAGE_LOCATIONS: dict[Dungeon, str] = {
    Dungeon.DEKU_TREE: f"the {green('Deku Tree')}",
    Dungeon.DODONGOS_CAVERN: red("Dodongo's Cavern"),
    Dungeon.JABU_JABUS_BELLY: blue("Jabu Jabu's Belly"),
    Dungeon.FOREST_TEMPLE: f"the {green('Forest Temple')}",
    Dungeon.FIRE_TEMPLE: f"the {red('Fire Temple')}",
    Dungeon.WATER_TEMPLE: f"the {blue('Water Temple')}",
    Dungeon.SPIRIT_TEMPLE: f"the {yellow('Spirit Temple')}",
    Dungeon.SHADOW_TEMPLE: f"the {pink('Shadow Temple')}",
    Dungeon.BOTTOM_OF_THE_WELL: f"the {pink('Bottom of the Well')}",
    Dungeon.ICE_CAVERN: f"the {light_blue('Ice Cavern')}",
    Dungeon.GERUDO_TRAINING_GROUND: f"the {yellow('Gerudo Training', reset=None)}\x01{yellow('Ground')}",
    Dungeon.THIEVES_HIDEOUT: "the " + yellow("Thieves' Hideout"),
    Dungeon.GANONS_CASTLE: red("Ganon's Castle"),
}

BOSS_KEY_DUNGEONS: tuple[Dungeon, ...] = (
    Dungeon.FOREST_TEMPLE,
    Dungeon.FIRE_TEMPLE,
    Dungeon.WATER_TEMPLE,
    Dungeon.SPIRIT_TEMPLE,
    Dungeon.SHADOW_TEMPLE,
    Dungeon.GANONS_CASTLE,
)

COMPASS_DUNGEONS: tuple[Dungeon, ...] = (
    Dungeon.DEKU_TREE,
    Dungeon.DODONGOS_CAVERN,
    Dungeon.JABU_JABUS_BELLY,
    Dungeon.FOREST_TEMPLE,
    Dungeon.FIRE_TEMPLE,
    Dungeon.WATER_TEMPLE,
    Dungeon.SPIRIT_TEMPLE,
    Dungeon.SHADOW_TEMPLE,
    Dungeon.BOTTOM_OF_THE_WELL,
    Dungeon.ICE_CAVERN,
)

MAP_DUNGEONS: tuple[Dungeon, ...] = (
    Dungeon.DEKU_TREE,
    Dungeon.DODONGOS_CAVERN,
    Dungeon.JABU_JABUS_BELLY,
    Dungeon.FOREST_TEMPLE,
    Dungeon.FIRE_TEMPLE,
    Dungeon.WATER_TEMPLE,
    Dungeon.SPIRIT_TEMPLE,
    Dungeon.SHADOW_TEMPLE,
    Dungeon.BOTTOM_OF_THE_WELL,
    Dungeon.ICE_CAVERN,
)

SMALL_KEY_DUNGEONS: tuple[Dungeon, ...] = (
    Dungeon.FOREST_TEMPLE,
    Dungeon.FIRE_TEMPLE,
    Dungeon.WATER_TEMPLE,
    Dungeon.SPIRIT_TEMPLE,
    Dungeon.SHADOW_TEMPLE,
    Dungeon.BOTTOM_OF_THE_WELL,
    Dungeon.GERUDO_TRAINING_GROUND,
    Dungeon.THIEVES_HIDEOUT,
    Dungeon.GANONS_CASTLE,
)

LOCATION_COUNT_MESSAGES: list[tuple[int, str]] = [
    *[
        (
            0x9101 + dungeon,
            f"You got a {red('Gold Skulltula Token')}!\x01"
            f"That's {dungeon_count(dungeon, DungeonCount.TOKEN)} token(s)\x01"
            f"for {colored(name, color)}.\x01"
            f"That's {global_count(GlobalCount.TOKEN)} token(s) total!",
        )
        for dungeon, name, color in DUNGEON_TOKEN_MESSAGE_INFO
    ],
    *[
        (
            0x910B + area,
            f"You got a {red('Gold Skulltula Token')}!\x01"
            f"That's {area_count(area, AreaCount.TOKEN)} token(s)\x01"
            f"for {colored(name, color)}.\x01"
            f"That's {global_count(GlobalCount.TOKEN)} token(s) total!",
        )
        for area, name, color in AREA_MESSAGE_INFO
    ],
    *[
        (
            0x9117 + area,
            f"\x13\x73You got a {red('Piece of Heart')}!\x01"
            f"That's {area_count(area, AreaCount.HEART_PIECE)} heart piece(s)\x01"
            f"for {colored(name, color)}.",
        )
        for area, name, color in AREA_MESSAGE_INFO
    ],
    (
        0x9128,
        f"\x06\x49{red('WINNER')}!\x04\x13\x73"
        f"You got a {red('Piece of Heart')}!\x01"
        f"That's {area_count(Area.MARKET, AreaCount.HEART_PIECE)} heart piece(s)\x01"
        f"for {colored('Market Area', MessageColor.LIGHT_BLUE)}.",
    ),
    (
        0x912F,
        f"\x13\x73You got a {red('Piece of Heart')}!\x01"
        f"That's {dungeon_count(Dungeon.ICE_CAVERN, DungeonCount.HEART_PIECE)} heart piece(s)\x01"
        f"for {colored('Ice Cavern', MessageColor.BLUE)}.",
    ),
    *[
        (
            0x9200 + index,
            f"\x13\x74You got the {red('Boss Key')}\x01"
            f"for {DUNGEON_MESSAGE_LOCATIONS[dungeon]}!\x01"
            f"That's {global_count(GlobalCount.BOSS_KEY)} boss key(s)!",
        )
        for index, dungeon in enumerate(BOSS_KEY_DUNGEONS)
    ],
    *[
        (
            0x9206 + index,
            f"\x13\x75You found the {red('Compass')}\x01"
            f"for {DUNGEON_MESSAGE_LOCATIONS[dungeon]}!\x01"
            f"That's {global_count(GlobalCount.COMPASS)} compass(es)!",
        )
        for index, dungeon in enumerate(COMPASS_DUNGEONS)
    ],
    *[
        (
            0x9210 + index,
            f"\x13\x76You found the {red('Dungeon Map')}\x01"
            f"for {DUNGEON_MESSAGE_LOCATIONS[dungeon]}!\x01"
            f"That's {global_count(GlobalCount.MAP)} map(s)!",
        )
        for index, dungeon in enumerate(MAP_DUNGEONS)
    ],
    *[
        (
            0x921A + index,
            f"\x13\x77You found a {red('Small Key')}\x01"
            f"for {DUNGEON_MESSAGE_LOCATIONS[dungeon]}!\x01"
            f"That's {dungeon_count(dungeon, DungeonCount.SMALL_KEY)} small key(s)\x01"
            f"for this dungeon.",
        )
        for index, dungeon in enumerate(SMALL_KEY_DUNGEONS)
    ],
]


# convert byte array to an integer
def bytes_to_int(data: bytes, signed: bool = False) -> int:
    return int.from_bytes(data, byteorder='big', signed=signed)


# convert int to an array of bytes of the given width
def int_to_bytes(num: int, width: int, signed: bool = False) -> bytes:
    return int.to_bytes(num, width, byteorder='big', signed=signed)


def display_code_list(codes: list[TextCode]) -> str:
    message = ""
    for code in codes:
        message += str(code)
    return message


def encode_text_string(text: str) -> list[int]:
    result = []
    it = iter(text)
    for ch in it:
        n = ord(ch)
        if n in CONTROL_CODES:
            result.append(n)
            for _ in range(CONTROL_CODES[n][1]):
                result.append(ord(next(it)))
            continue
        mapped = CHARACTER_MAP.get(ch)
        if mapped:
            result.append(mapped)
            continue
        if n in CHARACTER_MAP.values(): # Character has already been translated
            result.append(n)
            continue
        raise ValueError(f"While encoding {text!r}: Unable to translate unicode character {ch!r} ({n}).  (Already decoded: {result!r})")
    return result


def parse_control_codes(text: list[int] | bytearray | str) -> list[TextCode]:
    if isinstance(text, list):
        text_bytes = text
    elif isinstance(text, bytearray):
        text_bytes = list(text)
    else:
        text_bytes = encode_text_string(text)

    text_codes = []
    index = 0
    while index < len(text_bytes):
        next_char = text_bytes[index]
        data = 0
        index += 1
        if next_char in CONTROL_CODES:
            extra_bytes = CONTROL_CODES[next_char][1]
            if extra_bytes > 0:
                data = bytes_to_int(text_bytes[index: index + extra_bytes])
                index += extra_bytes
        text_code = TextCode(next_char, data)
        text_codes.append(text_code)
        if text_code.code == 0x02:  # message end code
            break

    return text_codes


# holds a single character or control code of a string
class TextCode:
    def __init__(self, code: int, data: int) -> None:
        self.code: int = code
        if code in CONTROL_CODES:
            self.type = CONTROL_CODES[code][0]
        else:
            self.type = 'character'
        self.data: int = data

    def display(self) -> str:
        if self.code in CONTROL_CODES:
            return CONTROL_CODES[self.code][2](self.data)
        elif self.code in SPECIAL_CHARACTERS:
            return SPECIAL_CHARACTERS[self.code]
        elif self.code >= 0x7F:
            return '?'
        else:
            return chr(self.code)

    def get_string(self) -> str:
        if self.code in CONTROL_CODES:
            ret = ''
            subdata = self.data
            for _ in range(0, CONTROL_CODES[self.code][1]):
                ret = chr(subdata & 0xFF) + ret
                subdata = subdata >> 8
            ret = chr(self.code) + ret
            return ret
        else:
            # raise ValueError(repr(REVERSE_MAP))
            return REVERSE_MAP[self.code]

    def size(self) -> int:
        size = 1
        if self.code in CONTROL_CODES:
            size += CONTROL_CODES[self.code][1]
        return size

    # writes the code to the given offset, and returns the offset of the next byte
    def write(self, rom: Rom, text_start: int, offset: int) -> int:
        rom.write_byte(text_start + offset, self.code)

        extra_bytes = 0
        if self.code in CONTROL_CODES:
            extra_bytes = CONTROL_CODES[self.code][1]
            bytes_to_write = int_to_bytes(self.data, extra_bytes)
            rom.write_bytes(text_start + offset + 1, bytes_to_write)

        return offset + 1 + extra_bytes

    __str__ = __repr__ = display


# holds a single message, and all its data
class Message:
    def __init__(self, raw_text: list[int] | bytearray | str, index: int, id: int, opts: int, offset: int, length: int) -> None:
        if isinstance(raw_text, str):
            raw_text = encode_text_string(raw_text)
        elif not isinstance(raw_text, bytearray):
            raw_text = bytearray(raw_text)

        self.raw_text: bytearray = raw_text

        self.index: int = index
        self.id: int = id
        self.opts: int = opts  # Textbox type and y position
        self.box_type: int = (self.opts & 0xF0) >> 4
        self.position: int = (self.opts & 0x0F)
        self.offset: int = offset
        self.length: int = length

        self.has_goto: bool = False
        self.has_keep_open: bool = False
        self.has_event: bool = False
        self.has_fade: bool = False
        self.has_ocarina: bool = False
        self.has_two_choice: bool = False
        self.has_three_choice: bool = False
        self.ending: Optional[TextCode] = None

        self.text_codes: list[TextCode] = []
        self.text: str = ''
        self.unpadded_length: int = 0
        self.parse_text()

    def display(self) -> str:
        meta_data = [
            "#" + str(self.index),
            "ID: 0x" + "{:04x}".format(self.id),
            "Offset: 0x" + "{:06x}".format(self.offset),
            "Length: 0x" + "{:04x}".format(self.unpadded_length) + "/0x" + "{:04x}".format(self.length),
            "Box Type: " + str(self.box_type),
            "Postion: " + str(self.position)
        ]
        return ', '.join(meta_data) + '\n' + self.text

    def parse_text(self) -> None:
        self.text_codes = parse_control_codes(self.raw_text)

        index = 0
        for text_code in self.text_codes:
            index += text_code.size()
            if text_code.code == 0x02:  # message end code
                break
            if text_code.code == 0x07:  # goto
                self.has_goto = True
                self.ending = text_code
            if text_code.code == 0x0A:  # keep-open
                self.has_keep_open = True
                self.ending = text_code
            if text_code.code == 0x0B:  # event
                self.has_event = True
                self.ending = text_code
            if text_code.code == 0x0E:  # fade out
                self.has_fade = True
                self.ending = text_code
            if text_code.code == 0x10:  # ocarina
                self.has_ocarina = True
                self.ending = text_code
            if text_code.code == 0x1B:  # two choice
                self.has_two_choice = True
            if text_code.code == 0x1C:  # three choice
                self.has_three_choice = True
        self.text = display_code_list(self.text_codes)
        self.unpadded_length = index

    # computes the size of a message, including padding
    def size(self) -> int:
        size = 0

        for code in self.text_codes:
            size += code.size()

        size = (size + 3) & -4 # align to nearest 4 bytes

        return size

    # applies whatever transformations we want to the dialogs
    def transform(self, replace_ending: bool = False, ending: Optional[TextCode] = None,
                  always_allow_skip: bool = True, speed_up_text: bool = True) -> None:
        ending_codes = [0x02, 0x07, 0x0A, 0x0B, 0x0E, 0x10]
        box_breaks = [0x04, 0x0C]
        slows_text = [0x08, 0x09, 0x14]
        slow_icons = [0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x04, 0x02]

        text_codes = []
        instant_text_code = TextCode(0x08, 0)

        # # speed the text
        if speed_up_text:
            text_codes.append(instant_text_code) # allow instant

        # write the message
        for code in self.text_codes:
            # ignore ending codes if it's going to be replaced
            if replace_ending and code.code in ending_codes:
                pass
            # ignore the "make unskippable flag"
            elif always_allow_skip and code.code == 0x1A:
                pass
            # ignore anything that slows down text
            elif speed_up_text and code.code in slows_text:
                pass
            elif speed_up_text and code.code in box_breaks:
                # some special cases for text that needs to be on a timer
                if (self.id == 0x605A or  # twinrova transformation
                    self.id == 0x706C or  # rauru ending text
                    self.id == 0x70DD or  # ganondorf ending text
                    self.id in (0x706F, 0x7091, 0x7092, 0x7093, 0x7094, 0x7095, 0x7070)  # zelda ending text
                ):
                    text_codes.append(code)
                    text_codes.append(instant_text_code)  # allow instant
                else:
                    text_codes.append(TextCode(0x04, 0))  # un-delayed break
                    text_codes.append(instant_text_code)  # allow instant
            elif speed_up_text and code.code == 0x13 and code.data in slow_icons:
                text_codes.append(code)
                text_codes.pop(find_last(text_codes, instant_text_code))  # remove last instance of instant text
                text_codes.append(instant_text_code)  # allow instant
            else:
                text_codes.append(code)

        if replace_ending:
            if ending:
                if speed_up_text and ending.code == 0x10:  # ocarina
                    text_codes.append(TextCode(0x09, 0))  # disallow instant text
                text_codes.append(ending)  # write special ending
            text_codes.append(TextCode(0x02, 0))  # write end code

        self.text_codes = text_codes

    # writes a Message back into the rom, using the given index and offset to update the table
    # returns the offset of the next message
    def write(self, rom: Rom, index: int, text_start: int, offset: int, bank: int) -> int:
        # construct the table entry
        id_bytes = int_to_bytes(self.id, 2)
        offset_bytes = int_to_bytes(offset, 3)
        entry = id_bytes + bytes([self.opts, 0x00, bank]) + offset_bytes
        # write it back
        entry_offset = EXTENDED_TABLE_START + 8 * index
        rom.write_bytes(entry_offset, entry)

        for code in self.text_codes:
            offset = code.write(rom, text_start, offset)

        while offset % 4 > 0:
            offset = TextCode(0x00, 0).write(rom, text_start, offset) # pad to 4 byte align

        return offset

    # read a single message from rom
    @classmethod
    def from_rom(cls, rom: Rom, index: int, eng: bool = True) -> Message:
        if eng:
            table_start = ENG_TABLE_START
            text_start = ENG_TEXT_START
        else:
            table_start = JPN_TABLE_START
            text_start = JPN_TEXT_START
        entry_offset = table_start + 8 * index
        entry = rom.read_bytes(entry_offset, 8)
        next = rom.read_bytes(entry_offset + 8, 8)

        id = bytes_to_int(entry[0:2])
        opts = entry[2]
        offset = bytes_to_int(entry[5:8])
        length = bytes_to_int(next[5:8]) - offset

        raw_text = rom.read_bytes(text_start + offset, length)

        return cls(raw_text, index, id, opts, offset, length)

    @classmethod
    def from_string(cls, text: str, id: int = 0, opts: int = 0x00) -> Message:
        bytes = text + "\x02"
        return cls(bytes, 0, id, opts, 0, len(bytes) + 1)

    @classmethod
    def from_bytearray(cls, text: bytearray, id: int = 0, opts: int = 0x00) -> Message:
        bytes = list(text) + [0x02]
        return cls(bytes, 0, id, opts, 0, len(bytes) + 1)

    __str__ = __repr__ = display


# wrapper for updating the text of a message, given its message id
# if the id does not exist in the list, then it will add it
# Checks if the message being updated is a newly added message in order to prevent duplicates.
# Use allow_duplicates=True if the same message is purposely updated multiple times
def update_message_by_id(messages: list[Message], id: int, text: bytearray | str, opts: Optional[int] = None, allow_duplicates: bool = False):
    # Check is we have previously added/modified this message.
    if id in new_messages and not allow_duplicates:
        raise Exception(f'Attempting to add duplicate message {hex(id)}')

    new_messages.append(id)
    # get the message index
    index = next( (m.index for m in messages if m.id == id), -1)
    # update if it was found
    if index >= 0:
        update_message_by_index(messages, index, text, opts)
    else:
        add_message(messages, text, id, opts)


# wrapper for updating the text of a message, given its index in the list
def update_message_by_index(messages: list[Message], index: int, text: bytearray | str, opts: Optional[int] = None) -> None:
    if opts is None:
        opts = messages[index].opts

    if isinstance(text, bytearray):
        messages[index] = Message.from_bytearray(text, messages[index].id, opts)
    else:
        messages[index] = Message.from_string(text, messages[index].id, opts)
    messages[index].index = index


# wrapper for adding a string message to a list of messages
def add_message(messages: list[Message], text: bytearray | str, id: int = 0, opts: int = 0x00) -> None:
    if isinstance(text, bytearray):
        messages.append(Message.from_bytearray(text, id, opts))
    else:
        messages.append(Message.from_string(text, id, opts))
    messages[-1].index = len(messages) - 1


# reduce item message sizes and add new item messages
def update_item_messages(messages: list[Message]) -> None:
    for id, text in ITEM_MESSAGES:
        update_message_by_id(messages, id, text, 0x23)

    for id, text in LOCATION_COUNT_MESSAGES:
        update_message_by_id(messages, id, text, 0x23)

# run all keysanity related patching to add messages for dungeon specific items
def add_item_messages(messages: list[Message]) -> None:
    update_item_messages(messages)


# reads each of the game's messages into a list of Message objects
def read_messages(rom: Rom) -> list[Message]:
    table_offset = ENG_TABLE_START
    index = 0
    messages = []
    while True:
        entry = rom.read_bytes(table_offset, 8)
        id = bytes_to_int(entry[0:2])

        if id == 0xFFFD:
            table_offset += 8
            continue # this is only here to give an ending offset
        if id == 0xFFFF:
            break # this marks the end of the table

        messages.append(Message.from_rom(rom, index))

        index += 1
        table_offset += 8

    # Also grab 0xFFFC entry from JP table.
    messages.append(read_fffc_message(rom))
    return messages


# The JP text table is the only source for ID 0xFFFC, which is used by the
# title and file select screens. Preserve this table entry and text data when
# overwriting the JP data. The regular read_messages function only reads English
# data.
def read_fffc_message(rom: Rom) -> Message:
    table_offset = JPN_TABLE_START
    index = 0
    while True:
        entry = rom.read_bytes(table_offset, 8)
        id = bytes_to_int(entry[0:2])

        if id == 0xFFFC:
            message = Message.from_rom(rom, index, eng=False)
            break

        index += 1
        table_offset += 8

    return message


# write the messages back
def repack_messages(rom: Rom, messages: list[Message], permutation: Optional[list[int]] = None,
                    always_allow_skip: bool = True, speed_up_text: bool = True) -> None:
    rom.update_dmadata_record(ENG_TEXT_START, ENG_TEXT_START, ENG_TEXT_START + ENG_TEXT_SIZE_LIMIT)
    rom.update_dmadata_record(JPN_TEXT_START, JPN_TEXT_START, JPN_TEXT_START + JPN_TEXT_SIZE_LIMIT)

    if permutation is None:
        permutation = range(len(messages))

    # repack messages
    offset = 0
    text_start = JPN_TEXT_START
    text_size_limit = EXTENDED_TEXT_SIZE_LIMIT
    text_bank = 0x08 # start with the Japanese text bank
    jp_bytes = 0
    # An extra dummy message is inserted after exhausting the JP text file.
    # Written message IDs are independent of the python list index, but the
    # index has to be maintained for old/new lookups. This wouldn't be an
    # issue if text shuffle didn't exist.
    jp_index_offset = 0

    for old_index, new_index in enumerate(permutation):
        old_message = messages[old_index]
        new_message = messages[new_index]
        remember_id = new_message.id
        new_message.id = old_message.id

        # modify message, making it represent how we want it to be written
        if new_message.id != 0xFFFC:
            new_message.transform(True, old_message.ending, always_allow_skip, speed_up_text)

        # check if there is space to write the message
        message_size = new_message.size()
        if message_size + offset > JPN_TEXT_SIZE_LIMIT and text_start == JPN_TEXT_START:
            # Add a dummy entry to the table for the last entry in the
            # JP file. This is used by the game to calculate message
            # length. Since the next entry in the English table has an
            # offset of zero, which would lead to a negative length.
            # 0xFFFD is used as the text ID for this in vanilla.
            # Text IDs need to be in order across the table for the
            # split to work.
            entry = bytes([0xFF, 0xFD, 0x00, 0x00, text_bank]) + int_to_bytes(offset, 3)
            entry_offset = EXTENDED_TABLE_START + 8 * old_index
            rom.write_bytes(entry_offset, entry)
            # if there is no room then switch to the English text bank
            text_bank = 0x07
            text_start = ENG_TEXT_START
            jp_bytes = offset
            jp_index_offset = 1
            offset = 0

        # Special handling for text ID 0xFFFC, which has hard-coded offsets to
        # the JP file in function Font_LoadOrderedFont in z_kanfont.c
        if new_message.id == 0xFFFC:
            # hard-coded offset including segment
            rom.write_int16(0xAD1CE2, (text_bank << 8) + ((offset & 0xFFFF0000) >> 16) + (1 if offset & 0xFFFF > 0x8000 else 0))
            rom.write_int16(0xAD1CE6, offset & 0XFFFF)
            # hard-coded message length, represented by offset of end of message
            rom.write_int16(0xAD1D16, (text_bank << 8) + (((offset + new_message.size()) & 0xFFFF0000) >> 16) + (1 if (offset + new_message.size()) & 0xFFFF > 0x8000 else 0))
            rom.write_int16(0xAD1D1E, (offset + new_message.size()) & 0XFFFF)
            # hard-coded segment, default JP file (0x08)
            rom.write_int16(0xAD1D12, (text_bank << 8))
            # hard-coded text file start address in rom, default JP
            rom.write_int16(0xAD1D22, ((text_start & 0xFFFF0000) >> 16) + (1 if text_start & 0xFFFF > 0x8000 else 0))
            rom.write_int16(0xAD1D2E, text_start & 0XFFFF)

        # actually write the message
        offset = new_message.write(rom, old_index + jp_index_offset, text_start, offset, text_bank)

        new_message.id = remember_id

    # raise an exception if too much is written
    # we raise it at the end so that we know how much overflow there is
    if jp_bytes + offset > text_size_limit:
        raise(TypeError("Message Text table is too large: 0x" + "{:x}".format(jp_bytes + offset) + " written / 0x" + "{:x}".format(EXTENDED_TEXT_SIZE_LIMIT) + " allowed."))

    # end the table, accounting for additional entry for file split
    table_index = len(messages) + (1 if text_bank == 0x07 else 0)
    entry = bytes([0xFF, 0xFD, 0x00, 0x00, text_bank]) + int_to_bytes(offset, 3)
    entry_offset = EXTENDED_TABLE_START + 8 * table_index
    rom.write_bytes(entry_offset, entry)
    table_index += 1
    entry_offset = EXTENDED_TABLE_START + 8 * table_index
    if 8 * (table_index + 1) > EXTENDED_TABLE_SIZE:
        raise(TypeError("Message ID table is too large: 0x" + "{:x}".format(8 * (table_index + 1)) + " written / 0x" + "{:x}".format(EXTENDED_TABLE_SIZE) + " allowed."))
    rom.write_bytes(entry_offset, [0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00])
