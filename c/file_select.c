#include "file_select.h"

#include "gfx.h"
#include "text.h"

#define FILE_SELECT_MAIN_TO_OPTIONS_MODE 36
#define FILE_SELECT_OPTIONS_MODE 37
#define FILE_SELECT_START_OPTIONS_MODE 38
#define FILE_SELECT_OPTIONS_TO_MAIN_MODE 39
#define FILE_SELECT_OVERLAY_INDEX 5
#define FILE_SELECT_DRAW_OPTIONS_OFFSET 0x7080
#define FILE_SELECT_SELECTED_SETTING_OFFSET 0xFF30
#define FILE_SELECT_SONG_SPEEDUP_SETTING 2
#define FILE_SELECT_BONKO_SETTING 3
#define FILE_SELECT_SETTING_COUNT 4
#define BONKO_OPTIONS_MAGIC 0xB0
#define SONG_SPEEDUP_OPTIONS_MAGIC 0x53

typedef void (*sram_write_header_func_t)(void*);
typedef void (*audio_set_output_func_t)(int8_t);
typedef void (*draw_options_func_t)(z64_ctxt_t*);

extern uint32_t CFG_DEADLY_BONKS;
uint32_t CFG_SONG_SPEEDUPS;

typedef struct {
    z64_ctxt_t state;
    char unk_00A4[0x0134];
    z64_sram_data_t* sram_buffer;
    char unk_01DC[0x1C854];
    int16_t config_mode;
    char unk_1CA32[0x007A];
    int16_t stick_x;
    int16_t stick_y;
} file_select_t;

static sram_write_header_func_t const write_sram_header =
    (sram_write_header_func_t)0x800911C0;
static audio_set_output_func_t const set_audio_output =
    (audio_set_output_func_t)0x800C7420;
static uint8_t selected_setting;
static uint8_t custom_options_alpha;

typedef char assert_bonko_option_offset[
    __builtin_offsetof(z64_sram_data_t, bonko_options) == 0x0C ? 1 : -1];
typedef char assert_song_speedup_option_offset[
    __builtin_offsetof(z64_sram_data_t, song_speedup_options) == 0x0E ? 1 : -1];
typedef char assert_sram_save_offset[
    __builtin_offsetof(z64_sram_data_t, primary_saves) == 0x20 ? 1 : -1];
typedef char assert_file_select_sram_offset[
    __builtin_offsetof(file_select_t, sram_buffer) == 0x1D8 ? 1 : -1];
typedef char assert_file_select_mode_offset[
    __builtin_offsetof(file_select_t, config_mode) == 0x1CA30 ? 1 : -1];
typedef char assert_file_select_stick_offset[
    __builtin_offsetof(file_select_t, stick_x) == 0x1CAAC ? 1 : -1];

static uint8_t* get_bonko_option(file_select_t* menu) {
    uintptr_t sram_buffer = menu == NULL ? 0 : (uintptr_t)menu->sram_buffer;
    if (menu == NULL || menu->config_mode < 0 || menu->config_mode > 40 ||
        sram_buffer < 0x80000000 || sram_buffer >= 0x80800000) {
        return NULL;
    }
    return &menu->sram_buffer->bonko_options;
}

static uint8_t* get_native_selected_setting() {
    uintptr_t overlay_base =
        (uintptr_t)z64_state_ovl_tab[FILE_SELECT_OVERLAY_INDEX].ptr;
    return (uint8_t*)(overlay_base + FILE_SELECT_SELECTED_SETTING_OFFSET);
}

void load_file_select_settings(z64_sram_data_t* sram_buffer) {
    if (sram_buffer->bonko_options_magic != BONKO_OPTIONS_MAGIC) {
        sram_buffer->bonko_options = 0;
        sram_buffer->bonko_options_magic = BONKO_OPTIONS_MAGIC;
    } else if (sram_buffer->bonko_options != 1) {
        sram_buffer->bonko_options = 0;
    }
    CFG_DEADLY_BONKS = sram_buffer->bonko_options;

    if (sram_buffer->song_speedup_options_magic != SONG_SPEEDUP_OPTIONS_MAGIC) {
        sram_buffer->song_speedup_options = 0;
        sram_buffer->song_speedup_options_magic = SONG_SPEEDUP_OPTIONS_MAGIC;
    } else if (sram_buffer->song_speedup_options != 1) {
        sram_buffer->song_speedup_options = 0;
    }
    CFG_SONG_SPEEDUPS = sram_buffer->song_speedup_options;
}

