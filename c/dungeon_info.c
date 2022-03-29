#include "dungeon_info.h"

#include "gfx.h"
#include "text.h"
#include "util.h"
#include "z64.h"

uint32_t gGsFlagsMasks[] = {0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000};
uint32_t gGsFlagsShifts[] = {0, 8, 16, 24};

#define GET_GS_FLAGS(index) \
    ((z64_file.gs_flags[(index) >> 2] & gGsFlagsMasks[(index)&3]) >> gGsFlagsShifts[(index)&3])

typedef struct
{
    uint8_t index;
    struct
    {
        uint8_t has_keys : 1;
        uint8_t has_boss_key : 1;
        uint8_t has_card : 1;
        uint8_t has_map : 1;
        uint8_t has_tokens : 1;
    };
    char name[10];
} dungeon_entry_t;

dungeon_entry_t dungeons[] = {
    {0, 0, 0, 0, 1, 1, "Deku"},
    {1, 0, 0, 0, 1, 1, "Dodongo"},
    {2, 0, 0, 0, 1, 1, "Jabu"},

    {3, 1, 1, 0, 1, 1, "Forest"},
    {4, 1, 1, 0, 1, 1, "Fire"},
    {5, 1, 1, 0, 1, 1, "Water"},
    {7, 1, 1, 0, 1, 1, "Shadow"},
    {6, 1, 1, 0, 1, 1, "Spirit"},

    {8, 1, 0, 0, 1, 1, "BotW"},
    {9, 0, 0, 0, 1, 1, "Ice"},
    {12, 1, 0, 1, 0, 0, "Hideout"},
    {11, 1, 0, 0, 0, 0, "GTG"},
    {13, 1, 1, 0, 0, 0, "Ganon"},
};

int dungeon_count = array_size(dungeons);

typedef struct
{
    uint8_t r;
    uint8_t g;
    uint8_t b;
} medal_color_t;

medal_color_t medal_colors[] = {
    {0x00, 0xFF, 0x00}, // Forest
    {0xFF, 0x3C, 0x00}, // Fire
    {0x00, 0x64, 0xFF}, // Water
    {0xFF, 0x82, 0x00}, // Spirit
    {0xC8, 0x32, 0xFF}, // Shadow
    {0xC8, 0xC8, 0x00}, // Light
};

extern uint32_t CFG_DUNGEON_INFO_ENABLE;

int8_t CFG_DUNGEON_REWARDS[14] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13};

