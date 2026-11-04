#include "count_info.h"

#include "bingo.h"
#include "counts.h"
#include "gfx.h"
#include "pause_menu.h"
#include "text.h"
#include "util.h"

typedef struct {
    uint8_t count;
    char name[12];
} global_count_entry_t;

static const global_count_entry_t enemy_counts[] = {
    { GCOUNT_STALFOS, "Stalfos" },
    { GCOUNT_GIBDO, "Gibdos" },
    { GCOUNT_IRON_KNUCKLE, "Iron Kns" },
    { GCOUNT_DEAD_HANDS, "Dead Hands" },
    { GCOUNT_WHITE_WOLFOS, "White Wfs" },
    { GCOUNT_SKULL_KID, "Skull Kids" },
};

typedef enum {
    COUNT_ICON_BEAN,
    COUNT_ICON_ICE,
    COUNT_ICON_GOLD_RUPEE,
    COUNT_ICON_SILVER_RUPEE,
    COUNT_ICON_MILK,
    COUNT_ICON_SKULLTULA,
} count_icon_t;

typedef struct {
    uint8_t count;
    uint8_t icon;
    char name[8];
} other_count_entry_t;

static const other_count_entry_t other_counts[] = {
    { GCOUNT_BEANS_PLANTED, COUNT_ICON_BEAN, "Patches" },
    { GCOUNT_TRAP, COUNT_ICON_ICE, "Traps" },
    { GCOUNT_GOLD_RUPEE, COUNT_ICON_GOLD_RUPEE, "Chests" },
    { GCOUNT_SILVER_RUPEE_ROOMS, COUNT_ICON_SILVER_RUPEE, "Rooms" },
    { GCOUNT_COW, COUNT_ICON_MILK, "Cows" },
    { GCOUNT_SOIL_SKULLTULA, COUNT_ICON_SKULLTULA, "Soil" },
};

extern uint32_t CFG_DUNGEON_INFO_ENABLE;

static void draw_rupee_icon(z64_disp_buf_t* db, int left, int top, uint8_t red, uint8_t green, uint8_t blue) {
    gDPPipeSync(db->p++);
    gDPSetPrimColor(db->p++, 0, 0, red, green, blue, 0xFF);
    sprite_draw(db, &key_rupee_clock_sprite, 0, left, top, 12, 12);
}

static void draw_count_icon(z64_disp_buf_t* db, count_icon_t icon, int left, int top) {
    gDPPipeSync(db->p++);
    gDPSetPrimColor(db->p++, 0, 0, 0xFF, 0xFF, 0xFF, 0xFF);

    switch (icon) {
        case COUNT_ICON_BEAN:
            sprite_load(db, &items_sprite, 0x10, 1);
            sprite_draw(db, &items_sprite, 0, left, top, 14, 14);
            break;
        case COUNT_ICON_ICE:
            sprite_load(db, &items_sprite, 0x0C, 1);
            sprite_draw(db, &items_sprite, 0, left, top, 14, 14);
            break;
        case COUNT_ICON_GOLD_RUPEE:
            sprite_load(db, &key_rupee_clock_sprite, 1, 1);
            draw_rupee_icon(db, left + 1, top + 1, 0xFF, 0xD0, 0x30);
            break;
        case COUNT_ICON_SILVER_RUPEE:
            sprite_load(db, &key_rupee_clock_sprite, 1, 1);
            draw_rupee_icon(db, left + 1, top + 1, 0xC0, 0xD8, 0xE0);
            break;
        case COUNT_ICON_MILK:
            sprite_load(db, &items_sprite, 0x1A, 1);
            sprite_draw(db, &items_sprite, 0, left, top, 14, 14);
            break;
        case COUNT_ICON_SKULLTULA:
            sprite_load(db, &quest_items_sprite, 11, 1);
            sprite_draw(db, &quest_items_sprite, 0, left, top, 14, 14);
            break;
    }
}

void draw_count_info(z64_disp_buf_t* db) {
    if (!CFG_DUNGEON_INFO_ENABLE || pause_item_menu_active() != PAUSE_ITEM_MENU_COUNTS) {
        return;
    }

    int padding = 5;
    int row_height = 16;
    int row_count = array_size(other_counts);
    int bg_width = 222;
    int bg_height = padding * 2 + (row_count + 1) * row_height;
    int bg_left = 49;
    int bg_top = (Z64_SCREEN_HEIGHT - bg_height) / 2;
    int enemy_left = bg_left + padding;
    int enemy_value_left = enemy_left + 82;
    int other_icon_left = bg_left + 113;
    int other_label_left = other_icon_left + 18;
    int other_value_left = bg_left + bg_width - padding - 24;

    pause_menu_begin(db, bg_left, bg_top, bg_width, bg_height, 0xD0);

    text_print("Enemies", enemy_left, bg_top + padding);
    text_print("Other", other_icon_left, bg_top + padding);

    for (int i = 0; i < array_size(enemy_counts); i++) {
        const global_count_entry_t* entry = &enemy_counts[i];
        char count[4];
        int top = bg_top + padding + (i + 1) * row_height;

        text_print(entry->name, enemy_left, top);
        pause_menu_format_value(count, bingo_get_global_count(entry->count));
        text_print(count, enemy_value_left, top);
    }

    for (int i = 0; i < array_size(other_counts); i++) {
        const other_count_entry_t* entry = &other_counts[i];
        char count[4];
        int top = bg_top + padding + (i + 1) * row_height;

        draw_count_icon(db, entry->icon, other_icon_left, top);
        text_print(entry->name, other_label_left, top);
        pause_menu_format_value(count, bingo_get_global_count(entry->count));
        text_print(count, other_value_left, top);
    }

    pause_menu_end(db);
}
