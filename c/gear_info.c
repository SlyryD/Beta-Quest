#include "gear_info.h"

#include "bingo.h"
#include "counts.h"
#include "gfx.h"
#include "pause_menu.h"
#include "text.h"
#include "z64.h"

extern uint32_t CFG_DUNGEON_INFO_ENABLE;

static void draw_item(z64_disp_buf_t* db, int tile, int left, int top, bool owned) {
    pause_menu_draw_sprite(db, &items_sprite, tile, left, top, 14, 14, owned, 0xFF, 0xFF, 0xFF);
}

static void draw_ammo_number(z64_disp_buf_t* db, uint8_t value, int left, int top) {
    uint8_t tens = value / 10;
    uint8_t ones = value % 10;

    gDPPipeSync(db->p++);
    gDPSetCombineMode(db->p++, G_CC_MODULATEIA_PRIM, G_CC_MODULATEIA_PRIM);
    gDPSetPrimColor(db->p++, 0, 0, 120, 255, 0, 0xFF);
    sprite_load(db, &item_digit_sprite, tens, 1);
    sprite_draw(db, &item_digit_sprite, 0, left, top, 8, 8);
    sprite_load(db, &item_digit_sprite, ones, 1);
    sprite_draw(db, &item_digit_sprite, 0, left + 6, top, 8, 8);
}

static uint8_t capacity_for_level(uint8_t level, uint8_t first, uint8_t second, uint8_t third) {
    if (level == 1) {
        return first;
    } else if (level == 2) {
        return second;
    } else if (level == 3) {
        return third;
    }
    return 0;
}

static void draw_capacity(z64_disp_buf_t* db, uint8_t level, int small_tile, int large_tile,
                          uint8_t first_capacity, uint8_t second_capacity, uint8_t third_capacity,
                          int left, int top) {
    uint8_t capacity = capacity_for_level(level, first_capacity, second_capacity, third_capacity);

    draw_item(db, level == 3 ? large_tile : small_tile, left, top, capacity > 0);
    if (capacity > 0) {
        draw_ammo_number(db, capacity, left + 8, top + 6);
    }
}

static void draw_capacity_count(z64_disp_buf_t* db, int tile, uint8_t level,
                                uint8_t first_capacity, uint8_t second_capacity, uint8_t third_capacity,
                                uint8_t count, int left, int top) {
    char text[4];
    uint8_t capacity = capacity_for_level(level, first_capacity, second_capacity, third_capacity);

    draw_item(db, tile, left, top, true);
    if (capacity > 0) {
        draw_ammo_number(db, capacity, left, top + 6);
    }
    gDPPipeSync(db->p++);
    gDPSetPrimColor(db->p++, 0, 0, 0xFF, 0xFF, 0xFF, 0xFF);
    pause_menu_format_value(text, count);
    text_print(text, left + 16, top);
}

static void draw_quest(z64_disp_buf_t* db, int tile, int left, int top, bool owned) {
    pause_menu_draw_sprite(db, &quest_items_sprite, tile, left, top, 14, 14, owned, 0xFF, 0xFF, 0xFF);
}

static void draw_upgrade(z64_disp_buf_t* db, int first_tile, uint8_t level, int left, int top) {
    draw_item(db, first_tile + (level > 0 ? level - 1 : 0), left, top, level > 0);
}

static void draw_count_color(z64_disp_buf_t* db, sprite_t* sprite, int tile, int left, int top, uint16_t value,
                             uint8_t red, uint8_t green, uint8_t blue) {
    char text[4];
    gDPPipeSync(db->p++);
    gDPSetPrimColor(db->p++, 0, 0, red, green, blue, 0xFF);
    sprite_load(db, sprite, tile, 1);
    sprite_draw(db, sprite, 0, left, top, 14, 14);
    pause_menu_format_value(text, value);
    text_print(text, left + 16, top);
}

static void draw_count(z64_disp_buf_t* db, sprite_t* sprite, int tile, int left, int top, uint16_t value) {
    draw_count_color(db, sprite, tile, left, top, value, 0xFF, 0xFF, 0xFF);
}

