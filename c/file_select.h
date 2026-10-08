#ifndef FILE_SELECT_H
#define FILE_SELECT_H

#include "z64.h"

void load_file_select_settings(z64_sram_data_t* sram_buffer);
void update_file_select_options(z64_ctxt_t* menu);
void draw_file_select_options(z64_ctxt_t* menu);

#endif
