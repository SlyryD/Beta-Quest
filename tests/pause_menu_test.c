#include <assert.h>
#include <string.h>
#include <sys/mman.h>

#include "../c/pause_menu.h"

#define TEST_MEMORY_START ((void*)0x80100000)
#define TEST_MEMORY_SIZE 0x00100000

static void reset_state(void) {
    memset((void*)TEST_MEMORY_START, 0, TEST_MEMORY_SIZE);
    z64_game.pause_ctxt.state = 6;
    z64_game.pause_ctxt.screen_idx = 0;
    z64_ctxt.input[0].raw.pad.a = 1;
}

static void test_cardinal_menu_selection(void) {
    assert(pause_item_menu_active() == PAUSE_ITEM_MENU_DUNGEONS);

    z64_ctxt.input[0].raw.pad.dl = 1;
    assert(pause_item_menu_active() == PAUSE_ITEM_MENU_COUNTS);

    reset_state();
    z64_ctxt.input[0].raw.pad.dr = 1;
    assert(pause_item_menu_active() == PAUSE_ITEM_MENU_AREAS);

    reset_state();
    z64_ctxt.input[0].raw.pad.du = 1;
    assert(pause_item_menu_active() == PAUSE_ITEM_MENU_EVENTS);

    reset_state();
    z64_ctxt.input[0].raw.pad.dd = 1;
    assert(pause_item_menu_active() == PAUSE_ITEM_MENU_GEAR);
}

static void test_invalid_menu_inputs(void) {
    z64_ctxt.input[0].raw.pad.dl = 1;
    z64_ctxt.input[0].raw.pad.du = 1;
    assert(pause_item_menu_active() == PAUSE_ITEM_MENU_NONE);

    reset_state();
    z64_ctxt.input[0].raw.pad.a = 0;
    assert(pause_item_menu_active() == PAUSE_ITEM_MENU_NONE);

    reset_state();
    z64_game.pause_ctxt.changing = 1;
    assert(pause_item_menu_active() == PAUSE_ITEM_MENU_NONE);

    reset_state();
    z64_game.pause_ctxt.screen_idx = 1;
    assert(pause_item_menu_active() == PAUSE_ITEM_MENU_NONE);
}

int main(void) {
    void* memory =
        mmap(TEST_MEMORY_START, TEST_MEMORY_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED, -1, 0);
    assert(memory == TEST_MEMORY_START);

    reset_state();
    test_cardinal_menu_selection();
    reset_state();
    test_invalid_menu_inputs();
    return 0;
}
