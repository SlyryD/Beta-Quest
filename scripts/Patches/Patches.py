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

    #write title screen
    #patch_title_screen(rom, "resources/title_diff.bin", "resources/copyright_ia8.bin")

    # Set default hold targetting
    rom.write_byte(0xB71E6D, 0x01)

def diff(arr1, arr2):
	return bytearray([a ^ b for a, b in zip(arr1, arr2 )])


def patch_title_screen(rom:Rom, title_patch, copyright_file):
	title_address = 0x1795300
	copyright_address = 0x17AE300
	diff_logo_file = open(title_patch, 'rb')
	diff_bytes = diff_logo_file.read()
	diff_logo_file.close()
	f_copyright = open(copyright_file, 'rb')
	copyright_bytes = f_copyright.read()
	f_copyright.close()

	original_logo_bytes = rom.original.buffer[title_address: title_address+ len(diff_bytes)]
	new_logo_bytes = diff(original_logo_bytes, diff_bytes)
	rom.write_bytes(title_address, new_logo_bytes)
	rom.write_bytes(copyright_address, copyright_bytes)

