#include "file_select.h"

#include "gfx.h"
#include "memory.h"
#include "save.h"
#include "text.h"

#define FILE_SELECT_MAIN_TO_OPTIONS_MODE 36
#define FILE_SELECT_OPTIONS_MODE 37
#define FILE_SELECT_START_OPTIONS_MODE 38
#define FILE_SELECT_OPTIONS_TO_MAIN_MODE 39
#define FILE_SELECT_OVERLAY_INDEX 5
#define FILE_SELECT_DRAW_NAME_ENTRY_OFFSET 0x4E88
#define FILE_SELECT_DRAW_WINDOW_CONTENTS_OFFSET 0xB2F8
#define FILE_SELECT_UPDATE_KEYBOARD_OFFSET 0x6398
#define FILE_SELECT_DRAW_OPTIONS_OFFSET 0x7080
#define FILE_SELECT_ROTATE_TO_NAME_ENTRY_MODE 32
#define FILE_SELECT_START_NAME_ENTRY_MODE 34
#define FILE_SELECT_SELECTED_SETTING_OFFSET 0xFF30
#define FILE_SELECT_SONG_SPEEDUP_SETTING 2
#define FILE_SELECT_BONKO_SETTING 3
#define FILE_SELECT_SETTING_COUNT 4
#define FILE_SELECT_NAME_ENTRY_MODE 33
#define FILE_SELECT_NAME_ENTRY_TO_MAIN_MODE 35
#define BONKO_OPTIONS_VERSION 1
#define SONG_SPEEDUP_OPTIONS_VERSION 1
#define FILE_SELECT_CURSOR_SFX 0x4839
#define FILE_SELECT_DECIDE_SFX 0x483B

typedef void (*sram_write_header_func_t)(void*);
typedef void (*sram_file_func_t)(void*);
typedef void (*audio_set_output_func_t)(int8_t);
typedef void (*play_sfx_func_t)(uint16_t, z64_xyzf_t*, int8_t, float*, float*, int8_t*);
typedef void (*draw_options_func_t)(z64_ctxt_t*);
typedef void (*file_select_func_t)(z64_ctxt_t*);

extern uint32_t CFG_DEADLY_BONKS;
uint32_t CFG_SONG_SPEEDUPS;

typedef struct {
    z64_ctxt_t state;
    char unk_00A4[0x0134];
    z64_sram_data_t* sram_buffer;
    char unk_01DC[0x1C810];
    uint8_t n64dd_flag;
    char unk_1C9ED;
    uint16_t deaths[3];
    uint8_t names[3][8];
    uint16_t hearts[3];
    uint32_t quest_items[3];
    uint16_t n64dd_flags[3];
    uint8_t defense[3];
    char unk_1CA29;
    int16_t button_index;
    char unk_1CA2C[0x0004];
    int16_t config_mode;
    char unk_1CA32[0x0012];
    int16_t button_y_offsets[6];
    char unk_1CA50[0x0022];
    uint16_t name_alpha[3];
    uint16_t connector_alpha[3];
    uint16_t file_info_alpha[3];
    char unk_1CA84[0x000A];
    uint16_t name_entry_alpha;
    char unk_1CA90[0x001C];
    int16_t stick_x;
    int16_t stick_y;
    char unk_1CAB0[0x0008];
    int16_t keyboard_button;
    int16_t char_page;
    int16_t char_bg_alpha;
    int16_t char_index;
    int16_t keyboard_x;
    int16_t keyboard_y;
    int16_t name_char_count;
    int16_t keyboard_return_x[5];
} file_select_t;

static sram_write_header_func_t const write_sram_header = (sram_write_header_func_t)0x800911C0;
static sram_file_func_t const write_save = (sram_file_func_t)0x800905D4;
static audio_set_output_func_t const set_audio_output = (audio_set_output_func_t)0x800C7420;
static play_sfx_func_t const play_sfx = (play_sfx_func_t)0x800C806C;
static uint8_t selected_setting;
static uint8_t custom_options_alpha;
static uint8_t create_mq_file;
static uint8_t create_mq_alpha;
static char const option_labels[2][4] = { "OFF", "ON" };

