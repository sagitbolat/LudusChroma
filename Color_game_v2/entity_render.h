#pragma once
// Rendering layer for ECS entities. Depends on entity.h and the engine's sprite/shader API.
// game.cpp sets rt->transform.position.z before calling EntityRender to control draw order.
#include "entity.h"
#include "game_color_switcher.h"

extern Sprite color_changer_additive_sprite;
extern Sprite color_changer_subtractive_sprite;
extern Sprite letter_sprite;
extern float  global_anim_timer;

// Sprite index constants — must match the sprite array loaded in game.cpp
#define SPR_PLAYER_NEUTRAL   0
#define SPR_PLAYER_UP        1
#define SPR_PLAYER_DOWN      2
#define SPR_PLAYER_LEFT      3
#define SPR_PLAYER_RIGHT     4
#define SPR_PUSH_BLOCK       5
#define SPR_STATIC_BLOCK     6
#define SPR_EMITTER          7
#define SPR_EMITTER_NOZZLE   8
#define SPR_RECEIVER         10
#define SPR_RECEIVER_SIGNAL  11
#define SPR_RECEIVER_COLOR   12
#define SPR_DOOR_OPEN_V      13
#define SPR_DOOR_CLOSED_V    14
#define SPR_DOOR_OPEN_H      15
#define SPR_DOOR_CLOSED_H    16
#define SPR_ENDGOAL          17
#define SPR_BUTTON_UP        18
#define SPR_BUTTON_DOWN      19
#define SPR_TELEPORTER       20
#define SPR_COLOR_CHANGER    21
#define SPR_CC_FRAME         22
#define SPR_CC_OVERLAY       23
#define SPR_COLOR_PUDDLE     24
#define SPR_SUIT_NEUTRAL     25
#define SPR_SUIT_UP          26
#define SPR_SUIT_DOWN        27
#define SPR_SUIT_LEFT        28
#define SPR_SUIT_RIGHT       29
#define SPR_PUSH_BLOCK_HORIZ 30
#define SPR_PUSH_BLOCK_VERT  31

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

static inline void UVTopHalfAnim(GL_ID* sh, int frame, int num_frames) {
    float uv_w = 1.f / (float)num_frames;
    int   col  = frame % num_frames;
    ShaderSetVector(shaders, "bot_left_uv",  Vector2{ 0.f,        0.f });
    ShaderSetVector(shaders, "top_right_uv", Vector2{ uv_w,       0.5f });
    ShaderSetVector(shaders, "uv_offset",    Vector2{ col * uv_w, 0.5f });
}

static inline void UVBottomHalfAnim(GL_ID* sh, int frame, int num_frames) {
    float uv_w = 1.f / (float)num_frames;
    int   col  = frame % num_frames;
    ShaderSetVector(shaders, "bot_left_uv",  Vector2{ 0.f,        0.f });
    ShaderSetVector(shaders, "top_right_uv", Vector2{ uv_w,       0.5f });
    ShaderSetVector(shaders, "uv_offset",    Vector2{ col * uv_w, 0.f });

}

