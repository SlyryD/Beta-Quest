#include "dungeon_info.h"

#include <stdbool.h>

#include "count_tracker.h"
#include "counts.h"
#include "gfx.h"
#include "pause_menu.h"
#include "save.h"
#include "text.h"
#include "util.h"
#include "z64.h"

typedef struct {
    uint8_t index;
    struct {
        uint8_t has_keys : 1;
        uint8_t has_boss_key : 1;
        uint8_t has_card : 1;
        uint8_t has_map : 1;
        uint8_t has_tokens : 1;
    };
    char name[10];
} dungeon_entry_t;

static const dungeon_entry_t dungeons[] = {
    { SCENE_DEKU_TREE, 0, 0, 0, 1, 1, "Deku" },
    { SCENE_DODONGOS_CAVERN, 0, 0, 0, 1, 1, "Dodongo" },
    { SCENE_JABU_JABU, 0, 0, 0, 1, 1, "Jabu" },

    { SCENE_FOREST_TEMPLE, 1, 1, 0, 1, 1, "Forest" },
    { SCENE_FIRE_TEMPLE, 1, 1, 0, 1, 1, "Fire" },
    { SCENE_WATER_TEMPLE, 1, 1, 0, 1, 1, "Water" },
    { SCENE_SHADOW_TEMPLE, 1, 1, 0, 1, 1, "Shadow" },
    { SCENE_SPIRIT_TEMPLE, 1, 1, 0, 1, 1, "Spirit" },

    { SCENE_BOTTOM_OF_THE_WELL, 1, 0, 0, 1, 1, "BotW" },
    { SCENE_ICE_CAVERN, 0, 0, 0, 1, 1, "Ice" },
    { SCENE_THIEVES_HIDEOUT, 1, 0, 1, 0, 0, "Hideout" },
    { SCENE_GERUDO_TRAINING_GROUND, 1, 0, 0, 0, 0, "GTG" },
    { SCENE_INSIDE_GANONS_CASTLE, 1, 1, 0, 0, 0, "Ganon" },
};

static const int dungeons_length = array_size(dungeons);

extern uint32_t CFG_DUNGEON_INFO_ENABLE;

bool is_valid_dungeon_index(uint8_t dungeon_index) {
    for (int i = 0; i < dungeons_length; i++) {
        if (dungeons[i].index == dungeon_index) {
            return true;
        }
    }

    return false;
}

uint8_t get_dungeon_tokens(uint8_t dungeon_index) {
    return get_tokens((gs_flags_index_t)dungeon_index);
}

uint8_t get_dungeon_hps(uint8_t dungeon_index) {
    switch (dungeon_index) {
        case SCENE_ICE_CAVERN: {
            bool has_ice_cavern_hp =
                (z64_game.scene_index == SCENE_ICE_CAVERN && z64_game.collect_flags & (1 << 0x01)) ||
                z64_file.scene_flags[SCENE_ICE_CAVERN].collect & (1 << 0x01);
            return has_ice_cavern_hp ? 1 : 0;
        }
        default: {
            return 0xFF;
        }
    }
}

uint8_t get_dungeon_small_keys(uint8_t dungeon_index) {
    return z64_file.scene_flags[dungeon_index].unk_00_ >> 0x10;
}

uint8_t get_dungeon_count(uint8_t dungeon_index, uint8_t count_index) {
    if (!is_valid_dungeon_index(dungeon_index)) {
        return 0xFF;
    }

    switch (count_index) {
        case DCOUNT_TOKEN: {
            return get_dungeon_tokens(dungeon_index);
        }
        case DCOUNT_HEART_PIECE: {
            return get_dungeon_hps(dungeon_index);
        }
        case DCOUNT_SMALL_KEY: {
            return get_dungeon_small_keys(dungeon_index);
        }
        default: {
            return 0xFF;
        }
    }
}

static bool dungeon_reward_active(int row) {
    if (row >= 0 && row < 8) {
        return count_tracker_is_dungeon_completed(dungeons[row].index);
    }

    switch (row) {
        case 12:
            return z64_file.light_medallion;
        default:
            return false;
    }
}

static uint8_t boss_door_scene(uint8_t dungeon_index) {
    return dungeon_index == SCENE_INSIDE_GANONS_CASTLE ? SCENE_GANONS_TOWER : dungeon_index;
}

