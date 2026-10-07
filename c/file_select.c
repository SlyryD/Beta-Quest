#include "file_select.h"

#include "gfx.h"
#include "text.h"

#define FILE_SELECT_OVERLAY_INDEX 5
#define FILE_SELECT_DRAW_OPTIONS_OFFSET 0x7080
#define FILE_SELECT_SELECTED_SETTING_OFFSET 0xFF30
#define FILE_SELECT_BONKO_SETTING 2
#define FILE_SELECT_SETTING_COUNT 3
#define BONKO_OPTIONS_MAGIC 0xB0

typedef void (*sram_write_header_func_t)(void*);
typedef void (*audio_set_output_func_t)(int8_t);
typedef void (*draw_options_func_t)(z64_ctxt_t*);

extern uint32_t CFG_DEADLY_BONKS;

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

typedef char assert_bonko_option_offset[
    __builtin_offsetof(z64_sram_data_t, bonko_options) == 0x0C ? 1 : -1];
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

void load_bonko_setting(z64_sram_data_t* sram_buffer) {
    if (sram_buffer->bonko_options_magic != BONKO_OPTIONS_MAGIC) {
        sram_buffer->bonko_options = 0;
        sram_buffer->bonko_options_magic = BONKO_OPTIONS_MAGIC;
    } else if (sram_buffer->bonko_options != 1) {
        sram_buffer->bonko_options = 0;
    }
    CFG_DEADLY_BONKS = sram_buffer->bonko_options;
}

void update_file_select_options(z64_ctxt_t* menu) {
    z64_input_t* input = &menu->input[0];
    file_select_t* file_select = (file_select_t*)menu;
    uint8_t* bonko_option = get_bonko_option(file_select);
    int16_t adjusted_x = file_select->stick_x;
    int16_t adjusted_y = file_select->stick_y;

    if (bonko_option != NULL) {
        CFG_DEADLY_BONKS = *bonko_option;
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
        } else if (bonko_option != NULL) {
            *bonko_option ^= 1;
            CFG_DEADLY_BONKS = *bonko_option;
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

    *get_native_selected_setting() =
        selected_setting == FILE_SELECT_BONKO_SETTING ? 1 : selected_setting;
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

    bonko_option = get_bonko_option(file_select);
    if (bonko_option == NULL) {
        return;
    }

    db = &menu->gfx->poly_opa;
    gSPDisplayList(db->p++, &setup_db);
    gDPPipeSync(db->p++);
    gDPSetCombineMode(db->p++, G_CC_MODULATEIA_PRIM, G_CC_MODULATEIA_PRIM);
    gDPSetPrimColor(db->p++, 0, 0, 0xFF, 0xFF, 0xFF, 0xFF);

    if (selected_setting == FILE_SELECT_BONKO_SETTING) {
        text_print(">", 184, 156);
    }
    text_print("BONKO:", 192, 156);
    text_print(*bonko_option == 0 ? "OFF" : "ON", 264, 156);
    text_flush(db);

    gDPPipeSync(db->p++);
}
