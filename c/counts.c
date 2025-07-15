#include "counts.h"

#include "z64.h"

uint32_t gGsFlagsMasks[] = { 0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000 };
uint32_t gGsFlagsShifts[] = { 0, 8, 16, 24 };

// gs_flags at 8011B46C
#define GET_GS_FLAGS(gs_flags_index) \
    ((z64_file.gs_flags[(gs_flags_index) >> 2] & gGsFlagsMasks[(gs_flags_index)&3]) >> gGsFlagsShifts[(gs_flags_index)&3])

uint8_t get_tokens(uint8_t gs_flags_index) {
    uint8_t tokens = 0;
    int32_t token_flags = GET_GS_FLAGS(gs_flags_index);
    while (token_flags) {
        tokens += token_flags & 1;
        token_flags >>= 1;
    }

    return tokens;
}
