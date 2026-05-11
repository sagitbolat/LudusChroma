#define INCLUDE_IMGUI
#define DEBUG_MODE
#define NO_SPLASH_SCREEN
#include "../Engine/SDL_sky.cpp"
#include "../Engine/skymath.h"
#include "../Engine/scene.h"

#define MAX_ENTITIES 256 
#define MAX_NUM_PLAYERS 24
#include "tilemap.h"
#include "entity.h"
#include "level_loader.h"


#ifdef PROFILING
#include <chrono>
#endif


// SECTION: Initialization of stuff...
void Init(int *w, int *h, float *w_in_world_space, bool *fullscreen, fColor *clear_color)
{
    *w = 1280;
    *h = 720;
    *w_in_world_space = 14.0f;
    *fullscreen = false;
    //*clear_color = {0.8f/2, 0.83f/2, 1.0f/2, 1.0f};
    *clear_color = {43.0f/255.0f, 43.0f/255.0f, 39.0f/255.0f, 1.0f};
}

Tileset tileset = {0}; // The tile asset data
Tilemap tilemap = {0}; // The tile state
Tileset sprite_atlas = {0}; // The asset data for individual sprites
