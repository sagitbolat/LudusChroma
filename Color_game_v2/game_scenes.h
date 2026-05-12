// ============================================================
// SECTION: Title-card text helper
// ============================================================

static bool DisplayTextAnim(Sprite spr, Transform t,
    float delay, float fade_in, float linger, float fade_out, float post_delay, float timer) {
    float total = delay + fade_in + linger + fade_out + post_delay;
    if (timer >= total) return false;
    float alpha = 0.f;
    if      (timer < delay)                              alpha = 0.f;
    else if (timer < delay + fade_in)                    alpha = (timer - delay) / fade_in;
    else if (timer < total - fade_out - post_delay)      alpha = 1.f;
    else if (timer < total - post_delay)                 alpha = 1.f - (timer - (total - fade_out - post_delay)) / fade_out;
    ShaderSetVector(shaders, "i_color_multiplier", Vec4(fColor{ 1.f, 1.f, 1.f, alpha }));
    DrawSprite(spr, t, main_camera);
    ShaderSetVector(shaders, "i_color_multiplier", Vector4{ 1.f, 1.f, 1.f, 1.f });
    return true;
}


// ============================================================
// SECTION: Scene functions
// ============================================================

// ------------ Title scene ------------

static float title_timer = 0.f;

void TitleUpdate(GameState* gs, KeyboardState* ks, double dt);
void Act2Update(GameState* gs, KeyboardState* ks, double dt);
void GameUpdate(GameState* gs, KeyboardState* ks, double dt);

static void SceneNoOp(GameState*, KeyboardState*, double) {}
static void SceneNoOpVoid() {}

void TitleUpdate(GameState* gs, KeyboardState* ks, double dt) {
    title_timer += (float)dt;

    Transform t{};
    t.position.x = float(tilemap.width / 2);
    t.position.y = main_camera.position.y + 1.f;
    t.scale      = { 2.25f, 1.f, 1.f };

    DisplayTextAnim(title_text,  t, 1000.f, 2000.f, 4000.f, 1000.f, 1000.f, title_timer);
    t.scale      = { 3.3472f * 2.f, 2.f, 1.f };
    t.position.y -= 2.f;
    DisplayTextAnim(title_text2, t, 3000.f, 2000.f, 2000.f, 1000.f, 1000.f, title_timer);

    t.position.y += 1.f;
    t.scale      = { 1.685f * 2.f, 2.f, 1.f };
    t.position.z += 1.f;
    DisplayTextAnim(act1_text,   t, 10000.f, 2000.f, 2000.f, 1000.f, 1000.f, title_timer);
    t.position.y -= 1.5f;
    t.scale      = { 7.5f, 0.5f, 1.f };
    t.position.z += 1.f;

    if (!DisplayTextAnim(act1_1_text, t, 12000.f, 2000.f, 3000.f, 1000.f, 1000.f, title_timer)) {
        scene_manager.SwitchScene(2, gs, ks, dt);
    }

    if (ks->state.SPACE && !ks->prev_state.SPACE) {
        scene_manager.SwitchScene(2, gs, ks, dt);
    }
}

static float act2_timer = 0.f;

void Act2Update(GameState* gs, KeyboardState* ks, double dt) {
    act2_timer += (float)dt;

    Transform t{};
    t.position.x = float(tilemap.width / 2);
    t.position.y = main_camera.position.y + 0.5f;
    t.scale      = { 1.685f * 2.f, 2.f, 1.f };

    DisplayTextAnim(act2_text,   t, 1000.f, 2000.f, 2000.f, 1000.f, 1000.f, act2_timer);
    t.position.y -= 1.5f;
    t.scale      = { 7.5f, 0.5f, 1.f };
    t.position.z += 1.f;

    if (!DisplayTextAnim(act1_1_text, t, 3000.f, 2000.f, 3000.f, 1000.f, 1000.f, act2_timer)) {
        scene_manager.SwitchScene(2, gs, ks, dt);
    }

    if (ks->state.SPACE && !ks->prev_state.SPACE) {
        scene_manager.SwitchScene(2, gs, ks, dt);
    }
}


