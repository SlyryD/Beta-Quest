#include "gfx.h"
#include "util.h"
#include "dpad.h"
#include "z64.h"

void c_init() {
    heap_init();
    gfx_init();
}

void before_game_state_update() {
    handle_dpad();
}

void after_game_state_update() {

}

void before_skybox_init(z64_game_t* game, int16_t skyboxId) {

}

void after_scene_init() {

}