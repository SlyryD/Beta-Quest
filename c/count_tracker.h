#ifndef COUNT_TRACKER_H
#define COUNT_TRACKER_H

#include <stdint.h>

void count_tracker_update();
void count_tracker_update_chests();
uint8_t count_tracker_get_silver_rooms_cleared();
uint8_t count_tracker_get_enemy_count(uint8_t enemy_type);

#endif
