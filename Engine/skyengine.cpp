#pragma once
#include "skyengine.h"

// NOTE: includes imgui UI
#include "UI/sky_ui.h"
// NOTE: includes nuklear UI // TODO: This doesnt work now as a backend, so use imgui only

#include <iostream>

// NOTE: TODO: Change this to sample multiple frames for better precision.
inline double DeltaTimeToFps(double delta_time) {
    return delta_time > 0 ? (1000/delta_time) : 1000;
}

// SECTION: User Defined Functions
static void Init(int* width, int* height, float* width_in_world_space, bool* is_fullscreen, fColor* clear_color);
static void Awake(GameMemory* game_memory);
static void Start(GameState* game_state, KeyboardState* keyboard_state);
static void Update(GameState* game_state, KeyboardState* keyboard_state, double delta_time);
static void UserFree();

static void GameUpdateAndRender(Vector2Int screen_size, GameMemory* memory, KeyboardState* keyboard_state, double delta_time) {

    GameState* game_state = (GameState*)memory->permanent_storage.memory;

    if (!memory->is_initialized) {
        memory->is_initialized = true;
    }
    
    static bool first = true;

    UI_FrameStart();
    
    
    if (first) {
        Start(game_state, keyboard_state);
        first = false;
    }
    else {
        Update(game_state, keyboard_state, delta_time);
    }
    
#ifdef FPS_SHOW
    // FIXME: Since moving window code to user space, this doesnt work, since it needs an active window to work. Find a fix?
    double fps = DeltaTimeToFps(delta_time);
    char fps_str[64];
    sprintf(fps_str, "FPS: %f", fps);
    DrawSimpleText(fps_str, {1.0f, 0.0f}, UI_Alignment::TOP_RIGHT);
#endif
    
    UI_FrameRender(); 
    //std::cout << fps << std::endl;
}