static void draw_hud_count(z64_disp_buf_t* db, sprite_t* sprite, int tile, int left, int top, uint16_t value,
                           uint8_t prim_red, uint8_t prim_green, uint8_t prim_blue,
                           uint8_t env_red, uint8_t env_green, uint8_t env_blue) {
    char text[4];
    gDPPipeSync(db->p++);
    gDPSetCombineLERP(db->p++, PRIMITIVE, ENVIRONMENT, TEXEL0, ENVIRONMENT, TEXEL0, 0, PRIMITIVE, 0,
                      PRIMITIVE, ENVIRONMENT, TEXEL0, ENVIRONMENT, TEXEL0, 0, PRIMITIVE, 0);
    gDPSetPrimColor(db->p++, 0, 0, prim_red, prim_green, prim_blue, 0xFF);
    gDPSetEnvColor(db->p++, env_red, env_green, env_blue, 0xFF);
    sprite_load(db, sprite, tile, 1);
    sprite_draw(db, sprite, 0, left, top, 14, 14);
    pause_menu_format_value(text, value);
    text_print(text, left + 16, top);
    gDPPipeSync(db->p++);
    gDPSetCombineMode(db->p++, G_CC_MODULATEIA_PRIM, G_CC_MODULATEIA_PRIM);
}

static void draw_rupee_count(z64_disp_buf_t* db, int left, int top, uint16_t value) {
    static const uint8_t colors[4][3] = {
        { 0xC8, 0xFF, 0x64 },
        { 0x82, 0x82, 0xFF },
        { 0xFF, 0x64, 0x64 },
        { 0xFF, 0x5A, 0xFF },
    };
    const uint8_t* color = colors[z64_file.wallet];
    draw_hud_count(db, &key_rupee_clock_sprite, 1, left, top, value,
                   color[0], color[1], color[2], 0x00, 0x50, 0x00);
}

static void draw_owned_row(z64_disp_buf_t* db, const char* label, int top, int first_tile, uint8_t count) {
    text_print(label, 28, top);
    for (uint8_t i = 0; i < count; i++) {
        draw_item(db, first_tile + i, 92 + i * 17, top, bingo_item_owned(first_tile + i));
    }
}