void draw_dungeon_info(z64_disp_buf_t *db)
{
    int draw = CFG_DUNGEON_INFO_ENABLE &&
               z64_game.pause_ctxt.state == 6 &&
               z64_game.pause_ctxt.screen_idx == 0 &&
               !z64_game.pause_ctxt.changing &&
               z64_ctxt.input[0].raw.pad.a;
    if (!draw)
    {
        return;
    }

    db->p = db->buf;

    // Call setup display list
    gSPDisplayList(db->p++, &setup_db);

    // Set up dimensions

    int icon_size = 16;
    int padding = 1;
    int rows = 13;
    int bg_width =
        (5 * icon_size) +
        (8 * font_sprite.tile_w) +
        (7 * padding) +
        (6 * font_sprite.tile_w) +
        padding;
    int bg_height = (rows * icon_size) + ((rows + 1) * padding);
    int bg_left = (Z64_SCREEN_WIDTH - bg_width) / 2;
    int bg_top = (Z64_SCREEN_HEIGHT - bg_height) / 2;

    int left = bg_left + padding;
    int start_top = bg_top + padding;

    // Draw background

    gDPSetCombineMode(db->p++, G_CC_PRIMITIVE, G_CC_PRIMITIVE);
    gDPSetPrimColor(db->p++, 0, 0, 0x00, 0x00, 0x00, 0xD0);
    gSPTextureRectangle(db->p++,
                        bg_left << 2, bg_top << 2,
                        (bg_left + bg_width) << 2, (bg_top + bg_height) << 2,
                        0,
                        0, 0,
                        1 << 10, 1 << 10);

    gDPPipeSync(db->p++);
    gDPSetCombineMode(db->p++, G_CC_MODULATEIA_PRIM, G_CC_MODULATEIA_PRIM);

    // Draw medals

    sprite_load(db, &medals_sprite, 0, medals_sprite.tile_count);

    for (int i = 0; i < dungeon_count; i++)
    {
        dungeon_entry_t *d = &(dungeons[i]);
        int reward = CFG_DUNGEON_REWARDS[d->index];
        if (reward < 3)
            continue;
        reward -= 3;

        medal_color_t *c = &(medal_colors[reward]);
        gDPSetPrimColor(db->p++, 0, 0, c->r, c->g, c->b, 0xFF);

        int top = start_top + ((icon_size + padding) * i);
        sprite_draw(db, &medals_sprite, reward,
                    left, top, icon_size, icon_size);
    }

    gDPSetPrimColor(db->p++, 0, 0, 0xFF, 0xFF, 0xFF, 0xFF);

    // Draw stones

    sprite_load(db, &stones_sprite, 0, stones_sprite.tile_count);

    for (int i = 0; i < dungeon_count; i++)
    {
        dungeon_entry_t *d = &(dungeons[i]);
        int reward = CFG_DUNGEON_REWARDS[d->index];
        if (reward < 0 || reward >= 3)
            continue;

        int top = start_top + ((icon_size + padding) * i);
        sprite_draw(db, &stones_sprite, reward,
                    left, top, icon_size, icon_size);
    }

    left += icon_size + padding;

    // Draw dungeon names

    for (int i = 0; i < dungeon_count; i++)
    {
        dungeon_entry_t *d = &(dungeons[i]);
        int top = start_top + ((icon_size + padding) * i) + 1;
        text_print(d->name, left, top);
    }

    left += (8 * font_sprite.tile_w) + padding;

    // Draw small key counts

    sprite_load(db, &quest_items_sprite, 17, 1);

    for (int i = 0; i < dungeon_count; i++)
    {
        dungeon_entry_t *d = &(dungeons[i]);
        if (!d->has_keys)
            continue;

        int8_t keys = z64_file.dungeon_keys[d->index];
        if (keys < 0)
            keys = 0;
        if (keys > 9)
            keys = 9;

        char count[2] = "0";
        count[0] += keys;
        int top = start_top + ((icon_size + padding) * i) + 1;
        text_print(count, left, top);
    }

    left += icon_size + padding;

    // Draw boss keys

    sprite_load(db, &quest_items_sprite, 14, 1);

    for (int i = 0; i < dungeon_count; i++)
    {
        dungeon_entry_t *d = &(dungeons[i]);
        // Replace index 13 (Ganon's Castle) with 10 (Ganon's Tower)
        int index = d->index == 13 ? 10 : d->index;

        if (d->has_boss_key && z64_file.dungeon_items[index].boss_key)
        {
            int top = start_top + ((icon_size + padding) * i);
            sprite_draw(db, &quest_items_sprite, 0,
                        left, top, icon_size, icon_size);
        }
    }

    // Draw gerudo card

    sprite_load(db, &quest_items_sprite, 10, 1);

    for (int i = 0; i < dungeon_count; i++)
    {
        dungeon_entry_t *d = &(dungeons[i]);
        if (d->has_card && z64_file.gerudos_card)
        {
            int top = start_top + ((icon_size + padding) * i);
            sprite_draw(db, &quest_items_sprite, 0,
                        left, top, icon_size, icon_size);
        }
    }

    left += icon_size + padding;

    // Draw maps

    sprite_load(db, &quest_items_sprite, 16, 1);

    for (int i = 0; i < dungeon_count; i++)
    {
        dungeon_entry_t *d = &(dungeons[i]);
        if (d->has_map && z64_file.dungeon_items[d->index].map)
        {
            int top = start_top + ((icon_size + padding) * i);
            sprite_draw(db, &quest_items_sprite, 0,
                        left, top, icon_size, icon_size);
        }
    }

    left += icon_size + padding;

    // Draw compasses

    sprite_load(db, &quest_items_sprite, 15, 1);

    for (int i = 0; i < dungeon_count; i++)
    {
        dungeon_entry_t *d = &(dungeons[i]);
        if (d->has_map && z64_file.dungeon_items[d->index].compass)
        {
            int top = start_top + ((icon_size + padding) * i);
            sprite_draw(db, &quest_items_sprite, 0,
                        left, top, icon_size, icon_size);
        }
    }

    left += icon_size + padding;

    // Draw dungeon skull count

    for (int i = 0; i < dungeon_count; i++)
    {
        dungeon_entry_t *d = &(dungeons[i]);
        if (!d->has_tokens)
            continue;

        int8_t tokens = GET_GS_FLAGS(i); // TODO: Get correct dungeon minimap_index
        if (tokens < 0)
            tokens = 0;
        if (tokens > 9)
            tokens = 9;

        char count[2] = "0";
        count[0] += tokens;
        int top = start_top + ((icon_size + padding) * i) + 1;
        text_print(count, left, top);
    }

    left += icon_size + padding;

    // Finish

    text_flush(db);

    gDPFullSync(db->p++);
    gSPEndDisplayList(db->p++);
}
