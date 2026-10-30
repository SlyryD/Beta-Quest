#include <assert.h>
#include <stdint.h>

#include "../c/counts.h"

int main() {
    uint16_t event_chk_inf[14] = { 0 };
    uint8_t complete_counts[] = { 1, 3, 1, 9, 2 };
    uint8_t incomplete_counts[] = { 1, 3, 0, 9, 2 };

    assert(all_counts_nonzero(complete_counts, 5));
    assert(!all_counts_nonzero(incomplete_counts, 5));
    incomplete_counts[2] = 1;
    assert(all_counts_nonzero(incomplete_counts, 5));
    incomplete_counts[4] = 0;
    assert(!all_counts_nonzero(incomplete_counts, 5));

    assert(get_carpenters_freed(0x0000) == 0);
    assert(get_carpenters_freed(0x0001) == 1);
    assert(get_carpenters_freed(0x0003) == 2);
    assert(get_carpenters_freed(0x0007) == 3);
    assert(get_carpenters_freed(0x000F) == 4);
    assert(get_carpenters_freed(0x0010) == 0);
    assert(get_carpenters_freed(0x1110) == 0);
    assert(get_carpenters_freed(0xFFFF) == 4);

    assert(get_trial_beams_cleared(event_chk_inf) == 0);
    event_chk_inf[0xB] = (1 << 0xC) | (1 << 0xF) | (1 << 0xE);
    event_chk_inf[0xA] = 1 << 0xD;
    assert(get_trial_beams_cleared(event_chk_inf) == 4);
    event_chk_inf[0xB] |= (1 << 0xD) | (1 << 0xB);
    assert(get_trial_beams_cleared(event_chk_inf) == 6);
    return 0;
}
