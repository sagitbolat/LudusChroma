#include "SDL_sky.cpp"
#include "truetype.cpp"
static void Init(int* w, int* h) {
    *w = 1280;
    *h = 720;
}

FontData font_data = {0};

static void Awake(GameMemory* game_memory) {
    
    font_data.data = (uint8_t**)malloc(sizeof(uint8_t*)*128);

    LoadFont(&game_memory->asset_storage, "roboto.ttf", &font_data, 64);  

}
&f
static void Start(GameState* game_state, KeyboardState* keyboard_state) {
    
    
    return;
}

static void Update(GameState* game_state, KeyboardState* keyboard_state, int delta_time) {
    DisplayText("POG WOG DOG FROG HOG LOG NOG SOG BOG", &font_data, 100, 100, 1, 0);
    DisplayText("POG WOG DOG FROG HOG LOG NOG SOG BOG", &font_data, 100, 200, 1, 0);
    DisplayText("POG WOG DOG FROG HOG LOG NOG SOG BOG", &font_data, 100, 300, 1, 0);
    DisplayText("POG WOG DOG FROG HOG LOG NOG SOG BOG", &font_data, 100, 400, 1, 0);
    DisplayText("POG WOG DOG FROG HOG LOG NOG SOG BOG", &font_data, 100, 500, 1, 0);
    DisplayText("POG WOG DOG FROG HOG LOG NOG SOG BOG", &font_data, 100, 600, 1, 0);
    DisplayText("POG WOG DOG FROG HOG LOG NOG SOG BOG", &font_data, 100, 700, 1, 0);
    DisplayText("POG WOG DOG FROG HOG LOG NOG SOG BOG", &font_data, 100, 0, 1, 0);

    return;
}

static void UserFree() {

    free(font_data.data);

}