typedef char assert_bonko_option_offset[__builtin_offsetof(z64_sram_data_t, bonko_options) == 0x0C ? 1 : -1];
typedef char
    assert_song_speedup_option_offset[__builtin_offsetof(z64_sram_data_t, song_speedup_options) == 0x0E ? 1 : -1];
typedef char assert_sram_save_offset[__builtin_offsetof(z64_sram_data_t, primary_saves) == 0x20 ? 1 : -1];
typedef char assert_file_select_sram_offset[__builtin_offsetof(file_select_t, sram_buffer) == 0x1D8 ? 1 : -1];
typedef char assert_file_select_mode_offset[__builtin_offsetof(file_select_t, config_mode) == 0x1CA30 ? 1 : -1];
typedef char assert_file_select_stick_offset[__builtin_offsetof(file_select_t, stick_x) == 0x1CAAC ? 1 : -1];
typedef char assert_file_select_n64dd_offset[__builtin_offsetof(file_select_t, n64dd_flag) == 0x1C9EC ? 1 : -1];
typedef char assert_file_select_button_offset[__builtin_offsetof(file_select_t, button_index) == 0x1CA2A ? 1 : -1];
typedef char assert_file_select_keyboard_offset[__builtin_offsetof(file_select_t, keyboard_button) == 0x1CAB8 ? 1 : -1];
typedef char assert_file_select_keyboard_x_offset[__builtin_offsetof(file_select_t, keyboard_x) == 0x1CAC0 ? 1 : -1];
typedef char
    assert_file_select_name_entry_alpha_offset[__builtin_offsetof(file_select_t, name_entry_alpha) == 0x1CA8E ? 1 : -1];
void update_mq_name_entry(z64_ctxt_t* menu) {
    file_select_t* file_select = (file_select_t*)menu;
    uintptr_t overlay_base = (uintptr_t)z64_state_ovl_tab[FILE_SELECT_OVERLAY_INDEX].ptr;
    file_select_func_t update_keyboard_cursor = (file_select_func_t)(overlay_base + FILE_SELECT_UPDATE_KEYBOARD_OFFSET);
    int16_t previous_x = file_select->keyboard_x;
    int16_t previous_y = file_select->keyboard_y;
    int16_t adjusted_x = file_select->stick_x;
    int16_t adjusted_y = file_select->stick_y;
    int32_t selected_mq = 0;

    if (menu->input[0].pad_pressed.a) {
        if (file_select->keyboard_button == 2) {
            selected_mq = 1;
            create_mq_file = 1;
            file_select->keyboard_x = 4;
            file_select->keyboard_button = 4;
        } else if (file_select->keyboard_button == 4) {
            create_mq_file = 0;
        }
    }

    update_keyboard_cursor(menu);

    if (file_select->config_mode != FILE_SELECT_NAME_ENTRY_MODE) {
        return;
    }

    if (selected_mq && file_select->name_char_count == 0) {
        create_mq_file = 0;
        file_select->keyboard_x = 2;
        file_select->keyboard_y = 5;
        file_select->keyboard_button = 2;
    } else if (previous_y == 0 && (previous_x == 6 || previous_x == 7) && adjusted_y > 30) {
        file_select->keyboard_return_x[2] = previous_x;
        file_select->keyboard_x = 2;
        file_select->keyboard_y = 5;
    } else if (previous_y == 4 && (previous_x == 6 || previous_x == 7) && adjusted_y < -30) {
        file_select->keyboard_return_x[2] = previous_x;
        file_select->keyboard_x = 2;
        file_select->keyboard_y = 5;
    } else if (file_select->keyboard_button == 4 && previous_x == 3 && adjusted_x < -30) {
        file_select->keyboard_x = 2;
    } else if (file_select->keyboard_button == 3 && previous_x == 4 && adjusted_x > 30) {
        file_select->keyboard_x = 2;
    }
    if (file_select->keyboard_y == 5 && file_select->keyboard_x == 2) {
        file_select->keyboard_button = 2;
    }
}

