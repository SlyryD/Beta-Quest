#include <stdint.h>

typedef void (*set_framebuffer_info_func_t)(void*, int32_t, int32_t, void*, void*);

extern uint32_t CFG_PJ64_PAUSE_FIX;

static set_framebuffer_info_func_t const set_framebuffer_info = (set_framebuffer_info_func_t)0x8009DD10;

void set_pause_framebuffer_info(void* info, int32_t width, int32_t height, void* framebuffer, void* depth_buffer) {
    if (CFG_PJ64_PAUSE_FIX) {
        height = 0;
    }
    set_framebuffer_info(info, width, height, framebuffer, depth_buffer);
}
