#include "z64.h"

#include "count_tracker.h"
#include "dungeon_info.h"
#include "entrance_info.h"
#include "gfx.h"
#include "text.h"
#include "util.h"
#include "dpad.h"
#include "weather.h"
#include "world_map_info.h"
#include "song_speedup.h"
#include "mq.h"
#include "save.h"

void Gameplay_InitSkybox(z64_game_t* globalCtx, int16_t skyboxId);
void Scene_ExecuteCommands(z64_game_t* game, void* segment);

void c_init() {
    heap_init();
    gfx_init();
    text_init();
}

void before_game_state_update() {
    count_save_load_file();
    handle_dpad();
}

void after_game_state_update() {
    count_tracker_update();
    draw_dungeon_info(&(z64_ctxt.gfx->overlay));
    draw_entrance_info(&(z64_ctxt.gfx->overlay));
    draw_world_map_info(&(z64_ctxt.gfx->overlay));
}

void before_skybox_init(z64_game_t* game, int16_t skyboxId) {
    override_weather_state();
    Gameplay_InitSkybox(game, skyboxId);
}

void before_scene_commands(z64_game_t* game, void* segment) {
    apply_mq_scene_patches(game, segment);
    apply_song_speedups(game);
    Scene_ExecuteCommands(game, segment);
}

void before_room_commands(z64_game_t* game, void* segment) {
    apply_mq_room_patches(game, segment);
    Scene_ExecuteCommands(game, segment);
}