void create_file_with_mq_config(z64_ctxt_t* menu, void* sram_context) {
    file_select_t* file_select = (file_select_t*)menu;
    typedef void (*init_save_func_t)(z64_ctxt_t*, void*);
    init_save_func_t init_save = (init_save_func_t)0x80090C18;
    int16_t file_index;

    init_save(menu, sram_context);
    count_save_initialize_file(z64_file.file_index, file_select->sram_buffer);
    if (!create_mq_file) {
        return;
    }

    z64_file.master_quest = 1;
    write_save(sram_context);

    file_index = z64_file.file_index;
    memory_copy(&file_select->sram_buffer->primary_saves[file_index], &z64_file, sizeof(z64_file));
    memory_copy(&file_select->sram_buffer->backup_saves[file_index], &z64_file, sizeof(z64_file));
    create_mq_file = 0;
}

void draw_file_select_mq(z64_ctxt_t* menu) {
    file_select_t* file_select = (file_select_t*)menu;
    uintptr_t overlay_base = (uintptr_t)z64_state_ovl_tab[FILE_SELECT_OVERLAY_INDEX].ptr;
    file_select_func_t draw_name_entry = (file_select_func_t)(overlay_base + FILE_SELECT_DRAW_NAME_ENTRY_OFFSET);
    z64_disp_buf_t* db;

    draw_name_entry(menu);

    if (file_select->config_mode == FILE_SELECT_ROTATE_TO_NAME_ENTRY_MODE) {
        create_mq_alpha = create_mq_alpha < 0xFF - 31 ? create_mq_alpha + 31 : 0xFF;
    } else if (file_select->config_mode == FILE_SELECT_NAME_ENTRY_MODE ||
               file_select->config_mode == FILE_SELECT_START_NAME_ENTRY_MODE) {
        create_mq_alpha = 0xFF;
    } else if (file_select->config_mode == FILE_SELECT_NAME_ENTRY_TO_MAIN_MODE) {
        create_mq_alpha = create_mq_alpha > 31 ? create_mq_alpha - 31 : 0;
    } else {
        return;
    }

    db = &menu->gfx->poly_opa;
    gSPDisplayList(db->p++, &setup_db);
    gDPPipeSync(db->p++);
    gDPSetCombineMode(db->p++, G_CC_MODULATEIA_PRIM, G_CC_MODULATEIA_PRIM);

    if (create_mq_alpha != 0) {
        if (file_select->config_mode == FILE_SELECT_NAME_ENTRY_MODE && file_select->keyboard_button == 2) {
            gDPSetPrimColor(db->p++, 0, 0, 0xFF, 0xFF, 0, create_mq_alpha);
        } else {
            gDPSetPrimColor(db->p++, 0, 0, 0xFF, 0xFF, 0xFF, create_mq_alpha);
        }
        text_print("MQ", 168, 170);
        text_flush(db);
    }

    gDPPipeSync(db->p++);
}

void draw_file_select_mq_markers(z64_ctxt_t* menu) {
    file_select_t* file_select = (file_select_t*)menu;
    uintptr_t overlay_base = (uintptr_t)z64_state_ovl_tab[FILE_SELECT_OVERLAY_INDEX].ptr;
    file_select_func_t draw_window_contents =
        (file_select_func_t)(overlay_base + FILE_SELECT_DRAW_WINDOW_CONTENTS_OFFSET);
    z64_disp_buf_t* db;
    int32_t i;

    draw_window_contents(menu);
    if (file_select->config_mode >= FILE_SELECT_ROTATE_TO_NAME_ENTRY_MODE &&
        file_select->config_mode <= FILE_SELECT_OPTIONS_TO_MAIN_MODE) {
        return;
    }

    db = &menu->gfx->poly_opa;
    gSPDisplayList(db->p++, &setup_db);
    gDPPipeSync(db->p++);
    gDPSetCombineMode(db->p++, G_CC_MODULATEIA_PRIM, G_CC_MODULATEIA_PRIM);

    for (i = 0; i < 3; i++) {
        uint16_t alpha = file_select->name_alpha[i];
        int32_t y;

        if (file_select->file_info_alpha[i] > alpha) {
            alpha = file_select->file_info_alpha[i];
        }
        if (file_select->sram_buffer->primary_saves[i].master_quest != 1 || alpha == 0) {
            continue;
        }

        y = 76 + i * 16 - file_select->button_y_offsets[i];
        gDPSetPrimColor(db->p++, 0, 0, 0xFF, 0xFF, 0xFF, alpha);
        text_print("MQ", 230, y);
        text_flush(db);
    }

    gDPPipeSync(db->p++);
}

