#include "memory.h"

#include <stdbool.h>
#include <stdint.h>

typedef void* (*memcopy_func_t)(void* destination, const void* source, int32_t size);

static memcopy_func_t const z64_memcopy = (memcopy_func_t)0x80057030;

static bool can_copy_words(const void* destination, const void* source, size_t size) {
    const uintptr_t alignment_mask = sizeof(uint32_t) - 1;

    return (((uintptr_t)destination | (uintptr_t)source | size) & alignment_mask) == 0;
}

void memory_copy(void* destination, const void* source, size_t size) {
    if (can_copy_words(destination, source, size)) {
        typedef uint32_t aliasing_uint32_t __attribute__((may_alias));
        aliasing_uint32_t* destination_words = destination;
        const aliasing_uint32_t* source_words = source;

        for (size_t i = 0; i < size / sizeof(uint32_t); i++) {
            destination_words[i] = source_words[i];
        }
        return;
    }

    z64_memcopy(destination, source, (int32_t)size);
}
