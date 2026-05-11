#pragma once
// Rendering layer for ECS entities. Depends on entity.h and the engine's sprite/shader API.
// game.cpp sets rt->transform.position.z before calling EntityRender to control draw order.
#include "entity.h"

// Sprite index constants — must match the sprite array loaded in game.cpp
#define SPR_PLAYER_NEUTRAL  0
#define SPR_PLAYER_UP       1
#define SPR_PLAYER_DOWN     2
#define SPR_PLAYER_LEFT     3
#define SPR_PLAYER_RIGHT    4
#define SPR_PUSH_BLOCK      5
#define SPR_STATIC_BLOCK    6
#define SPR_EMITTER         7
#define SPR_EMITTER_NOZZLE  8
#define SPR_RECEIVER        10
#define SPR_RECEIVER_SIGNAL 11
#define SPR_RECEIVER_COLOR  12
#define SPR_DOOR_OPEN_V     13
#define SPR_DOOR_CLOSED_V   14
#define SPR_DOOR_OPEN_H     15
#define SPR_DOOR_CLOSED_H   16
#define SPR_ENDGOAL         17
#define SPR_BUTTON_UP       18
#define SPR_BUTTON_DOWN     19
#define SPR_TELEPORTER      20
#define SPR_COLOR_CHANGER   21
#define SPR_CC_FRAME        22
#define SPR_CC_OVERLAY      23
#define SPR_COLOR_PUDDLE    24
#define SPR_SUIT_NEUTRAL    25
#define SPR_SUIT_UP         26
#define SPR_SUIT_DOWN       27
#define SPR_SUIT_LEFT       28
#define SPR_SUIT_RIGHT      29

// ============================================================
// Internal helpers — UV clipping for split-sprite rendering
// ============================================================

static inline void UVReset(GL_ID* sh) {
    ShaderSetVector(sh, "bot_left_uv",  Vector2{ 0.f, 0.f });
    ShaderSetVector(sh, "top_right_uv", Vector2{ 1.f, 1.f });
    ShaderSetVector(sh, "uv_offset",    Vector2{ 0.f, 0.f });
}

// Clips to upper half of sprite sheet row and shifts position up by 0.5 for top-half rendering
static inline void UVTopHalf(GL_ID* sh) {
    ShaderSetVector(sh, "bot_left_uv",  Vector2{ 0.f, 0.f   });
    ShaderSetVector(sh, "top_right_uv", Vector2{ 1.f, 0.5f  });
    ShaderSetVector(sh, "uv_offset",    Vector2{ 0.f, 0.5f  });
}

// Clips to lower half (no offset shift)
static inline void UVBottomHalf(GL_ID* sh) {
    ShaderSetVector(sh, "bot_left_uv",  Vector2{ 0.f, 0.f  });
    ShaderSetVector(sh, "top_right_uv", Vector2{ 1.f, 0.5f });
    ShaderSetVector(sh, "uv_offset",    Vector2{ 0.f, 0.f  });
}

static inline void ColorMul(GL_ID* sh, Color c, bool active) {
    if (active) ShaderSetVector(sh, "i_color_multiplier", Vec4(c));
}
static inline void ColorMulReset(GL_ID* sh, bool active) {
    if (active) ShaderSetVector(sh, "i_color_multiplier", Vector4{ 1.f, 1.f, 1.f, 1.f });
}


// ============================================================
// EmissionRender — call per-tile after entity renders for that y row
// Clears the emission tile after rendering (frame-transient)
// ============================================================

