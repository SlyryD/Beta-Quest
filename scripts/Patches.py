import random
import struct
import itertools
import re
import zlib
from collections import defaultdict

from Rom import Rom
from Messages import read_messages, update_message_by_id, read_shop_items, update_warp_song_text, \
        write_shop_items, remove_unused_messages, \
        add_item_messages, update_item_messages, repack_messages, shuffle_messages, \
        get_message_by_id
from MQ import patch_files
from Saveflags import default_save_flags


def patch_rom(rom:Rom, options):
    if options['mq_enabled']:
        mq_scenes = [
            0, # Deku Tree
            1, # Dodongos Cavern
            2, # Jabu Jabus Belly
            3, # Forest Temple
            4, # Fire Temple
            5, # Water Temple
            6, # Spirit Temple
            7, # Shadow Temple
            8, # Bottom of the Well
            9, # Ice Cavern
            11, # Gerudo Training Ground
            13 # Ganons Castle
        ]
        patch_files(rom, mq_scenes)

    # Load Message and Shop Data
    messages = read_messages(rom)
    update_item_messages(messages)
    repack_messages(rom, messages)

    # Set save file flags
    default_save_flags(rom)

    # Set default hold targetting
    rom.write_byte(0xB71E6D, 0x01)

    rom.write_int32(rom.sym('CFG_DUNGEON_INFO_ENABLE'), 1)
    rom.write_int32(rom.sym('CFG_WORLD_MAP_INFO_ENABLE'), 1)
