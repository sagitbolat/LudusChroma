#define INCLUDE_IMGUI
#define DEBUG_UI
#define NO_SPLASH_SCREEN
#include "../Engine/SDL_sky.cpp"
#include "../Engine/skymath.h"

#include "../Engine/data_structs/dynamic_array.h"
DEFINE_LIST(int);

#define MAX_NUM_PLAYERS 256
#define MAX_ENTITIES 256 

#include <chrono>
#include <thread>


// SECTION: Initialization of stuff...
void Init(int *w, int *h, float *w_in_world_space, bool *fullscreen, fColor *clear_color)
{
    *w = 1280;
    *h = 720;
    *w_in_world_space = 14.0f;
    *fullscreen = false;
    //*clear_color = {43.0f/255, 43.0f/255, 39.0f/255, 1.0f};
    *clear_color = {0.0f, 0.0f, 0.0f, 1.0f};
}

void PrintList(int_List* list) {
    printf("List (len = %zu, cap = %zu): [", list->length, list->capacity);
    for (index_t i = 0; i < list->length; i++) {
        printf("%d", list->data[i]);
        if (i + 1 < list->length) printf(", ");
    }
    printf("]\n");
}

void Awake(GameMemory *gm){

}

void Start(GameState *gs, KeyboardState *ks) {
}

void Update(GameState *gs, KeyboardState *ks, double dt) {

}

void UserFree() {
}