void EmissionRender(int x, int y, EmissionMap& emission_map, Sprite emission_sprite,
                    GL_ID* shaders, bool level_transitioning = false) {
    EmissionTile tile = emission_map.GetEmissionTile(x, y);
    if (!tile.active) return;

    float uv_x = 0.f;
    float sx    = 1.f;
    float sy    = 6.f / 16.f;

    if (tile.orientation == EmissionTile::VERTICAL) {
        uv_x = 1.f / 5.f;
        sx   = sy;
        sy   = 1.f;
    } else if (tile.orientation == EmissionTile::CROSSED) {
        uv_x = 2.f / 5.f;
        sx   = 6.f / 16.f;
    }

    ShaderSetVector(shaders, "bot_left_uv",  Vector2{ 0.f,       0.f });
    ShaderSetVector(shaders, "top_right_uv", Vector2{ 1.f / 5.f, 1.f });
    ShaderSetVector(shaders, "uv_offset",    Vector2{ uv_x,      0.f });

    Transform t{};
    t.position = Vector3{ float(x), float(y) + 0.5f, 0.5f - float(2 * y) };
    t.rotation = { 0.f, 0.f, 0.f };
    t.scale    = { sx, sy, 1.f };

    ColorMul(shaders, emission_map.GetEmissionTile(x, y).color, !level_transitioning);
    DrawSprite(emission_sprite, t, main_camera);

    if (tile.orientation == EmissionTile::CROSSED) {
        // Horizontal bar on top
        t.scale.y      = 6.f / 16.f;
        t.scale.x      = 1.f;
        t.position.z  += 0.1f;
        ShaderSetVector(shaders, "uv_offset", Vector2{ 3.f / 5.f, 0.f });
        ColorMul(shaders, emission_map.GetEmissionTile(x, y).horizontal_color, !level_transitioning);
        DrawSprite(emission_sprite, t, main_camera);

        // Vertical bar on top of that
        t.scale.y      = 1.f;
        t.scale.x      = 6.f / 16.f;
        t.position.z  += 0.1f;
        ShaderSetVector(shaders, "uv_offset", Vector2{ 4.f / 5.f, 0.f });
        ColorMul(shaders, emission_map.GetEmissionTile(x, y).vertical_color, !level_transitioning);
        DrawSprite(emission_sprite, t, main_camera);
    }

    UVReset(shaders);
    ColorMulReset(shaders, !level_transitioning);

    emission_map.SetEmissionTile(x, y, { false, EmissionTile::HORIZONTAL,
                                          { 0,0,0,0 }, { 0,0,0,0 }, { 0,0,0,0 } });
}


// ============================================================
// EntityRender — renders one entity by querying its components.
// Determine type by component presence (same priority as v1).
// rendering_top=true: render upper half of sprite at y+0.5 (standard).
// rendering_top=false: render lower half at y-0.5 (second-pass, optional).
// ============================================================