static uint8_t* get_bonko_option(file_select_t* menu) {
    uintptr_t sram_buffer = menu == NULL ? 0 : (uintptr_t)menu->sram_buffer;
    if (menu == NULL || menu->config_mode < 0 || menu->config_mode > 40 || sram_buffer < 0x80000000 ||
        sram_buffer >= 0x80800000) {
        return NULL;
    }
    return &menu->sram_buffer->bonko_options;
}

static uint8_t* get_native_selected_setting() {
    uintptr_t overlay_base = (uintptr_t)z64_state_ovl_tab[FILE_SELECT_OVERLAY_INDEX].ptr;
    return (uint8_t*)(overlay_base + FILE_SELECT_SELECTED_SETTING_OFFSET);
}

static void play_file_select_sfx(uint16_t sfx) {
    play_sfx(sfx, (z64_xyzf_t*)0x80104394, 4, (float*)0x801043A0, (float*)0x801043A0, (int8_t*)0x801043A8);
}

static const char* get_option_label(uint8_t enabled) {
    return option_labels[enabled != 0];
}

static void normalize_file_select_settings(z64_sram_data_t* sram_buffer) {
    if (sram_buffer->bonko_options_version != BONKO_OPTIONS_VERSION) {
        sram_buffer->bonko_options = 0;
        sram_buffer->bonko_options_version = BONKO_OPTIONS_VERSION;
    } else if (sram_buffer->bonko_options != 1) {
        sram_buffer->bonko_options = 0;
    }
    CFG_DEADLY_BONKS = sram_buffer->bonko_options;

    if (sram_buffer->song_speedup_options_version != SONG_SPEEDUP_OPTIONS_VERSION) {
        sram_buffer->song_speedup_options = 0;
        sram_buffer->song_speedup_options_version = SONG_SPEEDUP_OPTIONS_VERSION;
    } else if (sram_buffer->song_speedup_options != 1) {
        sram_buffer->song_speedup_options = 0;
    }
    CFG_SONG_SPEEDUPS = sram_buffer->song_speedup_options;
}

void load_file_select_settings(z64_sram_data_t* sram_buffer) {
    count_save_cache_sram(sram_buffer);
    normalize_file_select_settings(sram_buffer);
}

