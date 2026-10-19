#include <assert.h>
#include <stdint.h>

#include "../c/counts.h"

int main() {
    assert(get_carpenters_freed(0x0000) == 0);
    assert(get_carpenters_freed(0x0001) == 1);
    assert(get_carpenters_freed(0x0003) == 2);
    assert(get_carpenters_freed(0x0007) == 3);
    assert(get_carpenters_freed(0x000F) == 4);
    assert(get_carpenters_freed(0x0010) == 0);
    assert(get_carpenters_freed(0xFFFF) == 4);
    return 0;
}
