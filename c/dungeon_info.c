#include "dungeon_info.h"

#include "gfx.h"
#include "text.h"
#include "util.h"
#include "world_map_info.h"
#include "z64.h"

#define PAUSE_ITEM 0

#define HAS_ICE_CAVERN_HP \
    ((z64_game.scene_index == ICE_CAVERN && z64_game.collect_flags & (1 << 0x01)) || z64_file.scene_flags[ICE_CAVERN].collect & (1 << 0x01))

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
    {DEKU_TREE, 0, 0, 0, 1, 1, "Deku"},
    {DODONGOS_CAVERN, 0, 0, 0, 1, 1, "Dodongo"},
    {JABU_JABU, 0, 0, 0, 1, 1, "Jabu"},

    {FOREST_TEMPLE, 1, 1, 0, 1, 1, "Forest"},
    {FIRE_TEMPLE, 1, 1, 0, 1, 1, "Fire"},
    {WATER_TEMPLE, 1, 1, 0, 1, 1, "Water"},
    {SHADOW_TEMPLE, 1, 1, 0, 1, 1, "Shadow"},
    {SPIRIT_TEMPLE, 1, 1, 0, 1, 1, "Spirit"},

    {BOTTOM_OF_THE_WELL, 1, 0, 0, 1, 1, "BotW"},
    {ICE_CAVERN, 0, 0, 0, 1, 1, "Ice"},
    {THIEVES_HIDEOUT, 1, 0, 1, 0, 0, "Hideout"},
    {GERUDO_TRAINING_GROUND, 1, 0, 0, 0, 0, "GTG"},
    {INSIDE_GANONS_CASTLE, 1, 1, 0, 0, 0, "Ganon"},
};

int dungeon_count = array_size(dungeons);

extern uint32_t CFG_DUNGEON_INFO_ENABLE;

void draw_dungeon_info(z64_disp_buf_t *db)
{
    int draw = CFG_DUNGEON_INFO_ENABLE &&
               z64_game.pause_ctxt.state == 6 &&
               z64_game.pause_ctxt.screen_idx == PAUSE_ITEM &&
               !z64_game.pause_ctxt.changing &&
               z64_ctxt.input[0].raw.pad.a;
    if (!draw)
        return;

    db->p = db->buf;

    // Call setup display list

    gSPDisplayList(db->p++, &setup_db);

    // Set up dimensions

    int icon_size = 16;
    int padding = 1;
    int bg_width =
        padding +
        (8 * font_sprite.tile_w) + // dungeon names
        padding +
        (7 * (icon_size + padding)); // skull, skull count, key, key count, bk, map, compass
    int bg_height = padding + (dungeon_count * (icon_size + padding));
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

    // Set the primary color to white for drawing sprites

    gDPPipeSync(db->p++);
    gDPSetCombineMode(db->p++, G_CC_MODULATEIA_PRIM, G_CC_MODULATEIA_PRIM);

    gDPSetPrimColor(db->p++, 0, 0, 0xFF, 0xFF, 0xFF, 0xFF);

    // Draw dungeon names

    for (int i = 0; i < dungeon_count; i++)
    {
        dungeon_entry_t *d = &(dungeons[i]);
        int top = start_top + ((icon_size + padding) * i) + 1;
        text_print(d->name, left, top);
    }

    left += (8 * font_sprite.tile_w) + padding;

    // Draw skull sprite

    sprite_load(db, &quest_items_sprite, 11, 1);

    for (int i = 0; i < dungeon_count; i++)
    {
        dungeon_entry_t *d = &(dungeons[i]);
        if (!d->has_tokens)
            continue;

        int top = start_top + ((icon_size + padding) * i);
        sprite_draw(db, &quest_items_sprite, 0,
                    left, top, icon_size, icon_size);
    }

    left += icon_size + padding;

    // Draw skull count

    for (int i = 0; i < dungeon_count; i++)
    {
        dungeon_entry_t *d = &(dungeons[i]);
        if (!d->has_tokens)
            continue;

        int8_t tokens = get_tokens(d->index);

        char count[2] = "0";
        count[0] += (tokens % 10);
        int top = start_top + ((icon_size + padding) * i) + 1;
        text_print(count, left, top);
    }

    left += icon_size + padding;

    // Draw key sprite

    sprite_load(db, &key_rupee_clock_sprite, 0, key_rupee_clock_sprite.tile_count);

    for (int i = 0; i < dungeon_count; i++)
    {
        dungeon_entry_t *d = &(dungeons[i]);
        if (!d->has_keys)
            continue;

        int top = start_top + ((icon_size + padding) * i);
        sprite_draw(db, &key_rupee_clock_sprite, 0,
                    left, top, icon_size, icon_size);
    }

    // Draw hp sprite

    sprite_load(db, &quest_items_sprite, 13, 1);

    for (int i = 0; i < world_map_area_count; i++)
    {
        dungeon_entry_t *d = &(dungeons[i]);
        if (d->index != ICE_CAVERN)
            continue;

        int top = start_top + ((icon_size + padding) * i);
        sprite_draw(db, &quest_items_sprite, 0,
                    left, top, icon_size, icon_size);
    }

    left += icon_size + padding;

    // Draw small key counts

    for (int i = 0; i < dungeon_count; i++)
    {
        dungeon_entry_t *d = &(dungeons[i]);
        if (!d->has_keys)
            continue;

        int8_t keys = z64_file.dungeon_keys[d->index];
        if (keys < 0)
            keys = 0;

        char count[2] = "0";
        count[0] += (keys % 10);
        int top = start_top + ((icon_size + padding) * i) + 1;
        text_print(count, left, top);
    }

    // Draw hp count

    for (int i = 0; i < dungeon_count; i++)
    {
        dungeon_entry_t *d = &(dungeons[i]);
        if (d->index != ICE_CAVERN)
            continue;

        int8_t hps = 0;
        if (HAS_ICE_CAVERN_HP)
            hps = 1;

        char count[2] = "0";
        count[0] += (hps % 10);
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

    // Finish

    text_flush(db);

    gDPFullSync(db->p++);
    gSPEndDisplayList(db->p++);
}
