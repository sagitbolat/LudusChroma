#include "Engine/SDL_sky.cpp"
#include "Engine/bmp.cpp"
#include "Engine/tilemaps.cpp"
#include "Engine/pathfinding.cpp"
#include "Engine/button.cpp"
#include "Engine/array_utils.cpp"


// SECTION: main
void Init(int* w, int* h) {
    *w = 1296;
    *h = 720;
}

TilemapData grass_tilemap;
LevelTiledata level1_data; 
RectButton* tilemap_buttons;

void Awake(GameMemory* gm) {
    LoadTilemap(&gm->asset_storage, "tilemap_square.bmp", 16, 16, &grass_tilemap);
}


void Start(GameState* gs, KeyboardState* ks) {
    level1_data = LoadLevel("level1.bin");
    

    // NOTE: The button components of the tilemap
    tilemap_buttons = (RectButton*)malloc(sizeof(RectButton) * level1_data.height * level1_data.width);
    int num_tiles = level1_data.height * level1_data.width;
    // TODO: set the correct positions for the button locations
}

int total_time = 0;

void Update(GameState* gs, KeyboardState* ks, int dt) {
    total_time += dt;
    int scale = 3;
    
    for (int h = 0; h < level1_data.height; ++h) {
        for (int w = 0; w < level1_data.width; ++w) {
            BlitBitmapScaled(graphics_buffer, &grass_tilemap.tiles[level1_data.tile_data[h*level1_data.width+w]], w, h, scale, scale * 16, false); 
        }
    }

    // SECTION: Do button cheking
    MousePosition pos = GetMousePosition();
    
    // NOTE: loop over buttons and check if mouse pos matches the button
    int num_tiles = level1_data.height * level1_data.width;
    for (int i = 0; i < num_tiles; ++i) {
        
    }
}

void UserFree() {
        
}
