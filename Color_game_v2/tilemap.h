#pragma once
#include "../Engine/sky_structs.h"

struct Tileset {
    Sprite atlas;
    int width_in_tiles;
    int height_in_tiles;
};
struct Tilemap {
    int* map;
    int width;
    int height;
};

// FIXME: Right now, values other than 0 and 1 are hardcoded to be walls. Change this to read collider data.
// NOTE: Returns true if colliding with wall, false if not colliding with wall.
bool TestTileCollide(Tilemap map, Vector2Int tile_pos) {
    int tile = map.map[tile_pos.y * map.width + tile_pos.x];
    if ((tile == 1 || tile == 0 || tile == 10 || tile == 11 || tile == 15 || tile == 16) || tile > 14) return false;
    else return true;
}


void DrawTile(Tileset tileset, Vector3 world_position, int atlas_x, int atlas_y) {
    float uv_width    = float(1)/float(tileset.width_in_tiles);
    float uv_height   = float(1)/float(tileset.height_in_tiles);
    float uv_x_offset = atlas_x * uv_width;
    float uv_y_offset = atlas_y * uv_height;

    ShaderSetVector(shaders, "bot_left_uv", Vector2{0.0f, 0.0f});
    ShaderSetVector(shaders, "top_right_uv", Vector2{uv_width, uv_height});
    ShaderSetVector(shaders, "uv_offset", Vector2{uv_x_offset, uv_y_offset});
    
    tile_default_transform.position = Vector3{world_position.x, world_position.y, world_position.z};
    DrawSprite(tileset.atlas, tile_default_transform, main_camera);
    
    ShaderSetVector(shaders, "bot_left_uv", Vector2{0.0f, 0.0f});
    ShaderSetVector(shaders, "top_right_uv", Vector2{1.0f, 1.0f});
    ShaderSetVector(shaders, "uv_offset", Vector2{0.0f, 0.0f});
}
void DrawTile(Tileset tileset, Vector3 world_position, uint8_t atlas_index) {
    int atlas_x = atlas_index % tileset.width_in_tiles;
    int atlas_y = atlas_index / tileset.width_in_tiles;
    DrawTile(tileset, world_position, atlas_x, atlas_y);
}