void update_file_select_options(z64_ctxt_t* menu) {
    z64_input_t* input = &menu->input[0];
    file_select_t* file_select = (file_select_t*)menu;
    uint8_t* bonko_option = get_bonko_option(file_select);
    int16_t adjusted_x = file_select->stick_x;
    int16_t adjusted_y = file_select->stick_y;

    if (bonko_option != NULL) {
        normalize_file_select_settings(file_select->sram_buffer);
    }

    if (input->pad_pressed.b) {
        play_file_select_sfx(FILE_SELECT_DECIDE_SFX);
        file_select->config_mode = 39;
        file_select->sram_buffer->sound_options = z64_file.sound_setting;
        file_select->sram_buffer->z_target_options = z64_file.z_targeting;
        write_sram_header(&file_select->sram_buffer);
        set_audio_output(z64_file.sound_setting);
        return;
    }

    if (adjusted_x < -30 || adjusted_x > 30) {
        play_file_select_sfx(FILE_SELECT_CURSOR_SFX);
        if (selected_setting == 0) {
            if (adjusted_x < -30) {
                z64_file.sound_setting = z64_file.sound_setting == 0 ? 3 : z64_file.sound_setting - 1;
            } else {
                z64_file.sound_setting = z64_file.sound_setting == 3 ? 0 : z64_file.sound_setting + 1;
            }
        } else if (selected_setting == 1) {
            z64_file.z_targeting ^= 1;
        } else if (selected_setting == FILE_SELECT_BONKO_SETTING && bonko_option != NULL) {
            *bonko_option ^= 1;
            CFG_DEADLY_BONKS = *bonko_option;
        } else if (bonko_option != NULL) {
            file_select->sram_buffer->song_speedup_options ^= 1;
            CFG_SONG_SPEEDUPS = file_select->sram_buffer->song_speedup_options;
        }
    }

    if (adjusted_y < -30) {
        play_file_select_sfx(FILE_SELECT_CURSOR_SFX);
        selected_setting++;
        if (selected_setting == FILE_SELECT_SETTING_COUNT) {
            selected_setting = 0;
        }
    } else if (adjusted_y > 30) {
        play_file_select_sfx(FILE_SELECT_CURSOR_SFX);
        selected_setting = selected_setting == 0 ? FILE_SELECT_SETTING_COUNT - 1 : selected_setting - 1;
    } else if (input->pad_pressed.a) {
        play_file_select_sfx(FILE_SELECT_DECIDE_SFX);
        selected_setting++;
        if (selected_setting == FILE_SELECT_SETTING_COUNT) {
            selected_setting = 0;
        }
    }

    *get_native_selected_setting() = selected_setting;
}

void draw_file_select_options(z64_ctxt_t* menu) {
    file_select_t* file_select = (file_select_t*)menu;
    uintptr_t overlay_base = (uintptr_t)z64_state_ovl_tab[FILE_SELECT_OVERLAY_INDEX].ptr;
    draw_options_func_t draw_native_options = (draw_options_func_t)(overlay_base + FILE_SELECT_DRAW_OPTIONS_OFFSET);
    uint8_t* bonko_option;
    z64_disp_buf_t* db;

    draw_native_options(menu);

    if (file_select->config_mode == FILE_SELECT_MAIN_TO_OPTIONS_MODE) {
        custom_options_alpha = custom_options_alpha < 0xFF - 31 ? custom_options_alpha + 31 : 0xFF;
    } else if (file_select->config_mode == FILE_SELECT_OPTIONS_MODE ||
               file_select->config_mode == FILE_SELECT_START_OPTIONS_MODE) {
        custom_options_alpha = 0xFF;
    } else if (file_select->config_mode == FILE_SELECT_OPTIONS_TO_MAIN_MODE) {
        custom_options_alpha = custom_options_alpha > 31 ? custom_options_alpha - 31 : 0;
    } else {
        return;
    }

    bonko_option = get_bonko_option(file_select);
    if (bonko_option == NULL) {
        return;
    }
    CFG_DEADLY_BONKS = *bonko_option;
    CFG_SONG_SPEEDUPS = file_select->sram_buffer->song_speedup_options;

    db = &menu->gfx->poly_opa;
    gSPDisplayList(db->p++, &setup_db);
    gDPPipeSync(db->p++);
    gDPSetCombineMode(db->p++, G_CC_MODULATEIA_PRIM, G_CC_MODULATEIA_PRIM);
    gDPSetPrimColor(db->p++, 0, 0, 0xFF, 0xFF, 0xFF, custom_options_alpha);

    if (selected_setting == FILE_SELECT_SONG_SPEEDUP_SETTING) {
        text_print(">", 160, 138);
    }
    text_print("SONG SKIPS:", 168, 138);
    text_print(get_option_label(file_select->sram_buffer->song_speedup_options), 264, 138);
    if (selected_setting == FILE_SELECT_BONKO_SETTING) {
        text_print(">", 184, 156);
    }
    text_print("BONKO:", 192, 156);
    text_print(get_option_label(*bonko_option), 264, 156);
    text_flush(db);

    gDPPipeSync(db->p++);
}
