#pragma once
#include "sky_structs.h"

// SpriteSheet describes the asset: texture + grid layout.
// frame_count <= cols * rows to allow partially-filled grids.
struct SpriteSheet {
    Sprite sprite;
    int    cols;
    int    rows;
    int    frame_count;
};

// SpriteAnim is per-instance playback state.
// Point sheet at a SpriteSheet, set fps and looping, then call
// SpriteAnimUpdate each frame and DrawSpriteAnim to render.
struct SpriteAnim {
    SpriteSheet* sheet;
    float        fps;
    float        timer;    // ms accumulator
    int          frame;    // current frame index (0-based)
    bool         looping;
};

inline SpriteSheet MakeSpriteSheet(Sprite sprite, int cols, int rows, int frame_count) {
    SpriteSheet s;
    s.sprite      = sprite;
    s.cols        = cols;
    s.rows        = rows;
    s.frame_count = frame_count;
    return s;
}

// Advance the animation by dt_ms milliseconds.
inline void SpriteAnimUpdate(SpriteAnim* anim, float dt_ms) {
    if (anim->fps <= 0.f) return;
    float frame_dur = 1000.f / anim->fps;
    anim->timer += dt_ms;
    while (anim->timer >= frame_dur) {
        anim->timer -= frame_dur;
        anim->frame++;
        if (anim->frame >= anim->sheet->frame_count)
            anim->frame = anim->looping ? 0 : anim->sheet->frame_count - 1;
    }
}

// Draw an arbitrary frame directly from a SpriteSheet (no SpriteAnim needed).
inline void DrawSpriteSheetFrame(SpriteSheet* sheet, int frame, Transform t, Camera cam, GL_ID* shaders) {
    float uv_w = 1.f / (float)sheet->cols;
    float uv_h = 1.f / (float)sheet->rows;
    int col = frame % sheet->cols;
    int row = frame / sheet->cols;
    ShaderSetVector(shaders, "bot_left_uv",  Vector2{ 0.f, 0.f });
    ShaderSetVector(shaders, "top_right_uv", Vector2{ uv_w, uv_h });
    ShaderSetVector(shaders, "uv_offset",    Vector2{ col * uv_w, row * uv_h });
    DrawSprite(sheet->sprite, t, cam);
    ShaderSetVector(shaders, "bot_left_uv",  Vector2{ 0.f, 0.f });
    ShaderSetVector(shaders, "top_right_uv", Vector2{ 1.f, 1.f });
    ShaderSetVector(shaders, "uv_offset",    Vector2{ 0.f, 0.f });
}

// Draw the current frame of a SpriteAnim.
inline void DrawSpriteAnim(SpriteAnim* anim, Transform t, Camera cam, GL_ID* shaders) {
    DrawSpriteSheetFrame(anim->sheet, anim->frame, t, cam, shaders);
}
