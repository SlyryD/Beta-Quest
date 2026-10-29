#include "dungeon_info.h"

#include <stdbool.h>

#include "count_tracker.h"
#include "counts.h"
#include "gfx.h"
#include "save.h"
#include "text.h"
#include "util.h"
#include "z64.h"

#define PAUSE_ITEM 0

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

typedef struct {
    uint8_t count;
    char name[12];
} global_count_entry_t;

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
    COUNT_ICON_HEART,
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
    { GCOUNT_HEART, COUNT_ICON_HEART, "Hearts" },
    { GCOUNT_SOIL_SKULLTULA, COUNT_ICON_SKULLTULA, "Soil" },
};

static const int dungeons_length = array_size(dungeons);
static const int enemy_counts_length = array_size(enemy_counts);
static const int other_counts_length = array_size(other_counts);

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
    return get_tokens(dungeon_index);
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

static void format_count(char count[4], uint8_t value) {
    if (value >= 100) {
        count[0] = '0' + value / 100;
        count[1] = '0' + (value / 10) % 10;
        count[2] = '0' + value % 10;
        count[3] = '\0';
    } else if (value >= 10) {
        count[0] = '0' + value / 10;
        count[1] = '0' + value % 10;
        count[2] = '\0';
    } else {
        count[0] = '0' + value;
        count[1] = '\0';
    }
}

static void draw_rupee_icon(z64_disp_buf_t* db, int left, int top, uint8_t red, uint8_t green, uint8_t blue,
                            bool collected) {
    gDPPipeSync(db->p++);
    gDPSetPrimColor(db->p++, 0, 0, red, green, blue, collected ? 0xFF : 0x40);
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
            draw_rupee_icon(db, left + 1, top + 1, 0xFF, 0xD0, 0x30, true);
            break;
        case COUNT_ICON_SILVER_RUPEE:
            sprite_load(db, &key_rupee_clock_sprite, 1, 1);
            draw_rupee_icon(db, left + 1, top + 1, 0xC0, 0xD8, 0xE0, true);
            break;
        case COUNT_ICON_MILK:
            sprite_load(db, &items_sprite, 0x1A, 1);
            sprite_draw(db, &items_sprite, 0, left, top, 14, 14);
            break;
        case COUNT_ICON_HEART:
            sprite_load(db, &quest_items_sprite, 12, 1);
            sprite_draw(db, &quest_items_sprite, 0, left, top, 14, 14);
            break;
        case COUNT_ICON_SKULLTULA:
            sprite_load(db, &quest_items_sprite, 11, 1);
            sprite_draw(db, &quest_items_sprite, 0, left, top, 14, 14);
            break;
    }
}

