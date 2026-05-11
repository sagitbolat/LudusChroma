#include "SDL_sky.cpp"
#include "bmp.cpp"

static void Init(int* w, int* h) {
    *w = 1920;
    *h = 1080;
}

static void Awake(GameMemory* m) {

}

static void Start(GameState* gs, KeyboardState* ks) {

}

static void Update(GameState* gs, KeyboardState* ks, int dt) {
    DrawLine(graphics_buffer, 255, 255, 255, 0, 0, 1920, 1080);
}

static void UserFree() {

}
