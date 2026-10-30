#include "text.h"

#include <stdbool.h>

#include "gfx.h"
#include "util.h"
#include "z64.h"

#define text_max_chars 384
#define text_bucket_count 6
#define text_bucket_size 18

typedef struct {
    uint32_t c : 8;
    uint32_t left : 10;
    uint32_t top : 9;
    uint32_t blue : 1;
    uint32_t dim : 1;
} text_char_t;

static text_char_t* text_end = NULL;
static text_char_t* text_buf = NULL;

void text_init() {
    text_buf = heap_alloc(text_max_chars * sizeof(text_char_t));
    text_end = text_buf;
}

static int text_print_color(const char* s, int left, int top, bool blue, bool dim) {
    char c;
    int count = 0;
    while (c = *(s++)) {
        if (text_end >= text_buf + text_max_chars)
            break;
        text_end->c = c;
        text_end->left = left;
        text_end->top = top;
        text_end->blue = blue;
        text_end->dim = dim;
        text_end++;
        left += font_sprite.tile_w;
        count++;
    }
    return count;
}

int text_print(const char* s, int left, int top) {
    return text_print_color(s, left, top, false, false);
}

int text_print_blue(const char* s, int left, int top) {
    return text_print_color(s, left, top, true, false);
}

int text_print_dim(const char* s, int left, int top) {
    return text_print_color(s, left, top, false, true);
}

void text_flush(z64_disp_buf_t* db) {
    for (int i = 0; i < text_bucket_count; i++) {
        sprite_load(db, &font_sprite, i * text_bucket_size, text_bucket_size);

        text_char_t* text_p = text_buf;
        while (text_p < text_end) {
            char c = text_p->c;
            int left = text_p->left;
            int top = text_p->top;
            text_p++;

            int bucket = (c - 32) / text_bucket_size;
            if (bucket != i || text_p[-1].blue || text_p[-1].dim)
                continue;

            int tile_index = (c - 32) % text_bucket_size;
            sprite_draw(db, &font_sprite, tile_index, left, top, font_sprite.tile_w, font_sprite.tile_h);
        }

        bool has_blue = false;
        text_p = text_buf;
        while (text_p < text_end) {
            char c = text_p->c;
            int left = text_p->left;
            int top = text_p->top;
            bool blue = text_p->blue;
            text_p++;

            int bucket = (c - 32) / text_bucket_size;
            if (bucket != i || !blue)
                continue;

            if (!has_blue) {
                gDPPipeSync(db->p++);
                gDPSetPrimColor(db->p++, 0, 0, 0x64, 0xC8, 0xFF, 0xFF);
                has_blue = true;
            }

            int tile_index = (c - 32) % text_bucket_size;
            sprite_draw(db, &font_sprite, tile_index, left, top, font_sprite.tile_w, font_sprite.tile_h);
        }

        if (has_blue) {
            gDPPipeSync(db->p++);
            gDPSetPrimColor(db->p++, 0, 0, 0xFF, 0xFF, 0xFF, 0xFF);
        }

        bool has_dim = false;
        text_p = text_buf;
        while (text_p < text_end) {
            char c = text_p->c;
            int left = text_p->left;
            int top = text_p->top;
            bool dim = text_p->dim;
            text_p++;

            int bucket = (c - 32) / text_bucket_size;
            if (bucket != i || !dim)
                continue;

            if (!has_dim) {
                gDPPipeSync(db->p++);
                gDPSetPrimColor(db->p++, 0, 0, 0xFF, 0xFF, 0xFF, 0x40);
                has_dim = true;
            }

            int tile_index = (c - 32) % text_bucket_size;
            sprite_draw(db, &font_sprite, tile_index, left, top, font_sprite.tile_w, font_sprite.tile_h);
        }

        if (has_dim) {
            gDPPipeSync(db->p++);
            gDPSetPrimColor(db->p++, 0, 0, 0xFF, 0xFF, 0xFF, 0xFF);
        }
    }

    text_end = text_buf;
}
