#include "item_effects.h"

void give_small_key()
{
    int8_t dungeon_id = z64_file.minimap_index;
    int8_t current_keys = z64_file.dungeon_keys[dungeon_id] > 0 ? z64_file.dungeon_keys[dungeon_id] : 0;
    z64_file.dungeon_keys[dungeon_id] = current_keys + 1;
    uint32_t flag = z64_file.scene_flags[dungeon_id].unk_00_;
    int8_t total_keys = flag >> 0x10;
    z64_file.scene_flags[dungeon_id].unk_00_ = (flag & 0x0000FFFF) | ((total_keys + 1) << 0x10);
}
