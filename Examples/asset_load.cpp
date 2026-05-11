#include "SDL_sky.cpp"
#include "bmp.cpp"

static void Init(int* w, int *h) {
    *w = 1920;
    *h = 1080;
    return;
}

ImageData image_data = {0};
static void Awake(GameMemory* game_memory) {
    
    size_t image_size = LoadBitmap(&game_memory->asset_storage, "background.bmp", &image_data);
    
    printf("Image size: %ld%c", image_size, '\n');
    
    return;
}


static void Start(GameState* game_state, KeyboardState* keyboard_state) {
    return;
}

static void Update(GameState* game_state, KeyboardState* keyboard_state, int delta_time) {
    BlitBitmap(graphics_buffer, &image_data, 0, 0, 1);
    return;
}

static void UserFree() {
    return;
}