void draw_gear_info(z64_disp_buf_t* db) {
    if (!CFG_DUNGEON_INFO_ENABLE || pause_item_menu_active() != PAUSE_ITEM_MENU_GEAR) {
        return;
    }

    pause_menu_begin(db, 20, 7, 280, 180, 0xD8);

    text_print("Gear", 28, 12);
    text_print("Inventory / Quest", 164, 12);

    draw_owned_row(db, "Swords", 31, Z64_ITEM_KOKIRI_SWORD, 3);
    draw_owned_row(db, "Shields", 48, Z64_ITEM_DEKU_SHIELD, 3);
    draw_owned_row(db, "Tunics", 65, Z64_ITEM_KOKIRI_TUNIC, 3);
    draw_owned_row(db, "Boots", 82, Z64_ITEM_KOKIRI_BOOTS, 3);

    text_print("Bow", 28, 99);
    draw_item(db, Z64_ITEM_BOW, 92, 99, bingo_item_owned(Z64_ITEM_BOW));
    draw_capacity(db, bingo_get_capacity_level(Z64_ITEM_BOW), Z64_ITEM_QUIVER_30, Z64_ITEM_QUIVER_50,
                  30, 40, 50, 109, 99);

    text_print("Sling", 28, 116);
    draw_item(db, Z64_ITEM_SLINGSHOT, 92, 116, bingo_item_owned(Z64_ITEM_SLINGSHOT));
    draw_capacity(db, bingo_get_capacity_level(Z64_ITEM_SLINGSHOT), Z64_ITEM_BULLET_BAG_30,
                  Z64_ITEM_BULLET_BAG_50, 30, 40, 50, 109, 116);

    text_print("Bombs", 28, 133);
    draw_item(db, Z64_ITEM_BOMB, 92, 133, bingo_item_owned(Z64_ITEM_BOMB));
    draw_capacity(db, bingo_get_capacity_level(Z64_ITEM_BOMB), Z64_ITEM_BOMB_BAG_20, Z64_ITEM_BOMB_BAG_40,
                  20, 30, 40, 109, 133);

    draw_capacity_count(db, Z64_ITEM_STICK, bingo_get_capacity_level(Z64_ITEM_STICK),
                        10, 20, 30, bingo_get_ammo(Z64_SLOT_STICK), 28, 150);
    draw_capacity_count(db, Z64_ITEM_NUT, bingo_get_capacity_level(Z64_ITEM_NUT),
                        20, 30, 40, bingo_get_ammo(Z64_SLOT_NUT), 73, 150);
    draw_count(db, &items_sprite, Z64_ITEM_BEANS, 118, 150, bingo_get_ammo(Z64_SLOT_BEANS));
    draw_rupee_count(db, 28, 167, bingo_get_goal_progress(BINGO_GOAL_N_500_RUPEES).current);
    draw_count(db, &quest_items_sprite, 11, 73, 167,
               bingo_get_goal_progress(BINGO_GOAL_N_30_DIFFERENT_SKULLTULAS).current);
    draw_hud_count(db, &heart_sprite, 4, 118, 167, bingo_get_global_count(GCOUNT_HEART),
                   0xFF, 0x46, 0x32, 0x32, 0x28, 0x3C);

    text_print("Magic", 164, 31);
    draw_quest(db, 18, 228, 31, bingo_get_magic_level() >= 1);
    draw_quest(db, 19, 245, 31, bingo_get_magic_level() >= 2);
    text_print("Def", 164, 48);
    draw_quest(db, 12, 190, 48, bingo_goal_is_complete(BINGO_GOAL_DOUBLE_DEFENSE));
    text_print("Agony", 209, 48);
    draw_quest(db, 9, 245, 48, bingo_goal_is_complete(BINGO_GOAL_STONE_OF_AGONY));
    text_print("Str", 164, 65);
    draw_upgrade(db, Z64_ITEM_GORONS_BRACELET, bingo_get_strength_level(), 190, 65);
    text_print("Scale", 209, 65);
    draw_upgrade(db, Z64_ITEM_SILVER_SCALE, bingo_get_scale_level(), 245, 65);

    text_print("Bottles", 164, 82);
    for (uint8_t i = 0; i < 4; i++) {
        uint8_t item = bingo_get_inventory_item(Z64_SLOT_BOTTLE_1 + i);
        bool occupied = item != (uint8_t)Z64_ITEM_NULL;
        draw_item(db, occupied ? item : Z64_ITEM_BOTTLE, 164 + i * 17, 99, occupied);
    }

    text_print("Trade", 164, 116);
    uint8_t child_trade = bingo_get_inventory_item(Z64_SLOT_CHILD_TRADE);
    uint8_t adult_trade = bingo_get_inventory_item(Z64_SLOT_ADULT_TRADE);
    bool child_trade_owned = child_trade != (uint8_t)Z64_ITEM_NULL;
    bool adult_trade_owned = adult_trade != (uint8_t)Z64_ITEM_NULL;
    draw_item(db, child_trade_owned ? child_trade : Z64_ITEM_KEATON_MASK, 228, 116, child_trade_owned);
    draw_item(db, adult_trade_owned ? adult_trade : Z64_ITEM_COJIRO, 245, 116, adult_trade_owned);

    text_print("Songs", 164, 133);
    static const uint8_t song_items[] = {
        Z64_ITEM_ZELDAS_LULLABY, Z64_ITEM_EPONAS_SONG, Z64_ITEM_SARIAS_SONG,
        Z64_ITEM_SUNS_SONG, Z64_ITEM_SONG_OF_TIME, Z64_ITEM_SONG_OF_STORMS,
        Z64_ITEM_MINUET, Z64_ITEM_BOLERO, Z64_ITEM_SERENADE, Z64_ITEM_REQUIEM, Z64_ITEM_NOCTURNE, Z64_ITEM_PRELUDE,
    };
    static const uint8_t song_colors[][3] = {
        { 0xFF, 0xFF, 0xFF }, { 0xFF, 0xFF, 0xFF }, { 0xFF, 0xFF, 0xFF },
        { 0xFF, 0xFF, 0xFF }, { 0xFF, 0xFF, 0xFF }, { 0xFF, 0xFF, 0xFF },
        { 0x4F, 0xE5, 0x7A }, { 0xFF, 0x64, 0x4D }, { 0x55, 0xBA, 0xFF },
        { 0xFF, 0xA9, 0x4F }, { 0xC7, 0x7C, 0xFF }, { 0xFF, 0xF4, 0x9B },
    };
    for (uint8_t i = 0; i < 12; i++) {
        gDPPipeSync(db->p++);
        gDPSetPrimColor(db->p++, 0, 0, song_colors[i][0], song_colors[i][1], song_colors[i][2],
                        bingo_item_owned(song_items[i]) ? 0xFF : 0x40);
        sprite_load(db, &song_note_sprite, 0, 1);
        sprite_draw(db, &song_note_sprite, 0, 164 + (i % 6) * 17, 151 + (i / 6) * 17, 10, 15);
    }

    pause_menu_end(db);
}
