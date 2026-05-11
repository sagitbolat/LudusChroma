#include "Engine/SDL_sky.cpp"
#include "Engine/bmp.cpp"

void Init(int* w, int* h) {
    *w = 1280;
    *h = 720;
}

TilemapData tilemap;

void Awake(GameMemory* gm) {
    LoadTilemap(&gm->asset_storage, "tilemap_square.bmp", 16, 16, &tilemap);
}

void Start(GameState* gs, KeyboardState* ks) {

}

void Update(GameState* gs, KeyboardState* ks, int dt) {
    int scale = 10;
    int tile_side_length = tilemap.tile_width;
    BlitBitmapScaled(graphics_buffer, &tilemap.tiles[0], 0, 0, scale, scale * tile_side_length, false);  
    BlitBitmapScaled(graphics_buffer, &tilemap.tiles[0], 1, 0, scale, scale * tile_side_length, false);  
    BlitBitmapScaled(graphics_buffer, &tilemap.tiles[0], 2, 0, scale, scale * tile_side_length, false);  
    BlitBitmapScaled(graphics_buffer, &tilemap.tiles[0], 3, 0, scale, scale * tile_side_length, false);  
    BlitBitmapScaled(graphics_buffer, &tilemap.tiles[1], 0, 1, scale, scale * tile_side_length, false);  
    BlitBitmapScaled(graphics_buffer, &tilemap.tiles[1], 1, 1, scale, scale * tile_side_length, false);  
    BlitBitmapScaled(graphics_buffer, &tilemap.tiles[1], 2, 1, scale, scale * tile_side_length, false);  
    BlitBitmapScaled(graphics_buffer, &tilemap.tiles[1], 3, 1, scale, scale * tile_side_length, false);  
}

void UserFree() {

}