void update_file_select_options(z64_ctxt_t* menu) {
    z64_input_t* input = &menu->input[0];
    file_select_t* file_select = (file_select_t*)menu;
    uint8_t* bonko_option = get_bonko_option(file_select);
    int16_t adjusted_x = file_select->stick_x;
    int16_t adjusted_y = file_select->stick_y;

    if (bonko_option != NULL) {
        CFG_DEADLY_BONKS = *bonko_option;
        CFG_SONG_SPEEDUPS = file_select->sram_buffer->song_speedup_options;
    }

    if (input->pad_pressed.b) {
        file_select->config_mode = 39;
        file_select->sram_buffer->sound_options = z64_file.sound_setting;
        file_select->sram_buffer->z_target_options = z64_file.z_targeting;
        write_sram_header(&file_select->sram_buffer);
        set_audio_output(z64_file.sound_setting);
        return;
    }

    if (adjusted_x < -30 || adjusted_x > 30) {
        if (selected_setting == 0) {
            if (adjusted_x < -30) {
                z64_file.sound_setting =
                    z64_file.sound_setting == 0 ? 3 : z64_file.sound_setting - 1;
            } else {
                z64_file.sound_setting =
                    z64_file.sound_setting == 3 ? 0 : z64_file.sound_setting + 1;
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
        selected_setting++;
        if (selected_setting == FILE_SELECT_SETTING_COUNT) {
            selected_setting = 0;
        }
    } else if (adjusted_y > 30) {
        selected_setting =
            selected_setting == 0 ? FILE_SELECT_SETTING_COUNT - 1 : selected_setting - 1;
    } else if (input->pad_pressed.a) {
        selected_setting++;
        if (selected_setting == FILE_SELECT_SETTING_COUNT) {
            selected_setting = 0;
        }
    }

    *get_native_selected_setting() = selected_setting;
}

void draw_file_select_options(z64_ctxt_t* menu) {
    file_select_t* file_select = (file_select_t*)menu;
    uintptr_t overlay_base =
        (uintptr_t)z64_state_ovl_tab[FILE_SELECT_OVERLAY_INDEX].ptr;
    draw_options_func_t draw_native_options =
        (draw_options_func_t)(overlay_base + FILE_SELECT_DRAW_OPTIONS_OFFSET);
    uint8_t* bonko_option;
    z64_disp_buf_t* db;

    draw_native_options(menu);

    if (file_select->config_mode == FILE_SELECT_MAIN_TO_OPTIONS_MODE) {
        custom_options_alpha =
            custom_options_alpha < 0xFF - 31 ? custom_options_alpha + 31 : 0xFF;
    } else if (file_select->config_mode == FILE_SELECT_OPTIONS_MODE ||
               file_select->config_mode == FILE_SELECT_START_OPTIONS_MODE) {
        custom_options_alpha = 0xFF;
    } else if (file_select->config_mode == FILE_SELECT_OPTIONS_TO_MAIN_MODE) {
        custom_options_alpha =
            custom_options_alpha > 31 ? custom_options_alpha - 31 : 0;
    } else {
        return;
    }

    bonko_option = get_bonko_option(file_select);
    if (bonko_option == NULL) {
        return;
    }

    db = &menu->gfx->poly_opa;
    gSPDisplayList(db->p++, &setup_db);
    gDPPipeSync(db->p++);
    gDPSetCombineMode(db->p++, G_CC_MODULATEIA_PRIM, G_CC_MODULATEIA_PRIM);
    gDPSetPrimColor(db->p++, 0, 0, 0xFF, 0xFF, 0xFF, custom_options_alpha);

    if (selected_setting == FILE_SELECT_SONG_SPEEDUP_SETTING) {
        text_print(">", 160, 138);
    }
    text_print("SONG SKIPS:", 168, 138);
    text_print(file_select->sram_buffer->song_speedup_options == 0 ? "OFF" : "ON", 264, 138);
    if (selected_setting == FILE_SELECT_BONKO_SETTING) {
        text_print(">", 184, 156);
    }
    text_print("BONKO:", 192, 156);
    text_print(*bonko_option == 0 ? "OFF" : "ON", 264, 156);
    text_flush(db);

    gDPPipeSync(db->p++);
}
