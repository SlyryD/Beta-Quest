import random
import struct
import itertools
import re
import zlib
from collections import defaultdict

from Patches.Rom import Rom
from Patches.Messages import read_messages, update_message_by_id, read_shop_items, update_warp_song_text, \
        write_shop_items, remove_unused_messages, \
        add_item_messages, update_item_messages, repack_messages, shuffle_messages, \
        get_message_by_id
from Patches.Saveflags import default_save_flags


def patch_rom(rom:Rom):
    
    # Load Message and Shop Data
    messages = read_messages(rom)
    update_item_messages(messages)
    repack_messages(rom, messages)

    # Set save file flags
    default_save_flags(rom)

    # Set default hold targetting
    rom.write_byte(0xB71E6D, 0x01)

    rom.write_int32(rom.sym('CFG_DUNGEON_INFO_ENABLE'), 1)
