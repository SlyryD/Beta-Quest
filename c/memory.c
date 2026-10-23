#include "memory.h"

#include <stdbool.h>
#include <stdint.h>

static bool can_copy_words(const void* destination, const void* source, size_t size) {
    const uintptr_t alignment_mask = sizeof(uint32_t) - 1;

    return (((uintptr_t)destination | (uintptr_t)source | size) & alignment_mask) == 0;
}

void memory_copy(void* destination, const void* source, size_t size) {
    uint8_t* destination_bytes = destination;
    const uint8_t* source_bytes = source;

    if (can_copy_words(destination, source, size)) {
        typedef uint32_t aliasing_uint32_t __attribute__((may_alias));
        aliasing_uint32_t* destination_words = destination;
        const aliasing_uint32_t* source_words = source;

        for (size_t i = 0; i < size / sizeof(uint32_t); i++) {
            destination_words[i] = source_words[i];
        }
        return;
    }

    for (size_t i = 0; i < size; i++) {
        destination_bytes[i] = source_bytes[i];
    }
}