void draw_dungeon_info(z64_disp_buf_t* db) {
    int draw = CFG_DUNGEON_INFO_ENABLE && z64_game.pause_ctxt.state == 6 &&
               z64_game.pause_ctxt.screen_idx == PAUSE_ITEM && !z64_game.pause_ctxt.changing &&
               z64_ctxt.input[0].raw.pad.a && !z64_ctxt.input[0].raw.pad.dl &&
               !z64_ctxt.input[0].raw.pad.dr && !z64_ctxt.input[0].raw.pad.du;
    if (!draw)
        return;

    db->p = db->buf;

    // Call setup display list

    gSPDisplayList(db->p++, &setup_db);

    // Set up dimensions

    int icon_size = 16;
    int padding = 1;
    int bg_width = padding + (7 * font_sprite.tile_w) + // dungeon names
                   padding +
                   (9 * icon_size) + // boss, skull, skull count, key, unused key count, total key count, bk, map, compass
                   padding;
    int bg_height = padding + (dungeons_length * (icon_size + padding));
    int bg_left = (Z64_SCREEN_WIDTH - bg_width) / 2;
    int bg_top = (Z64_SCREEN_HEIGHT - bg_height) / 2;

    int left = bg_left + padding;
    int start_top = bg_top + padding;

    // Draw background

    gDPSetCombineMode(db->p++, G_CC_PRIMITIVE, G_CC_PRIMITIVE);
    gDPSetPrimColor(db->p++, 0, 0, 0x00, 0x00, 0x00, 0xD0);
    gSPTextureRectangle(db->p++, bg_left << 2, bg_top << 2, (bg_left + bg_width) << 2, (bg_top + bg_height) << 2, 0, 0,
                        0, 1 << 10, 1 << 10);

    // Set the primary color to white for drawing sprites

    gDPPipeSync(db->p++);
    gDPSetCombineMode(db->p++, G_CC_MODULATEIA_PRIM, G_CC_MODULATEIA_PRIM);

    gDPSetPrimColor(db->p++, 0, 0, 0xFF, 0xFF, 0xFF, 0xFF);

    // Draw navigation hints

    int hint_width = 48;
    int hint_height = 18;
    int left_hint_left = 4;
    int hint_top = (Z64_SCREEN_HEIGHT - hint_height) / 2;
    int up_hint_top = hint_top - 24;
    int right_hint_left = Z64_SCREEN_WIDTH - hint_width - 4;

    gDPSetCombineMode(db->p++, G_CC_PRIMITIVE, G_CC_PRIMITIVE);
    gDPSetPrimColor(db->p++, 0, 0, 0x00, 0x00, 0x00, 0xB0);
    gSPTextureRectangle(db->p++, left_hint_left << 2, hint_top << 2, (left_hint_left + hint_width) << 2,
                        (hint_top + hint_height) << 2, 0, 0, 0, 1 << 10, 1 << 10);
    gSPTextureRectangle(db->p++, right_hint_left << 2, hint_top << 2, (right_hint_left + hint_width) << 2,
                        (hint_top + hint_height) << 2, 0, 0, 0, 1 << 10, 1 << 10);
    gSPTextureRectangle(db->p++, left_hint_left << 2, up_hint_top << 2, (left_hint_left + hint_width) << 2,
                        (up_hint_top + hint_height) << 2, 0, 0, 0, 1 << 10, 1 << 10);

    gDPPipeSync(db->p++);
    gDPSetCombineMode(db->p++, G_CC_MODULATEIA_PRIM, G_CC_MODULATEIA_PRIM);
    gDPSetPrimColor(db->p++, 0, 0, 0xFF, 0xFF, 0xFF, 0xFF);
    text_print("< Ct", left_hint_left + 8, hint_top + 2);
    text_print("Ar >", right_hint_left + 8, hint_top + 2);
    text_print("^ Ev", left_hint_left + 8, up_hint_top + 2);

    // Draw dungeon names

    for (int i = 0; i < dungeons_length; i++) {
        const dungeon_entry_t* d = &(dungeons[i]);
        int top = start_top + ((icon_size + padding) * i) + 1;
        if (count_tracker_is_dungeon_completed(d->index)) {
            text_print_blue(d->name, left, top);
        } else {
            text_print(d->name, left, top);
        }
    }

    left += (7 * font_sprite.tile_w) + padding;

    // Draw boss-clear skulls

    sprite_load(db, &linkhead_skull_sprite, 1, 1);

    for (int i = 0; i < dungeons_length; i++) {
        const dungeon_entry_t* dungeon = &dungeons[i];
        if (!count_tracker_is_boss_cleared(dungeon->index))
            continue;

        int top = start_top + ((icon_size + padding) * i);
        sprite_draw(db, &linkhead_skull_sprite, 0, left, top, icon_size, icon_size);
    }

    left += icon_size;

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

    // Draw small key counts

    for (int i = 0; i < dungeons_length; i++) {
        const dungeon_entry_t* d = &(dungeons[i]);
        if (!d->has_keys)
            continue;

        uint8_t current_keys = z64_file.dungeon_keys[d->index];
        if (current_keys == 0xFF) {
            current_keys = 0;
        }

        uint8_t total_keys = get_dungeon_small_keys(d->index);
        if (total_keys == 0xFF) {
            total_keys = 0;
        }

        char count[5] = "0(0)";
        count[0] += (current_keys % 10);
        count[2] += (total_keys % 10);
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

    left += icon_size;

    // Draw maps

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

    // Finish

    text_flush(db);

    gDPFullSync(db->p++);
    gSPEndDisplayList(db->p++);
}

static void draw_status_sprite(z64_disp_buf_t* db, sprite_t* sprite, int tile, int left, int top, int size,
                               bool complete, uint8_t red, uint8_t green, uint8_t blue) {
    gDPPipeSync(db->p++);
    gDPSetCombineMode(db->p++, G_CC_MODULATEIA_PRIM, G_CC_MODULATEIA_PRIM);
    gDPSetPrimColor(db->p++, 0, 0, red, green, blue, complete ? 0xFF : 0x40);
    sprite_load(db, sprite, tile, 1);
    sprite_draw(db, sprite, 0, left, top, size, size);
}

static void draw_status_text(const char* text, int left, int top, bool complete) {
    if (complete) {
        text_print_blue(text, left, top);
    } else {
        text_print(text, left, top);
    }
}

static void draw_event_skull(z64_disp_buf_t* db, int left, int top, uint8_t objective, uint8_t red, uint8_t green,
                             uint8_t blue) {
    draw_status_sprite(db, &linkhead_skull_sprite, 1, left, top, 14, count_objective_is_complete(objective), red,
                       green, blue);
}

void draw_dungeon_events_info(z64_disp_buf_t* db) {
    int draw = CFG_DUNGEON_INFO_ENABLE && z64_game.pause_ctxt.state == 6 &&
               z64_game.pause_ctxt.screen_idx == PAUSE_ITEM && !z64_game.pause_ctxt.changing &&
               z64_ctxt.input[0].raw.pad.a && !z64_ctxt.input[0].raw.pad.dl &&
               !z64_ctxt.input[0].raw.pad.dr && z64_ctxt.input[0].raw.pad.du;
    if (!draw) {
        return;
    }

    static const char* labels[] = {
        "Dodongo", "Jabu", "Forest", "Fire", "Water", "Spirit",
        "Ganon",   "Trials", "Child GS", "Adult GS", "Sky",
    };
    static const uint8_t trial_colors[][3] = {
        { 0xFF, 0xF4, 0x9B }, { 0x4F, 0xE5, 0x7A }, { 0xFF, 0x64, 0x4D },
        { 0x55, 0xBA, 0xFF }, { 0xFF, 0xA9, 0x4F }, { 0xC7, 0x7C, 0xFF },
    };
    static const uint8_t child_dungeons[] = {
        SCENE_DEKU_TREE,
        SCENE_DODONGOS_CAVERN,
        SCENE_JABU_JABU,
    };
    static const uint8_t adult_dungeons[] = {
        SCENE_FOREST_TEMPLE,
        SCENE_FIRE_TEMPLE,
        SCENE_WATER_TEMPLE,
        SCENE_SHADOW_TEMPLE,
        SCENE_SPIRIT_TEMPLE,
    };
    static const uint8_t adult_medals[] = { 0, 1, 2, 4, 3 };
    static const uint8_t adult_colors[] = { 1, 2, 3, 5, 4 };

    db->p = db->buf;
    gSPDisplayList(db->p++, &setup_db);

    int bg_left = 26;
    int bg_top = 20;
    int bg_width = 268;
    int padding = 6;
    int title_height = 18;
    int row_height = 17;
    int bg_height = padding * 2 + title_height + array_size(labels) * row_height;
    int label_left = bg_left + padding;
    int indicator_left = label_left + 68;

    gDPSetCombineMode(db->p++, G_CC_PRIMITIVE, G_CC_PRIMITIVE);
    gDPSetPrimColor(db->p++, 0, 0, 0, 0, 0, 0xD0);
    gSPTextureRectangle(db->p++, bg_left << 2, bg_top << 2, (bg_left + bg_width) << 2, (bg_top + bg_height) << 2, 0,
                        0, 0, 1 << 10, 1 << 10);

    gDPPipeSync(db->p++);
    gDPSetCombineMode(db->p++, G_CC_MODULATEIA_PRIM, G_CC_MODULATEIA_PRIM);
    gDPSetPrimColor(db->p++, 0, 0, 0xFF, 0xFF, 0xFF, 0xFF);
    text_print("Dungeon Events", label_left, bg_top + padding);

    for (int i = 0; i < array_size(labels); i++) {
        text_print(labels[i], label_left, bg_top + padding + title_height + i * row_height);
    }

    int top = bg_top + padding + title_height;
    draw_event_skull(db, indicator_left, top, OBJECTIVE_DODONGO_LIZALFOS, 0xFF, 0xFF, 0xFF);

    top += row_height;
    draw_event_skull(db, indicator_left, top, OBJECTIVE_BIG_OCTO, 0xFF, 0xFF, 0xFF);
    text_print("Tentacles", indicator_left + 17, top);
    char count[4];
    format_count(count, get_global_count(GCOUNT_TENTACLE));
    text_print(count, indicator_left + 17 + 9 * font_sprite.tile_w, top);

    top += row_height;
    draw_event_skull(db, indicator_left, top, OBJECTIVE_POE_AMY, 0x4C, 0xFF, 0x69);
    draw_event_skull(db, indicator_left + 17, top, OBJECTIVE_POE_JOELLE, 0xFF, 0x64, 0x64);
    draw_event_skull(db, indicator_left + 34, top, OBJECTIVE_POE_BETH, 0x65, 0xA8, 0xFF);
    draw_event_skull(db, indicator_left + 51, top, OBJECTIVE_POE_MEG, 0xD0, 0x6A, 0xFF);
    draw_status_sprite(db, &quest_items_sprite, 14, indicator_left + 70, top, 14,
                       count_objective_is_complete(OBJECTIVE_FOREST_BOSS_DOOR), 0xFF, 0xFF, 0xFF);
    text_print("Door", indicator_left + 85, top);

    top += row_height;
    draw_event_skull(db, indicator_left, top, OBJECTIVE_FIRE_FLARE_DANCERS, 0xFF, 0xFF, 0xFF);
    int fire_left = indicator_left + 17;
    if (!z64_file.master_quest) {
        draw_status_sprite(db, &key_rupee_clock_sprite, 1, fire_left, top + 1, 12,
                           count_objective_is_complete(OBJECTIVE_FIRE_GOLD_RUPEE), 0xFF, 0xD0, 0x30);
        fire_left += 17;
    }
    draw_status_sprite(db, &items_sprite, 0x28, fire_left, top, 14, true, 0xFF, 0xFF, 0xFF);
    format_count(count, get_global_count(GCOUNT_CARPENTER));
    text_print(count, fire_left + 17, top);

    top += row_height;
    draw_event_skull(db, indicator_left, top, OBJECTIVE_DARK_LINK, 0xFF, 0xFF, 0xFF);

    top += row_height;
    draw_event_skull(db, indicator_left, top, OBJECTIVE_NABOORU_KNUCKLE, 0xFF, 0xFF, 0xFF);
    draw_status_sprite(db, &items_sprite, 0x09, indicator_left + 17, top, 14,
                       count_objective_is_complete(OBJECTIVE_SPIRIT_BOMBCHU_CHEST), 0xFF, 0xFF, 0xFF);
    draw_status_text("Plat", indicator_left + 34, top, count_objective_is_complete(OBJECTIVE_SPIRIT_PLATFORM));
    draw_status_sprite(db, &items_sprite, 0x11, indicator_left + 68, top, 14,
                       count_completed_objectives(OBJECTIVE_SPIRIT_RUSTY_SWITCH_1, 2) == 2, 0xFF, 0xFF, 0xFF);
    format_count(count, count_completed_objectives(OBJECTIVE_SPIRIT_RUSTY_SWITCH_1, 2));
    text_print(count, indicator_left + 85, top);

    top += row_height;
    draw_status_sprite(db, &quest_items_sprite, 14, indicator_left, top, 14,
                       count_objective_is_complete(OBJECTIVE_GANON_BOSS_DOOR), 0xFF, 0xFF, 0xFF);
    text_print("Door", indicator_left + 15, top);
    text_print("Beams", indicator_left + 52, top);
    format_count(count, get_global_count(GCOUNT_TRIAL_BEAMS));
    text_print(count, indicator_left + 97, top);

    top += row_height;
    for (int i = 0; i < 6; i++) {
        draw_status_sprite(db, &medals_sprite, i, indicator_left + i * 17, top, 14, get_trial_beam_cleared(i),
                           trial_colors[i][0], trial_colors[i][1], trial_colors[i][2]);
    }

    top += row_height;
    for (int i = 0; i < array_size(child_dungeons); i++) {
        draw_status_sprite(db, &stones_sprite, i, indicator_left + i * 17, top, 14,
                           get_dungeon_tokens(child_dungeons[i]) > 0, 0xFF, 0xFF, 0xFF);
    }

    top += row_height;
    for (int i = 0; i < array_size(adult_dungeons); i++) {
        int color = adult_colors[i];
        draw_status_sprite(db, &medals_sprite, adult_medals[i], indicator_left + i * 17, top, 14,
                           get_dungeon_tokens(adult_dungeons[i]) > 0, trial_colors[color][0],
                           trial_colors[color][1], trial_colors[color][2]);
    }

    top += row_height;
    draw_event_skull(db, indicator_left, top, OBJECTIVE_SKY_TEMPLE, 0xFF, 0xFF, 0xFF);

    gDPPipeSync(db->p++);
    gDPSetPrimColor(db->p++, 0, 0, 0xFF, 0xFF, 0xFF, 0xFF);
    text_flush(db);
    gDPFullSync(db->p++);
    gSPEndDisplayList(db->p++);
}

void draw_counts_info(z64_disp_buf_t* db) {
    int draw = CFG_DUNGEON_INFO_ENABLE && z64_game.pause_ctxt.state == 6 &&
               z64_game.pause_ctxt.screen_idx == PAUSE_ITEM && !z64_game.pause_ctxt.changing &&
               z64_ctxt.input[0].raw.pad.a && z64_ctxt.input[0].raw.pad.dl;
    if (!draw) {
        return;
    }

    db->p = db->buf;
    gSPDisplayList(db->p++, &setup_db);

    int padding = 5;
    int row_height = 16;
    int row_count = other_counts_length;
    int bg_width = 280;
    int bg_height = padding * 2 + (row_count + 1) * row_height;
    int bg_left = 20;
    int bg_top = (Z64_SCREEN_HEIGHT - bg_height) / 2;
    int enemy_left = bg_left + padding;
    int enemy_value_left = enemy_left + 88;
    int other_icon_left = bg_left + 136;
    int other_label_left = other_icon_left + 18;
    int other_value_left = other_label_left + 72;

    gDPSetCombineMode(db->p++, G_CC_PRIMITIVE, G_CC_PRIMITIVE);
    gDPSetPrimColor(db->p++, 0, 0, 0x00, 0x00, 0x00, 0xD0);
    gSPTextureRectangle(db->p++, bg_left << 2, bg_top << 2, (bg_left + bg_width) << 2, (bg_top + bg_height) << 2, 0, 0,
                        0, 1 << 10, 1 << 10);

    gDPPipeSync(db->p++);
    gDPSetCombineMode(db->p++, G_CC_MODULATEIA_PRIM, G_CC_MODULATEIA_PRIM);
    gDPSetPrimColor(db->p++, 0, 0, 0xFF, 0xFF, 0xFF, 0xFF);

    text_print("Enemies", enemy_left, bg_top + padding);
    text_print("Other", other_icon_left, bg_top + padding);

    for (int i = 0; i < enemy_counts_length; i++) {
        const global_count_entry_t* entry = &enemy_counts[i];
        char count[4];
        int top = bg_top + padding + (i + 1) * row_height;

        text_print(entry->name, enemy_left, top);
        format_count(count, get_global_count(entry->count));
        text_print(count, enemy_value_left, top);
    }

    for (int i = 0; i < other_counts_length; i++) {
        const other_count_entry_t* entry = &other_counts[i];
        char count[4];
        int top = bg_top + padding + (i + 1) * row_height;

        draw_count_icon(db, entry->icon, other_icon_left, top);
        text_print(entry->name, other_label_left, top);
        format_count(count, get_global_count(entry->count));
        text_print(count, other_value_left, top);
    }

    gDPPipeSync(db->p++);
    gDPSetPrimColor(db->p++, 0, 0, 0xFF, 0xFF, 0xFF, 0xFF);

    text_flush(db);
    gDPFullSync(db->p++);
    gSPEndDisplayList(db->p++);
}
