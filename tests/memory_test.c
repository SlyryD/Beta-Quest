#include <assert.h>
#include <stdint.h>

#include "../c/memory.h"

static void test_aligned_copy() {
    uint32_t source[] = { 0x01234567, 0x89ABCDEF, 0x13579BDF };
    uint32_t destination[] = { 0, 0, 0 };

    memory_copy(destination, source, sizeof(source));

    for (size_t i = 0; i < sizeof(source) / sizeof(source[0]); i++) {
        assert(destination[i] == source[i]);
    }
}

int main() {
    test_aligned_copy();
    return 0;
}
