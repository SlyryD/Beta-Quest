#ifndef WORLD_MAP_INFO_H
#define WORLD_MAP_INFO_H

#include "z64.h"

uint8_t get_area_tokens(uint8_t area_index);
uint8_t get_area_hps(uint8_t area_index);

void draw_world_map_info(z64_disp_buf_t* db);

#endif
