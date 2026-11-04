#include "event_info.h"

#include <stdbool.h>

#include "bingo.h"
#include "count_tracker.h"
#include "counts.h"
#include "gfx.h"
#include "pause_menu.h"
#include "save.h"
#include "text.h"
#include "util.h"
#include "z64.h"

extern uint32_t CFG_DUNGEON_INFO_ENABLE;

static void draw_status_sprite(z64_disp_buf_t* db, sprite_t* sprite, int tile, int left, int top, int size,
                               bool complete, uint8_t red, uint8_t green, uint8_t blue) {
    pause_menu_draw_sprite(db, sprite, tile, left, top, size, size, complete, red, green, blue);
}

static void draw_status_text(const char* text, int left, int top, bool complete) {
    if (complete) {
        text_print(text, left, top);
    } else {
        text_print_dim(text, left, top);
    }
}

static void draw_event_skull(z64_disp_buf_t* db, int left, int top, uint8_t objective, uint8_t red, uint8_t green,
                             uint8_t blue) {
    draw_status_sprite(db, &linkhead_skull_sprite, 1, left, top, 14, count_objective_is_complete(objective), red,
                       green, blue);
}

void draw_event_info(z64_disp_buf_t* db) {
    if (!CFG_DUNGEON_INFO_ENABLE || pause_item_menu_active() != PAUSE_ITEM_MENU_EVENTS) {
        return;
    }

    static const char* labels[] = {
        "Dodongo", "Jabu", "Forest", "Fire", "Water", "Spirit", "Ganon", "Trials", "Sky",
    };
    static const uint8_t trial_colors[][3] = {
        { 0xFF, 0xF4, 0x9B }, { 0x4F, 0xE5, 0x7A }, { 0xFF, 0x64, 0x4D },
        { 0x55, 0xBA, 0xFF }, { 0xFF, 0xA9, 0x4F }, { 0xC7, 0x7C, 0xFF },
    };
    int bg_left = 20;
    int bg_top = 20;
    int bg_width = 280;
    int padding = 6;
    int title_height = 18;
    int row_height = 17;
    int bg_height = padding * 2 + title_height + array_size(labels) * row_height;
    int label_left = bg_left + padding;
    int indicator_left = label_left + 68;
    int count_right = bg_left + bg_width - padding;

    pause_menu_begin(db, bg_left, bg_top, bg_width, bg_height, 0xD0);
    text_print("Dungeon Events", label_left, bg_top + padding);

    for (int i = 0; i < array_size(labels); i++) {
        text_print(labels[i], label_left, bg_top + padding + title_height + i * row_height);
    }

    int top = bg_top + padding + title_height;
    draw_event_skull(db, indicator_left, top, OBJECTIVE_DODONGO_LIZALFOS, 0xFF, 0xFF, 0xFF);

    top += row_height;
    draw_event_skull(db, indicator_left, top, OBJECTIVE_BIG_OCTO, 0xFF, 0xFF, 0xFF);
    char count[4];
    pause_menu_format_value(count, bingo_get_global_count(GCOUNT_TENTACLE));
    int count_left = count_right - pause_menu_text_width(count);
    text_print("Tentacles", count_left - 4 - 9 * font_sprite.tile_w, top);
    text_print(count, count_left, top);

    top += row_height;
    draw_event_skull(db, indicator_left, top, OBJECTIVE_POE_AMY, 0x4C, 0xFF, 0x69);
    draw_event_skull(db, indicator_left + 17, top, OBJECTIVE_POE_JOELLE, 0xFF, 0x64, 0x64);
    draw_event_skull(db, indicator_left + 34, top, OBJECTIVE_POE_BETH, 0x65, 0xA8, 0xFF);
    draw_event_skull(db, indicator_left + 51, top, OBJECTIVE_POE_MEG, 0xD0, 0x6A, 0xFF);
    bool forest_door_complete = count_scene_switch_set(SCENE_FOREST_TEMPLE, BOSS_DOOR_SWITCH_FLAG);
    draw_status_sprite(db, &quest_items_sprite, 14, indicator_left + 70, top, 14, forest_door_complete, 0xFF, 0xFF,
                       0xFF);
    draw_status_text("Door", indicator_left + 85, top, forest_door_complete);

    top += row_height;
    draw_event_skull(db, indicator_left, top, OBJECTIVE_FIRE_FLARE_DANCERS, 0xFF, 0xFF, 0xFF);
    int fire_left = indicator_left + 17;
    if (!z64_file.master_quest) {
        draw_status_sprite(db, &key_rupee_clock_sprite, 1, fire_left, top + 1, 12,
                           count_objective_is_complete(OBJECTIVE_FIRE_GOLD_RUPEE), 0xFF, 0xD0, 0x30);
    }
    pause_menu_format_value(count, bingo_get_global_count(GCOUNT_FIRE_GORON));
    count_left = count_right - pause_menu_text_width(count);
    draw_status_sprite(db, &items_sprite, 0x28, count_left - 17, top, 14, true, 0xFF, 0xFF, 0xFF);
    text_print(count, count_left, top);

    top += row_height;
    draw_event_skull(db, indicator_left, top, OBJECTIVE_DARK_LINK, 0xFF, 0xFF, 0xFF);

    top += row_height;
    draw_event_skull(db, indicator_left, top, OBJECTIVE_NABOORU_KNUCKLE, 0xFF, 0xFF, 0xFF);
    draw_status_sprite(db, &items_sprite, 0x09, indicator_left + 17, top, 14,
                       count_objective_is_complete(OBJECTIVE_SPIRIT_BOMBCHU_CHEST), 0xFF, 0xFF, 0xFF);
    draw_status_text("Plat", indicator_left + 34, top, count_objective_is_complete(OBJECTIVE_SPIRIT_PLATFORM));
    uint8_t rusty_switch_count = count_tracker_get_spirit_rusty_switch_count();
    pause_menu_format_value(count, rusty_switch_count);
    count_left = count_right - pause_menu_text_width(count);
    draw_status_sprite(db, &items_sprite, 0x11, count_left - 17, top, 14, true, 0xFF, 0xFF, 0xFF);
    text_print(count, count_left, top);

    top += row_height;
    bool ganon_door_complete = count_scene_switch_set(SCENE_GANONS_TOWER, BOSS_DOOR_SWITCH_FLAG);
    draw_status_sprite(db, &quest_items_sprite, 14, indicator_left, top, 14, ganon_door_complete, 0xFF, 0xFF, 0xFF);
    draw_status_text("Door", indicator_left + 15, top, ganon_door_complete);
    pause_menu_format_value(count, bingo_get_global_count(GCOUNT_TRIAL_BEAMS));
    count_left = count_right - pause_menu_text_width(count);
    text_print("Beams", count_left - 4 - 5 * font_sprite.tile_w, top);
    text_print(count, count_left, top);

    top += row_height;
    for (trial_t trial = TRIAL_LIGHT; trial < TRIAL_COUNT; trial++) {
        draw_status_sprite(db, &medals_sprite, trial, indicator_left + trial * 17, top, 14,
                           get_trial_beam_cleared(trial),
                           trial_colors[trial][0], trial_colors[trial][1], trial_colors[trial][2]);
    }

    top += row_height;
    draw_event_skull(db, indicator_left, top, OBJECTIVE_SKY_TEMPLE, 0xFF, 0xFF, 0xFF);

    pause_menu_end(db);
}
