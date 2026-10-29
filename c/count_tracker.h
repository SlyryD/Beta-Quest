#ifndef COUNT_TRACKER_H
#define COUNT_TRACKER_H

#include <stdbool.h>
#include <stdint.h>

void count_tracker_update();
void count_tracker_update_chests();
uint8_t count_tracker_get_silver_rooms_cleared();
uint8_t count_tracker_get_enemy_count(uint8_t enemy_type);
bool count_tracker_is_boss_cleared(uint8_t dungeon_index);
bool count_tracker_is_dungeon_completed(uint8_t dungeon_index);

#endif