void draw_dungeon_info(z64_disp_buf_t* db) {
    if (!CFG_DUNGEON_INFO_ENABLE || pause_item_menu_active() != PAUSE_ITEM_MENU_DUNGEONS) {
        return;
    }

    // Set up dimensions

    int icon_size = 16;
    int padding = 1;
    int bg_width = padding + (7 * font_sprite.tile_w) + padding +
                   (8 * icon_size) + // skull/count, key+current/total, map, compass, bk/card, reward
                   padding;
    int bg_height = padding + (dungeons_length * (icon_size + padding));
    int bg_left = (Z64_SCREEN_WIDTH - bg_width) / 2;
    int bg_top = (Z64_SCREEN_HEIGHT - bg_height) / 2;

    int left = bg_left + padding;
    int start_top = bg_top + padding;
    int indicator_left = left + (7 * font_sprite.tile_w) + padding;
    int token_left = indicator_left;
    int key_left = token_left + (2 * icon_size);
    int map_left = key_left + (2 * icon_size);
    int boss_key_left = map_left + (2 * icon_size);
    int reward_left = boss_key_left + icon_size;

    pause_menu_begin(db, bg_left, bg_top, bg_width, bg_height, 0xD0);

    // Draw navigation hints

    int hint_width = 42;
    int hint_height = 18;
    int left_hint_left = 4;
    int hint_top = (Z64_SCREEN_HEIGHT - hint_height) / 2;
    int up_hint_top = hint_top - 24;
    int right_hint_left = Z64_SCREEN_WIDTH - hint_width - 4;
    int gear_hint_width = 42;
    int gear_hint_left = Z64_SCREEN_WIDTH - gear_hint_width - 4;
    int gear_hint_top = hint_top + 24;

    gDPSetCombineMode(db->p++, G_CC_PRIMITIVE, G_CC_PRIMITIVE);
    gDPSetPrimColor(db->p++, 0, 0, 0x00, 0x00, 0x00, 0xB0);
    gSPTextureRectangle(db->p++, left_hint_left << 2, hint_top << 2, (left_hint_left + hint_width) << 2,
                        (hint_top + hint_height) << 2, 0, 0, 0, 1 << 10, 1 << 10);
    gSPTextureRectangle(db->p++, right_hint_left << 2, hint_top << 2, (right_hint_left + hint_width) << 2,
                        (hint_top + hint_height) << 2, 0, 0, 0, 1 << 10, 1 << 10);
    gSPTextureRectangle(db->p++, left_hint_left << 2, up_hint_top << 2, (left_hint_left + hint_width) << 2,
                        (up_hint_top + hint_height) << 2, 0, 0, 0, 1 << 10, 1 << 10);
    gSPTextureRectangle(db->p++, gear_hint_left << 2, gear_hint_top << 2, (gear_hint_left + gear_hint_width) << 2,
                        (gear_hint_top + hint_height) << 2, 0, 0, 0, 1 << 10, 1 << 10);

    gDPPipeSync(db->p++);
    gDPSetCombineMode(db->p++, G_CC_MODULATEIA_PRIM, G_CC_MODULATEIA_PRIM);
    gDPSetPrimColor(db->p++, 0, 0, 0xFF, 0xFF, 0xFF, 0xFF);
    text_print("< Ct", left_hint_left + 5, hint_top + 2);
    text_print("Ar >", right_hint_left + 5, hint_top + 2);
    text_print("^ Ev", left_hint_left + 5, up_hint_top + 2);
    text_print("Ge V", gear_hint_left + 5, gear_hint_top + 2);

    gDPPipeSync(db->p++);
    gDPSetCombineMode(db->p++, G_CC_MODULATEIA_PRIM, G_CC_MODULATEIA_PRIM);
    gDPSetPrimColor(db->p++, 0, 0, 0xFF, 0xFF, 0xFF, 0xFF);

    // Draw dungeon names

    for (int i = 0; i < dungeons_length; i++) {
        int top = start_top + ((icon_size + padding) * i) + 1;
        text_print(dungeons[i].name, left, top);
    }

    left += (7 * font_sprite.tile_w) + padding;

    // Draw skull sprite

    sprite_load(db, &quest_items_sprite, 11, 1);

    for (int i = 0; i < dungeons_length; i++) {
        const dungeon_entry_t* d = &(dungeons[i]);
        if (!d->has_tokens)
            continue;

        int top = start_top + ((icon_size + padding) * i);
        sprite_draw(db, &quest_items_sprite, 0, left, top, icon_size, icon_size);
    }

    // Draw Gerudo Mask icon for Hideout

    sprite_load(db, &items_sprite, 0x2A, 1);

    for (int i = 0; i < dungeons_length; i++) {
        const dungeon_entry_t* d = &(dungeons[i]);
        if (d->index != SCENE_THIEVES_HIDEOUT)
            continue;

        int top = start_top + ((icon_size + padding) * i);
        sprite_draw(db, &items_sprite, 0, left, top, icon_size, icon_size);
    }

    left += icon_size;

    // Draw skull and Hideout carpenter counts

    for (int i = 0; i < dungeons_length; i++) {
        const dungeon_entry_t* d = &(dungeons[i]);
        if (!d->has_tokens && d->index != SCENE_THIEVES_HIDEOUT)
            continue;

        uint8_t count_value =
            d->index == SCENE_THIEVES_HIDEOUT ? get_global_count(GCOUNT_CARPENTER) : get_dungeon_tokens(d->index);

        char count[2] = "0";
        count[0] += (count_value % 10);
        int top = start_top + ((icon_size + padding) * i) + 1;
        text_print(count, left, top);
    }

    left += icon_size;

    // Draw key sprite

    sprite_load(db, &key_rupee_clock_sprite, 0, key_rupee_clock_sprite.tile_count);

    for (int i = 0; i < dungeons_length; i++) {
        const dungeon_entry_t* d = &(dungeons[i]);
        if (!d->has_keys)
            continue;

        int top = start_top + ((icon_size + padding) * i);
        sprite_draw(db, &key_rupee_clock_sprite, 0, left, top, icon_size, icon_size);
    }

    // Draw hp sprite

    sprite_load(db, &quest_items_sprite, 13, 1);

    for (int i = 0; i < dungeons_length; i++) {
        const dungeon_entry_t* d = &(dungeons[i]);
        if (d->index != SCENE_ICE_CAVERN)
            continue;

        int top = start_top + ((icon_size + padding) * i);
        sprite_draw(db, &quest_items_sprite, 0, left, top, icon_size, icon_size);
    }

    left += icon_size;

    // Draw current Small Key counts over the key icons.

    gDPPipeSync(db->p++);
    gDPSetCombineMode(db->p++, G_CC_MODULATEIA_PRIM, G_CC_MODULATEIA_PRIM);
    gDPSetPrimColor(db->p++, 0, 0, 0xFF, 0xFF, 0xFF, 0xFF);

    for (int i = 0; i < dungeons_length; i++) {
        const dungeon_entry_t* d = &(dungeons[i]);
        if (!d->has_keys)
            continue;

        uint8_t current_keys = z64_file.dungeon_keys[d->index];
        if (current_keys == 0xFF) {
            current_keys = 0;
        }

        int top = start_top + ((icon_size + padding) * i);
        sprite_load(db, &item_digit_sprite, current_keys % 10, 1);
        sprite_draw(db, &item_digit_sprite, 0, left + 8, top + 8, 8, 8);
    }

    left += icon_size;

    // Draw total Small Key counts.

    for (int i = 0; i < dungeons_length; i++) {
        const dungeon_entry_t* d = &(dungeons[i]);
        if (!d->has_keys)
            continue;

        uint8_t total_keys = get_dungeon_small_keys(d->index);
        if (total_keys == 0xFF) {
            total_keys = 0;
        }

        char count[2] = "0";
        count[0] += (total_keys % 10);
        int top = start_top + ((icon_size + padding) * i) + 1;
        text_print(count, left, top);
    }

    // Draw hp counts

    for (int i = 0; i < dungeons_length; i++) {
        const dungeon_entry_t* d = &(dungeons[i]);
        uint8_t hps = get_dungeon_hps(d->index);
        if (hps == 0xFF)
            continue;

        char count[2] = "0";
        count[0] += (hps % 10);
        int top = start_top + ((icon_size + padding) * i) + 1;
        text_print(count, left, top);
    }
    left += icon_size;
    left += icon_size;

    // Draw boss keys

    left = boss_key_left;
    sprite_load(db, &quest_items_sprite, 14, 1);

    for (int i = 0; i < dungeons_length; i++) {
        const dungeon_entry_t* d = &(dungeons[i]);
        // Replace index 13 (Ganon's Castle) with 10 (Ganon's Tower)
        int index = d->index == 13 ? 10 : d->index;

        if (d->has_boss_key && z64_file.dungeon_items[index].boss_key) {
            int top = start_top + ((icon_size + padding) * i);
            sprite_draw(db, &quest_items_sprite, 0, left, top, icon_size, icon_size);
        }
    }

    // Draw gerudo card

    sprite_load(db, &quest_items_sprite, 10, 1);

    for (int i = 0; i < dungeons_length; i++) {
        const dungeon_entry_t* d = &(dungeons[i]);
        if (d->has_card && z64_file.gerudos_card) {
            int top = start_top + ((icon_size + padding) * i);
            sprite_draw(db, &quest_items_sprite, 0, left, top, icon_size, icon_size);
        }
    }

    sprite_load(db, &map_dungeon_entrance_sprite, 0, 1);
    for (int i = 0; i < dungeons_length; i++) {
        const dungeon_entry_t* d = &(dungeons[i]);
        if (!d->has_boss_key ||
            !count_scene_switch_set(boss_door_scene(d->index), BOSS_DOOR_SWITCH_FLAG)) {
            continue;
        }

        int top = start_top + ((icon_size + padding) * i);
        sprite_draw(db, &map_dungeon_entrance_sprite, 0,
            boss_key_left + 8, top + 8, 8, 8);
    }

    left += icon_size;

    // Draw maps

    left = map_left;
    sprite_load(db, &quest_items_sprite, 16, 1);

    for (int i = 0; i < dungeons_length; i++) {
        const dungeon_entry_t* d = &(dungeons[i]);
        if (d->has_map && z64_file.dungeon_items[d->index].map) {
            int top = start_top + ((icon_size + padding) * i);
            sprite_draw(db, &quest_items_sprite, 0, left, top, icon_size, icon_size);
        }
    }

    left += icon_size;

    // Draw compasses

    sprite_load(db, &quest_items_sprite, 15, 1);

    for (int i = 0; i < dungeons_length; i++) {
        const dungeon_entry_t* d = &(dungeons[i]);
        if (d->has_map && z64_file.dungeon_items[d->index].compass) {
            int top = start_top + ((icon_size + padding) * i);
            sprite_draw(db, &quest_items_sprite, 0, left, top, icon_size, icon_size);
        }
    }

    // Draw dungeon rewards

    for (int i = 0; i < 3; i++) {
        int top = start_top + ((icon_size + padding) * i);
        pause_menu_draw_sprite(db, &stones_sprite, i, reward_left, top, icon_size, icon_size,
                               dungeon_reward_active(i), 0xFF, 0xFF, 0xFF);
    }

    static const uint8_t medal_tiles[] = { 0, 1, 2, 4, 3 };
    static const uint8_t medal_colors[][3] = {
        { 0x4F, 0xE5, 0x7A },
        { 0xFF, 0x64, 0x4D },
        { 0x55, 0xBA, 0xFF },
        { 0xC7, 0x7C, 0xFF },
        { 0xFF, 0xA9, 0x4F },
    };
    for (int i = 0; i < 5; i++) {
        int row = i + 3;
        int top = start_top + ((icon_size + padding) * row);
        pause_menu_draw_sprite(db, &medals_sprite, medal_tiles[i], reward_left, top, icon_size, icon_size,
                               dungeon_reward_active(row), medal_colors[i][0], medal_colors[i][1],
                               medal_colors[i][2]);
    }

    int ganon_top = start_top + ((icon_size + padding) * 12);
    pause_menu_draw_sprite(db, &medals_sprite, 5, reward_left, ganon_top, icon_size, icon_size,
                           dungeon_reward_active(12), 0xFF, 0xFF, 0x64);

    // Overlay boss-clear skulls on the dungeon reward column.

    gDPPipeSync(db->p++);
    gDPSetCombineMode(db->p++, G_CC_MODULATEIA_PRIM, G_CC_MODULATEIA_PRIM);
    gDPSetPrimColor(db->p++, 0, 0, 0xFF, 0xFF, 0xFF, 0xFF);
    sprite_load(db, &map_boss_sprite, 0, 1);

    for (int i = 0; i < dungeons_length; i++) {
        const dungeon_entry_t* dungeon = &dungeons[i];
        if (!count_tracker_is_boss_cleared(dungeon->index))
            continue;

        int top = start_top + ((icon_size + padding) * i);
        sprite_draw(db, &map_boss_sprite, 0, reward_left + 8, top + 8, 8, 8);
    }

    pause_menu_end(db);
}