// ------------ Game scene ------------

void GameUpdate(GameState* gs, KeyboardState* ks, double dt) {

    // ---- First-load zoom-in transition ----
    if (first_load) {
        static float t = 0.f;
        const float DUR = 1000.f, ZOOM_EXTRA = 3.f;
        t += (float)dt;
        if (t < DUR) {
            float frac = t / DUR;
            main_camera.width  = (level_zoom[curr_level_index-1] + ZOOM_EXTRA) - frac * ZOOM_EXTRA;
            main_camera.height = (float)SCREEN_HEIGHT / (float)SCREEN_WIDTH * main_camera.width;
            ShaderSetVector(shaders, "i_color_multiplier", Vec4(fColor{ 1.f, 1.f, 1.f, frac }));
        } else {
            first_load = false;
            t = 0.f;
        }
    }

    // ---- Act 2 interstitial ----
    if (curr_level_index == 13 && !played_act2_card) {
        played_act2_card = true;
        first_load = true;
        act2_timer = 0.f;
        scene_manager.SwitchScene(1, gs, ks, dt);
        return;
    }

    // ---- Editor mode (Tab toggles) ----
    if (ks->state.TAB && !ks->prev_state.TAB) {
        editor_mode = !editor_mode;
        if (editor_mode) {
            int idx = curr_level_index - 1;
            if (idx >= 0 && idx < NUM_LEVELS) {
                strncpy(ed_name, level_names[idx], 255);
                ed_name[255] = '\0';
                ed_level_idx = idx;
            }
        } else {
            num_players = 0;
            for (int i = 0; i < level_info.num_entities; ++i)
                if (comp_arrays.grid_player_controlled_arr.Get(i)) player_ids[num_players++] = i;
            UndoReallocate();
        }
    }
    if (editor_mode) {
        bool just_toggled = ks->state.TAB && !ks->prev_state.TAB;
        if (!just_toggled) EditorUpdate(ks, dt);
        GameRender();
        return;
    }

    // ---- Level transition animation ----
    if (level_transitioning) {
        static float trans_t   = 0.f;
        static bool  fading_in = true;
        const float  DUR       = 500.f;
        const float  ZOOM_EXTRA = 3.f;

        trans_t += (float)dt;

        if (fading_in) {
            if (trans_t < DUR) {
                int  zoom_idx  = restarting_level ? curr_level_index : curr_level_index - 1;
                main_camera.width  = level_zoom[zoom_idx] + (trans_t / DUR) * ZOOM_EXTRA;
                main_camera.height = (float)SCREEN_HEIGHT / (float)SCREEN_WIDTH * main_camera.width;
                float alpha = 1.f - (trans_t / (DUR / 2.f));
                ShaderSetVector(shaders, "i_color_multiplier",
                    Vec4(fColor{ 1.f, 1.f, 1.f, FloatClamp(alpha, 0.f, 1.f) }));
            } else {
                LoadLevel(curr_level_index);
                ++curr_level_index;
                restarting_level = false;
                showing_wires    = false;
                ShaderSetVector(shaders, "i_color_multiplier", Vector4{ 1.f, 1.f, 1.f, 1.f });
                main_camera.width  = level_zoom[curr_level_index - 1];
                main_camera.height = (float)SCREEN_HEIGHT / (float)SCREEN_WIDTH * main_camera.width;
                CenterCamera();
                trans_t   = 0.f;
                fading_in = false;
                return;
            }
        } else {
            if (trans_t < DUR) {
                float frac = trans_t / DUR;
                main_camera.width  = (level_zoom[curr_level_index-1] + ZOOM_EXTRA) - frac * ZOOM_EXTRA;
                main_camera.height = (float)SCREEN_HEIGHT / (float)SCREEN_WIDTH * main_camera.width;
                ShaderSetVector(shaders, "i_color_multiplier", Vec4(fColor{ 1.f, 1.f, 1.f, frac }));
            } else {
                ShaderSetVector(shaders, "i_color_multiplier", Vector4{ 1.f, 1.f, 1.f, 1.f });
                level_transitioning = false;
                trans_t   = 0.f;
                fading_in = true;
            }
        }
    }

    // ---- Input ----
    if (!level_transitioning && num_players > 0) {
        int player_id = player_ids[0];
        GridPosition*         gp     = comp_arrays.grid_position_arr.Get(player_id);
        GridMover*            gm     = comp_arrays.grid_mover_arr.Get(player_id);
        GridPlayerControlled* player = comp_arrays.grid_player_controlled_arr.Get(player_id);

        if (gp && gm && player && !gm->moving) {
            player->orientation = Direction::Neutral;

            auto try_move = [&](Vector2Int dir, Direction face) {
                UndoSaveStep();
                bool moved = EntityMove(player_id, dir, tilemap, entity_map, &comp_arrays, MAX_ENTITIES);
                if (moved) {
                    player->orientation = face;
                } else {
                    undo_ptr -= undo_num_movers;
                }
            };

            if (!gm->moving && (ks->state.W || ks->state.ARROWUP))    try_move({ 0,  1 }, Direction::Up);
            if (!gm->moving && (ks->state.S || ks->state.ARROWDOWN))   try_move({ 0, -1 }, Direction::Down);
            if (!gm->moving && (ks->state.A || ks->state.ARROWLEFT))   try_move({-1,  0 }, Direction::Left);
            if (!gm->moving && (ks->state.D || ks->state.ARROWRIGHT))  try_move({ 1,  0 }, Direction::Right);
        }

        if (ks->state.U && !ks->prev_state.U) {
            UndoRevertStep();
        }

        if (ks->state.F && !ks->prev_state.F) {
            showing_wires = !showing_wires;
        }

        if (ks->state.R && !ks->prev_state.R) {
            showing_wires       = false;
            level_transitioning = true;
            restarting_level    = true;
            --curr_level_index;
        }

        if (gp && gm && !gm->moving) {
            int floor_id = entity_map.GetID(gp->position.x, gp->position.y, (int)GridLayer::GroundLayer);
            if (floor_id >= 0 && comp_arrays.endgoal_arr.Get(floor_id)) {
                showing_wires       = false;
                level_transitioning = true;
                if (curr_level_index >= NUM_LEVELS) curr_level_index = 0;
            }
        }

#ifdef DEBUG_MODE
        if (ks->state.Q && !ks->prev_state.Q) {
            --curr_level_index;
            if (curr_level_index < 0) curr_level_index = NUM_LEVELS - 1;
            LoadLevel(curr_level_index);
        }
        if (ks->state.E && !ks->prev_state.E) {
            ++curr_level_index;
            if (curr_level_index >= NUM_LEVELS) curr_level_index = 0;
            LoadLevel(curr_level_index);
        }
#endif
    }

    GameRender();

    // ---- Per-frame ECS updates ----
    ClearFrameState(&comp_arrays);
    for (int i = 0; i < level_info.num_entities; ++i) EntityUpdateEmit(i, &comp_arrays, tilemap, entity_map, emission_map);
    for (int i = 0; i < level_info.num_entities; ++i) EntityUpdateMover(i, &comp_arrays, (float)dt);
    for (int i = 0; i < level_info.num_entities; ++i) {
        EntityUpdateReceiver(i, &comp_arrays);
        EntityUpdateButton(i, &comp_arrays, entity_map);
    }
    for (int i = 0; i < level_info.num_entities; ++i) EntityUpdateDoor(i, &comp_arrays, entity_map, level_info.num_entities);
}
