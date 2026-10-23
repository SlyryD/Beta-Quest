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

static void test_unaligned_copy() {
    uint8_t source[] = { 0, 1, 2, 3, 4, 5, 6 };
    uint8_t destination[] = { 0, 0, 0, 0, 0, 0, 0 };

    memory_copy(destination + 1, source + 2, 3);

    assert(destination[0] == 0);
    assert(destination[1] == 2);
    assert(destination[2] == 3);
    assert(destination[3] == 4);
    assert(destination[4] == 0);
}

int main() {
    test_aligned_copy();
    test_unaligned_copy();
    return 0;
}