void EntityRender(int entity_id, ComponentArrays* ca, GL_ID* shaders, const Sprite* sprites,
                  bool rendering_top = true, bool level_transitioning = false) {
    RenderTransform* rt = ca->render_transform_arr.Get(entity_id);
    if (!rt) return;

    // Bump z for moving entities crossing y rows (prevents clipping behind lower-y entities)
    GridMover*    gm = ca->grid_mover_arr.Get(entity_id);
    GridPosition* gp = ca->grid_position_arr.Get(entity_id);
    bool z_bumped = false;
    if (gm && gm->moving && gp && gp->prev_position.y != gp->position.y) {
        rt->transform.position.z += 1.3f;
        z_bumped = true;
    }

    // ---- Player ----
    GridPlayerControlled* player = ca->grid_player_controlled_arr.Get(entity_id);
    if (player) {
        int body_spr = SPR_PLAYER_NEUTRAL;
        int suit_spr = SPR_SUIT_NEUTRAL;
        switch (player->orientation) {
            case Direction::Up:    body_spr = SPR_PLAYER_UP;    suit_spr = SPR_SUIT_UP;    break;
            case Direction::Down:  body_spr = SPR_PLAYER_DOWN;  suit_spr = SPR_SUIT_DOWN;  break;
            case Direction::Left:  body_spr = SPR_PLAYER_LEFT;  suit_spr = SPR_SUIT_LEFT;  break;
            case Direction::Right: body_spr = SPR_PLAYER_RIGHT; suit_spr = SPR_SUIT_RIGHT; break;
            default: break;
        }
        Transform t{};
        CopyTransform(&t, rt->transform);
        t.position.y += rendering_top ? 0.5f : -0.5f;
        if (rendering_top) UVTopHalf(shaders); else UVBottomHalf(shaders);
        DrawSprite(sprites[body_spr], t, main_camera);
        t.position.z += 0.05f;
        ColorMul(shaders, player->color, !level_transitioning);
        DrawSprite(sprites[suit_spr], t, main_camera);
        ColorMulReset(shaders, !level_transitioning);
        UVReset(shaders);
        goto done;
    }

    // ---- Door ----
    {
        Door* door = ca->door_arr.Get(entity_id);
        if (door) {
            Transform t = rt->transform;
            if (door->is_open) {
                int spr = door->open_by_default ? SPR_DOOR_OPEN_H : SPR_DOOR_OPEN_V;
                DrawSprite(sprites[spr], t, main_camera);
            } else {
                int spr = door->open_by_default ? SPR_DOOR_CLOSED_H : SPR_DOOR_CLOSED_V;
                t.position.y += rendering_top ? 0.5f : -0.5f;
                if (rendering_top) UVTopHalf(shaders); else UVBottomHalf(shaders);
                DrawSprite(sprites[spr], t, main_camera);
                UVReset(shaders);
            }
            goto done;
        }
    }

    // ---- Button ----
    {
        Button* btn = ca->button_arr.Get(entity_id);
        if (btn) {
            DrawSprite(sprites[btn->is_pressed ? SPR_BUTTON_DOWN : SPR_BUTTON_UP], rt->transform, main_camera);
            goto done;
        }
    }

    // ---- Emitter ----
    {
        LaserEmitter* le = ca->laser_emitter_arr.Get(entity_id);
        if (le) {
            Transform t{};
            CopyTransform(&t, rt->transform);
            t.position.y += rendering_top ? 0.5f : -0.5f;
            if (rendering_top) UVTopHalf(shaders); else UVBottomHalf(shaders);
            DrawSprite(sprites[SPR_EMITTER], t, main_camera);
            UVReset(shaders);
            t.position.z += 0.1f;
            ColorMul(shaders, le->color, !level_transitioning);
            if (rendering_top) UVTopHalf(shaders); else UVBottomHalf(shaders);
            DrawSprite(sprites[SPR_EMITTER_NOZZLE], t, main_camera);
            ColorMulReset(shaders, !level_transitioning);
            UVReset(shaders);
            goto done;
        }
    }

    // ---- Receiver ----
    {
        LaserReceiver* lr = ca->laser_receiver_arr.Get(entity_id);
        if (lr) {
            Transform t{};
            CopyTransform(&t, rt->transform);
            t.position.y += rendering_top ? 0.5f : -0.5f;
            if (rendering_top) UVTopHalf(shaders); else UVBottomHalf(shaders);
            DrawSprite(sprites[SPR_RECEIVER], t, main_camera);
            UVReset(shaders);

            // Accepted color indicator
            t.position.z += 0.1f;
            ColorMul(shaders, lr->accepted_color, !level_transitioning);
            if (rendering_top) UVTopHalf(shaders); else UVBottomHalf(shaders);
            DrawSprite(sprites[SPR_RECEIVER_COLOR], t, main_camera);
            ColorMulReset(shaders, !level_transitioning);
            UVReset(shaders);

            // Incoming signal overlay (only when signal is present)
            if (lr->received) {
                t.position.z += 0.1f;
                ColorMul(shaders, lr->incoming_color, !level_transitioning);
                if (rendering_top) UVTopHalf(shaders); else UVBottomHalf(shaders);
                DrawSprite(sprites[SPR_RECEIVER_SIGNAL], t, main_camera);
                ColorMulReset(shaders, !level_transitioning);
                UVReset(shaders);
            }
            goto done;
        }
    }

    // ---- Teleporter ----
    {
        Teleporter* tp = ca->teleporter_arr.Get(entity_id);
        if (tp) {
            ColorMul(shaders, tp->color, !level_transitioning);
            DrawSprite(sprites[SPR_TELEPORTER], rt->transform, main_camera);
            ColorMulReset(shaders, !level_transitioning);
            goto done;
        }
    }

    // ---- ColorChanger ----
    {
        ColorChanger* cc = ca->color_changer_arr.Get(entity_id);
        if (cc) {
            Transform t{};
            CopyTransform(&t, rt->transform);
            t.position.y += rendering_top ? 0.5f : -0.5f;

            ColorMul(shaders, cc->main_color, !level_transitioning);
            if (rendering_top) UVTopHalf(shaders); else UVBottomHalf(shaders);
            DrawSprite(sprites[SPR_COLOR_CHANGER], t, main_camera);
            ColorMulReset(shaders, !level_transitioning);
            UVReset(shaders);

            t.position.z += 0.1f;
            if (rendering_top) UVTopHalf(shaders); else UVBottomHalf(shaders);
            DrawSprite(sprites[SPR_CC_FRAME], t, main_camera);
            UVReset(shaders);

            // Laser visualization (only on top pass; uses frame-transient input colors)
            if (rendering_top) {
                ShaderSetVector(shaders, "bot_left_uv",  Vector2{ 0.f,       0.f });
                ShaderSetVector(shaders, "top_right_uv", Vector2{ 1.f / 5.f, 1.f });
                t.scale.y      = 2.f;
                t.position.y  -= 0.5f;
                t.position.z  += 0.1f;

                ShaderSetVector(shaders, "uv_offset", Vector2{ 0.f, 0.f });
                ColorMul(shaders, cc->horizontal_input_color, !level_transitioning);
                DrawSprite(sprites[SPR_CC_OVERLAY], t, main_camera);

                ShaderSetVector(shaders, "uv_offset", Vector2{ 1.f / 5.f, 0.f });
                t.position.z += 0.1f;
                ColorMul(shaders, cc->vertical_input_color, !level_transitioning);
                DrawSprite(sprites[SPR_CC_OVERLAY], t, main_camera);

                // Combined output beam
                float combined_offset = 2.f / 5.f;
                if (cc->vertical_input_color.a == 0)   combined_offset = 3.f / 5.f;
                else if (cc->horizontal_input_color.a == 0) combined_offset = 4.f / 5.f;
                ShaderSetVector(shaders, "uv_offset", Vector2{ combined_offset, 0.f });
                t.position.z += 0.1f;
                ColorMul(shaders, AddColor(cc->horizontal_input_color, cc->vertical_input_color), !level_transitioning);
                DrawSprite(sprites[SPR_CC_OVERLAY], t, main_camera);

                UVReset(shaders);
                ColorMulReset(shaders, !level_transitioning);
            }
            goto done;
        }
    }

    // ---- Endgoal ----
    if (ca->endgoal_arr.Get(entity_id)) {
        DrawSprite(sprites[SPR_ENDGOAL], rt->transform, main_camera);
        goto done;
    }

    // ---- Push block (has GridMover, no special component above) ----
    if (gm) {
        Transform t{};
        CopyTransform(&t, rt->transform);
        t.position.y += rendering_top ? 0.5f : -0.5f;
        if (rendering_top) UVTopHalf(shaders); else UVBottomHalf(shaders);
        DrawSprite(sprites[SPR_PUSH_BLOCK], t, main_camera);
        UVReset(shaders);
        goto done;
    }

    // ---- Static block ----
    {
        Transform t{};
        CopyTransform(&t, rt->transform);
        t.position.y += rendering_top ? 0.5f : -0.5f;
        if (rendering_top) UVTopHalf(shaders); else UVBottomHalf(shaders);
        DrawSprite(sprites[SPR_STATIC_BLOCK], t, main_camera);
        UVReset(shaders);
    }

done:
    if (z_bumped) rt->transform.position.z -= 1.3f;
}