static inline void UVAnim(GL_ID* sh, int frame, int num_frames) {
    float uv_w = 1.f / (float)num_frames;
    int   col  = frame % num_frames;
    ShaderSetVector(shaders, "bot_left_uv",  Vector2{ 0.f,        0.f });
    ShaderSetVector(shaders, "top_right_uv", Vector2{ uv_w,       1.f });
    ShaderSetVector(shaders, "uv_offset",    Vector2{ col * uv_w, 0.f });

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
// Portal clip helpers — applied per draw call during teleport animations.
// axis=0: clip along X (horizontal movement). axis=1: clip along Y (vertical movement).
// vis_min/vis_max: visible fraction of the full sprite in [0,1] sprite space.
// For Y-axis clips, each half-sprite draw covers either [0,0.5] or [0.5,1] of the sprite,
// so the clip range is remapped into that half's quad UV [0,1] space.
// ============================================================

static void SetPortalClipForHalf(GL_ID* sh, int axis, float vis_min, float vis_max, bool rendering_top) {
    ShaderSetFloat(sh, "portal_clip_enabled", 1.f);
    ShaderSetFloat(sh, "portal_clip_axis",    (float)axis);
    if (axis == 0) {
        ShaderSetFloat(sh, "portal_clip_uv_min", vis_min);
        ShaderSetFloat(sh, "portal_clip_uv_max", vis_max);
    } else {
        float half_lo = rendering_top ? 0.5f : 0.f;
        float half_hi = rendering_top ? 1.f  : 0.5f;
        float lo = vis_min > half_lo ? vis_min : half_lo;
        float hi = vis_max < half_hi ? vis_max : half_hi;
        if (lo >= hi) {
            ShaderSetFloat(sh, "portal_clip_uv_min",  2.f);   // impossible → discard all
            ShaderSetFloat(sh, "portal_clip_uv_max", -1.f);
        } else {
            ShaderSetFloat(sh, "portal_clip_uv_min", (lo - half_lo) * 2.f);
            ShaderSetFloat(sh, "portal_clip_uv_max", (hi - half_lo) * 2.f);
        }
    }
}

static void ResetPortalClip(GL_ID* sh) {
    ShaderSetFloat(sh, "portal_clip_enabled", 0.f);
}


// ============================================================
// EntityRender — renders one entity by querying its components.
// Determine type by component presence (same priority as v1).
// rendering_top=true: render upper half of sprite at y+0.5 (standard).
// rendering_top=false: render lower half at y-0.5 (second-pass, optional).
// ============================================================

void EntityRender(int entity_id, ComponentArrays* ca, GL_ID* shaders, const Sprite* sprites,
                  bool rendering_top = true, bool level_transitioning = false, bool force_render = false,
                  bool exit_render = false) {

    // If Entity is hidden due to the clear color mechanic, skip rendering it.
    if (!force_render && isHidden(entity_id, ca)) return;

    RenderTransform* rt = ca->render_transform_arr.Get(entity_id);
    if (!rt) return;

    GridMover*    gm = ca->grid_mover_arr.Get(entity_id);
    GridPosition* gp = ca->grid_position_arr.Get(entity_id);

    bool do_portal = (gm && gm->teleporting);

    if (do_portal && !exit_render && gm->teleport_direction.y != 0) {
        float base_z = (gp && gp->layer == GridLayer::EntityLayer) ? 1.f : 0.f;
        rt->transform.position.z = base_z - 2.f * (float)gm->teleport_entry.y;
    }

    // Portal clip: set directional mask before any draw calls for this entity
    if (do_portal) {
        int   axis    = (gm->teleport_direction.x != 0) ? 0 : 1;
        bool  pos_dir = (gm->teleport_direction.x > 0 || gm->teleport_direction.y > 0);
        float t  = gm->teleport_t;
        float ec = t;        // entry clip: 0→1 over full animation
        float xc = 1.f - t; // exit  clip: 1→0 over full animation

        float vis_min, vis_max;
        if (axis == 0) {
            // X-axis: simple UV-based clip
            if (!exit_render) {
                vis_min = pos_dir ? 0.f        : ec;
                vis_max = pos_dir ? (1.f - ec) : 1.f;
            } else {
                vis_min = pos_dir ? xc  : 0.f;
                vis_max = pos_dir ? 1.f : (1.f - xc);
            }
        } else {
            // Sprite is 2 units tall: top 1 unit = character, next 0.5 = sidewall, bottom 0.5 = empty padding.
            // Empty padding occupies v=[0, 0.25], so the clip range that covers real content is [0.25, 1.0].
            float clip_v = pos_dir ? (1.0f - t * 0.5f) : (0.5f + t * 0.5f);
            if (!exit_render) {
                vis_min = pos_dir ? 0.f    : clip_v;
                vis_max = pos_dir ? clip_v : 1.f;
            } else {
                vis_min = pos_dir ? clip_v : 0.f;
                vis_max = pos_dir ? 1.f    : clip_v;
            }
        }
        if (vis_max <= vis_min) vis_max = -1.f;
        SetPortalClipForHalf(shaders, axis, vis_min, vis_max, rendering_top);
    }

    // Bump z for moving entities crossing y rows (prevents clipping behind lower-y entities)
    bool z_bumped = false;
    if (!exit_render && gm && gm->moving && gp && gp->prev_position.y != gp->position.y) {
        rt->transform.position.z += 1.3f;
        z_bumped = true;
    }

    // Shake offset for merge-conflict visual feedback (entry render only)
    float shake_x = 0.f;
    if (!exit_render) {
        for (int s = 0; s < MAX_SHAKE_ENTRIES; ++s) {
            if (shake_entries[s].entity_id == entity_id && shake_entries[s].timer > 0.f) {
                float t = shake_entries[s].timer;
                shake_x = sinf(t * 0.05f) * 0.12f * (t / SHAKE_DURATION);
                break;
            }
        }
    }
    if (shake_x != 0.f) rt->transform.position.x += shake_x;

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
        ColorMul(shaders, player->color, !level_transitioning);
        DrawSprite(sprites[suit_spr], t, main_camera);
        ColorMulReset(shaders, !level_transitioning);
        UVReset(shaders);
        if (rendering_top) {
            t.position.z += 0.05f;
            switch (player->upwards_direction) {
                case Direction::Up:    t.rotation.z =   0.f; break;
                case Direction::Right: t.rotation.z =  270.f; break;
                case Direction::Down:  t.rotation.z = 180.f; break;
                case Direction::Left:  t.rotation.z = 90.f; break;
                default:               t.rotation.z =   0.f; break;
            }
            DrawSprite(sprites[body_spr], t, main_camera);
        }
        goto done;
    }

    // ---- Door ----
    {
        Door* door = ca->door_arr.Get(entity_id);
        if (door) {
            Transform t = rt->transform;
            bool is_horiz = door->open_by_default;

            auto DrawAnimFrame = [&](SpriteSheet& sheet, float timer, float duration) {
                int n    = sheet.frame_count > 0 ? sheet.frame_count : 1;
                int frame = (int)(timer / (duration / (float)n));
                if (frame >= n) frame = n - 1;
                float uv_w = 1.f / (float)sheet.cols;
                float uv_h = 1.f / (float)sheet.rows;
                int   col  = frame % sheet.cols;
                int   row  = frame / sheet.cols;
                t.position.y += rendering_top ? 0.5f : -0.5f;
                ShaderSetVector(shaders, "bot_left_uv",  Vector2{ 0.f, 0.f });
                ShaderSetVector(shaders, "top_right_uv", Vector2{ uv_w, uv_h * 0.5f });
                ShaderSetVector(shaders, "uv_offset",    Vector2{ col * uv_w, row * uv_h + (rendering_top ? uv_h * 0.5f : 0.f) });
                DrawSprite(sheet.sprite, t, main_camera);
                UVReset(shaders);
            };

            if (door->anim_state == DOOR_ANIM_OPEN) {
                DrawSprite(sprites[is_horiz ? SPR_DOOR_OPEN_H : SPR_DOOR_OPEN_V], t, main_camera);
            } else if (door->anim_state == DOOR_ANIM_OPENING) {
                DrawAnimFrame(door_open_sheet, door->anim_timer, door->anim_duration);
            } else if (door->anim_state == DOOR_ANIM_CLOSING) {
                DrawAnimFrame(door_close_sheet, door->anim_timer, door->anim_duration);
            } else {
                // CLOSED
                int spr = is_horiz ? SPR_DOOR_CLOSED_H : SPR_DOOR_CLOSED_V;
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
            if (btn->anim_state == BUTTON_ANIM_DOWN) {
                DrawSprite(sprites[SPR_BUTTON_DOWN], rt->transform, main_camera);
            } else if (btn->anim_state == BUTTON_ANIM_UP) {
                DrawSprite(sprites[SPR_BUTTON_UP], rt->transform, main_camera);
            } else {
                SpriteSheet& sheet = (btn->anim_state == BUTTON_ANIM_PRESSING) ? button_down_sheet : button_up_sheet;
                int n     = sheet.frame_count > 0 ? sheet.frame_count : 1;
                int frame = (int)(btn->anim_timer / (btn->anim_duration / (float)n));
                if (frame >= n) frame = n - 1;
                DrawSpriteSheetFrame(&sheet, frame, rt->transform, main_camera, shaders);
            }
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
            Transform t{};
            CopyTransform(&t, rt->transform);

            ColorMul(shaders, tp->color, !level_transitioning);
            // RENDER TOP HALF
            t.position.y +=0.5f;
            t.position.z += 1.5f;
            UVTopHalfAnim(shaders, teleporter_anim_frame, 3);
            DrawSprite(teleporter_anim_sheet.sprite, t, main_camera);

            // RENDER BOTTOM HALF
            t.position.y -= 1.0f;
            t.position.z -= 1.25f;
            UVBottomHalfAnim(shaders, teleporter_anim_frame, 3);
            DrawSprite(teleporter_anim_sheet.sprite, t, main_camera);
            ColorMulReset(shaders, !level_transitioning);
            UVReset(shaders);
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
            t.position.z += 0.1f;
            DrawSprite(sprites[SPR_CC_FRAME], t, main_camera);
            UVReset(shaders);
            ColorMulReset(shaders, !level_transitioning);
            // Laser visualization (only on top pass; uses frame-transient input colors)
            if (rendering_top) {
                ShaderSetVector(shaders, "bot_left_uv",  Vector2{ 0.f,       0.f });
                ShaderSetVector(shaders, "top_right_uv", Vector2{ 1.f / 5.f, 1.f });
                t.scale.y      = 2.f;
                t.position.y  -= 0.5f;
                t.position.z  += 0.1f;
                
                // Combined output beam — only when both beams are active
                if (cc->horizontal_input_color.a > 0 && cc->vertical_input_color.a > 0) {
                    ShaderSetVector(shaders, "uv_offset", Vector2{ 0.f, 0.f });
                    ColorMul(shaders, cc->horizontal_input_color, !level_transitioning);
                    DrawSprite(sprites[SPR_CC_OVERLAY], t, main_camera);
                    
                    ShaderSetVector(shaders, "uv_offset", Vector2{ 1.f / 5.f, 0.f });
                    t.position.z += 0.1f;
                    ColorMul(shaders, cc->vertical_input_color, !level_transitioning);
                    DrawSprite(sprites[SPR_CC_OVERLAY], t, main_camera);
                    
                    ShaderSetVector(shaders, "uv_offset", Vector2{ 2.f / 5.f, 0.f });
                    t.position.z += 0.1f;
                    ColorMul(shaders, AddColor(cc->horizontal_input_color, cc->vertical_input_color), !level_transitioning);
                    DrawSprite(sprites[SPR_CC_OVERLAY], t, main_camera);
                } else if (cc->horizontal_input_color.a > 0) {
                    ShaderSetVector(shaders, "uv_offset", Vector2{ 0.f, 0.f });
                    ColorMul(shaders, cc->horizontal_input_color, !level_transitioning);
                    DrawSprite(sprites[SPR_CC_OVERLAY], t, main_camera);
                    
                    ShaderSetVector(shaders, "uv_offset", Vector2{ 2.f / 5.f, 0.f });
                    t.position.z += 0.1f;
                    ColorMul(shaders, cc->horizontal_input_color, !level_transitioning);
                    DrawSprite(sprites[SPR_CC_OVERLAY], t, main_camera);
                } else if (cc->vertical_input_color.a > 0) {
                    ShaderSetVector(shaders, "uv_offset", Vector2{ 1.f / 5.f, 0.f });
                    t.position.z += 0.1f;
                    ColorMul(shaders, cc->vertical_input_color, !level_transitioning);
                    DrawSprite(sprites[SPR_CC_OVERLAY], t, main_camera);
                    
                    ShaderSetVector(shaders, "uv_offset", Vector2{ 2.f / 5.f, 0.f });
                    t.position.z += 0.1f;
                    ColorMul(shaders, cc->vertical_input_color, !level_transitioning);
                    DrawSprite(sprites[SPR_CC_OVERLAY], t, main_camera);
                }

                UVReset(shaders);
                ColorMulReset(shaders, !level_transitioning);

                t.scale.y = rt->transform.scale.y;
                t.position.y += 0.5f;
                t.position.z += 0.1f;

                ColorMul(shaders, cc->main_color, !level_transitioning);
                if (rendering_top) UVTopHalf(shaders); else UVBottomHalf(shaders);
                const Sprite& cc_sprite = (cc->mode == ColorBlendMode::Subtractive) ? color_changer_subtractive_sprite
                                : (cc->mode == ColorBlendMode::Additive)    ? color_changer_additive_sprite
                                :                                              sprites[SPR_COLOR_CHANGER];
                DrawSprite(cc_sprite, t, main_camera);
                ColorMulReset(shaders, !level_transitioning);
                UVReset(shaders);

            }
            goto done;
        }
    }

    // ---- Endgoal ----
    if (ca->endgoal_arr.Get(entity_id)) {
        ColorTag* ct = ca->color_tag_arr.Get(entity_id);
        DrawSprite(sprites[SPR_ENDGOAL], rt->transform, main_camera);
        rt->transform.position.z += 0.05f;
        UVAnim(shaders, endgoal_overlay_anim_frame, endgoal_overlay_anim_sheet.frame_count);
        if (ct) ColorMul(shaders, ct->color, !level_transitioning);
        DrawSprite(endgoal_overlay_anim_sheet.sprite, rt->transform, main_camera);
        if (ct) ColorMulReset(shaders, !level_transitioning);
        UVReset(shaders);
        goto done;
    }

    // ---- Push block (has GridMover, no special component above) ----
    if (gm) {
        ColorTag* ct = ca->color_tag_arr.Get(entity_id);
        Transform t{};
        CopyTransform(&t, rt->transform);
        t.position.y += rendering_top ? 0.5f : -0.5f;
        if (rendering_top) UVTopHalf(shaders); else UVBottomHalf(shaders);
        if (ct) ColorMul(shaders, ct->color, !level_transitioning);
        
        PushMoveConstraint* pmc = ca->push_move_constraint_arr.Get(entity_id);

        if(pmc->dir == PushBlockMoveDir::Default)DrawSprite(sprites[SPR_PUSH_BLOCK], t, main_camera);
        if(pmc->dir == PushBlockMoveDir::HorizontalOnly)DrawSprite(sprites[SPR_PUSH_BLOCK_HORIZ], t, main_camera);
        if(pmc->dir == PushBlockMoveDir::VerticalOnly)DrawSprite(sprites[SPR_PUSH_BLOCK_VERT], t, main_camera);
        
        if (ct) ColorMulReset(shaders, !level_transitioning);
        UVReset(shaders);
        goto done;
    }

    // ---- Pickup ----
    {
        Pickup* pk = ca->pickup_arr.Get(entity_id);
        if (pk && !pk->consumed) {
            float hover = sinf(global_anim_timer * 0.003f) * 0.15f;
            Transform t{};
            CopyTransform(&t, rt->transform);
            t.position.y += hover;
            DrawSprite(letter_sprite, t, main_camera);
            goto done;
        }
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
    if (do_portal) ResetPortalClip(shaders);
    if (shake_x != 0.f) rt->transform.position.x -= shake_x;
    if (z_bumped) rt->transform.position.z -= 1.3f;

    // Exit render: second draw at the exit portal position with complementary clip
    if (do_portal && !exit_render) {
        float saved_x = rt->transform.position.x;
        float saved_y = rt->transform.position.y;
        float saved_z = rt->transform.position.z;
        float base_z  = (gp && gp->layer == GridLayer::EntityLayer) ? 1.f : 0.f;
        rt->transform.position.x = gm->teleport_exit_x;
        rt->transform.position.y = gm->teleport_exit_y;
        rt->transform.position.z = base_z - 2.f * (float)gm->teleport_exit.y;
        EntityRender(entity_id, ca, shaders, sprites, rendering_top, level_transitioning, force_render, true);
        rt->transform.position.x = saved_x;
        rt->transform.position.y = saved_y;
        rt->transform.position.z = saved_z;
    }
}
