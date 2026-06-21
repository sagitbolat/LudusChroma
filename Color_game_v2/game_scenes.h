// ============================================================
// SECTION: Cutscene / dialogue system
// ============================================================

extern AudioSource* cutscene_voice_source;
extern Sprite       dialogue_frame_sprite;
extern ImFont*      dialogue_font;

static CutsceneState cutscene = {};

static void PlayLineAudio(int dialogue_id, int line) {
    StopSound(cutscene_voice_source);
    if (cutscene.clip) { FreeSoundClip(cutscene.clip); cutscene.clip = nullptr; }
    if (dialogue_id < 0 || dialogue_id >= NUM_DIALOGUES) return;
    if (line < 0 || line >= dialogues[dialogue_id].num_lines) return;
    const char* audio = dialogues[dialogue_id].audio_paths[line];
    if (audio && audio[0] != '\0') {
        cutscene.clip = LoadSoundClip(audio);
        if (cutscene.clip) PlaySoundFromSource(cutscene_voice_source, cutscene.clip);
    }
}

static void StartCutscene(int dialogue_id) {
    cutscene.active       = true;
    cutscene.dialogue_id  = dialogue_id;
    cutscene.current_line = 0;
    cutscene.text_timer   = 0.f;
    PlayLineAudio(dialogue_id, 0);
}

static void EndCutscene() {
    cutscene.active = false;
    StopSound(cutscene_voice_source);
    if (cutscene.clip) { FreeSoundClip(cutscene.clip); cutscene.clip = nullptr; }
}

static void ApplyPowerGrant(PowerGrant power) {
    switch (power) {
        case PowerGrant::ColorSwitcher: player_progress.color_switcher_unlocked = true; break;
        case PowerGrant::UnlockRed:     player_progress.red_unlocked            = true; break;
        case PowerGrant::UnlockGreen:   player_progress.green_unlocked          = true; break;
        case PowerGrant::UnlockBlue:    player_progress.blue_unlocked           = true; break;
        default: break;
    }
}

// Draws the dialogue overlay and handles SPACE to advance lines.
// Returns true while the cutscene is still active.
static bool RenderCutscene(KeyboardState* ks, double dt) {
    if (!cutscene.active) return false;
    if (cutscene.dialogue_id < 0 || cutscene.dialogue_id >= NUM_DIALOGUES) {
        EndCutscene();
        return false;
    }
    DialogueEntry& dlg = dialogues[cutscene.dialogue_id];

    const char* line     = (cutscene.current_line < dlg.num_lines) ? dlg.text_lines[cutscene.current_line] : "";
    int         line_len = line ? (int)strlen(line) : 0;

    const float CHARS_PER_SEC = 40.f;
    cutscene.text_timer      += (float)dt;
    int  chars_visible = (int)(cutscene.text_timer * CHARS_PER_SEC / 1000.f);
    if (chars_visible > line_len) chars_visible = line_len;
    bool typing_done   = (chars_visible >= line_len);

    // SPACE: skip typewriter if still animating, else advance to next line.
    if (ks->state.SPACE && !ks->prev_state.SPACE) {
        if (!typing_done) {
            cutscene.text_timer = (float)line_len * 1000.f / CHARS_PER_SEC + 1.f;
            chars_visible       = line_len;
        } else {
            ++cutscene.current_line;
            cutscene.text_timer = 0.f;
            if (cutscene.current_line >= dlg.num_lines) {
                ApplyPowerGrant(dlg.power);
                EndCutscene();
                return false;
            }
            PlayLineAudio(cutscene.dialogue_id, cutscene.current_line);
        }
    }

    // Frame: 90% of screen width, height = width/8, bottom-padded.
    ImGuiIO& io      = ImGui::GetIO();
    float sw         = io.DisplaySize.x;
    float sh         = io.DisplaySize.y;
    float frame_w    = sw * 0.9f;
    float frame_h    = frame_w / 8.0f;
    float frame_x    = (sw - frame_w) * 0.5f;
    float bottom_pad = sh * 0.03f;
    float box_y      = sh - frame_h - bottom_pad;

    ImGui::SetNextWindowPos(ImVec2(0, box_y));
    ImGui::SetNextWindowSize(ImVec2(sw, frame_h + bottom_pad));
    ImGui::SetNextWindowBgAlpha(0.0f);
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoInputs |
                             ImGuiWindowFlags_NoNav        | ImGuiWindowFlags_NoMove   |
                             ImGuiWindowFlags_NoBringToFrontOnFocus;
    ImGui::Begin("##cutscene", nullptr, flags);

    ImVec2 win_pos    = ImGui::GetWindowPos();
    ImVec2 frame_min  = ImVec2(win_pos.x + frame_x, win_pos.y);
    ImVec2 frame_max  = ImVec2(frame_min.x + frame_w, frame_min.y + frame_h);
    ImGui::GetWindowDrawList()->AddImage(
        (ImTextureID)(void*)SkyGetGLID(dialogue_frame_sprite.texture_id),
        frame_min, frame_max,
        ImVec2(0, 1), ImVec2(1, 0)
    );

    // Text — use dialogue_font if loaded, otherwise fall back to the default.
    if (dialogue_font) ImGui::PushFont(dialogue_font);

    float text_x      = frame_x + frame_w * 0.06f;
    float text_wrap_x = frame_x + frame_w * 0.94f;
    ImGui::SetCursorPos(ImVec2(text_x, frame_h * 0.15f));
    ImGui::PushTextWrapPos(text_wrap_x);
    ImGui::TextUnformatted(line, line + chars_visible);
    ImGui::PopTextWrapPos();

    // "SPACE to continue" hint anchored to right edge of frame
    ImVec2 hint_size = ImGui::CalcTextSize("[ SPACE ]");
    ImGui::SetCursorPos(ImVec2(frame_x + frame_w * 0.94f - hint_size.x, frame_h * 0.72f));
    ImGui::TextUnformatted("[ SPACE ]");

    if (dialogue_font) ImGui::PopFont();

    ImGui::End();
    return true;
}


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
    t.position.y = main_camera.position.y + 1.5f;
    t.scale      = { 2.21621622f * 1.5f, 1.5f, 1.f };

    DisplayTextAnim(title_text,  t, 1000.f, 2000.f, 4000.f, 1000.f, 1000.f, title_timer);
    t.scale      = { 3.28378378f * 1.5f, 1.5f, 1.f };
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
            main_camera.width  = (levels[curr_level_index-1].zoom + ZOOM_EXTRA) - frac * ZOOM_EXTRA;
            main_camera.height = (float)SCREEN_HEIGHT / (float)SCREEN_WIDTH * main_camera.width;
            ShaderSetVector(shaders, "i_color_multiplier", Vec4(fColor{ 1.f, 1.f, 1.f, frac }));
        } else {
            first_load = false;
            t = 0.f;
        }
    }

    // ---- Act 2 interstitial ----
    /*if (curr_level_index == 13 && !played_act2_card) {
        played_act2_card = true;
        first_load = true;
        act2_timer = 0.f;
        scene_manager.SwitchScene(1, gs, ks, dt);
        return;
    }*/

    // ---- Editor mode (Tab toggles) ----
    if (ks->state.TAB && !ks->prev_state.TAB) {
        editor_mode = !editor_mode;
        if (editor_mode) {
            int idx = curr_level_index - 1;
            if (idx >= 0 && idx < NUM_LEVELS) {
                strncpy(ed_name, levels[idx].name, 255);
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

    // ---- Screenshot export (F5 cycles through all levels) ----
    if (ks->state.F5 && !ks->prev_state.F5) {
        exporting_screenshots = true;
        export_idx            = 0;
        level_transitioning   = false;
        LoadLevel(export_idx);
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
                main_camera.width  = levels[zoom_idx].zoom + (trans_t / DUR) * ZOOM_EXTRA;
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
                main_camera.width  = levels[curr_level_index - 1].zoom;
                main_camera.height = (float)SCREEN_HEIGHT / (float)SCREEN_WIDTH * main_camera.width;
                CenterCamera();
                trans_t   = 0.f;
                fading_in = false;
                return;
            }
        } else {
            if (trans_t < DUR) {
                float frac = trans_t / DUR;
                main_camera.width  = (levels[curr_level_index-1].zoom + ZOOM_EXTRA) - frac * ZOOM_EXTRA;
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

    // ---- Pickup overlap check ----
    if (!cutscene.active && !level_transitioning) {
        for (int p = 0; p < num_players; ++p) {
            GridPosition* gp = comp_arrays.grid_position_arr.Get(player_ids[p]);
            GridMover*    gm = comp_arrays.grid_mover_arr.Get(player_ids[p]);
            if (!gp || (gm && gm->moving)) continue;
            int floor_id = entity_map.GetID(gp->position.x, gp->position.y, (int)GridLayer::GroundLayer);
            if (floor_id < 0) continue;
            Pickup* pk = comp_arrays.pickup_arr.Get(floor_id);
            if (!pk || pk->consumed) continue;
            pk->consumed = true;
            entity_map.SetID(gp->position.x, gp->position.y, (int)GridLayer::GroundLayer, -1);
            StartCutscene(pk->dialogue_id);
            break;
        }
    }

    // ---- Input ----
    if (!level_transitioning && !cutscene.active && num_players > 0) {
        bool any_moving = false;
        for (int p = 0; p < num_players; ++p) {
            GridMover* gm = comp_arrays.grid_mover_arr.Get(player_ids[p]);
            if (gm && gm->moving) { any_moving = true; break; }
        }

        if (!any_moving) {
            for (int p = 0; p < num_players; ++p) {
                GridPlayerControlled* pc = comp_arrays.grid_player_controlled_arr.Get(player_ids[p]);
                if (pc) pc->orientation = Direction::Neutral;
            }

            auto try_move_all = [&](Vector2Int dir, Direction face) {
                auto local_dir = [&](int pid) -> Vector2Int {
                    Direction up = comp_arrays.grid_player_controlled_arr.Get(pid)->upwards_direction;
                    switch (up) {
                        case Direction::Down:  return { -dir.x, -dir.y };
                        case Direction::Left:  return { -dir.y,  dir.x };
                        case Direction::Right: return {  dir.y, -dir.x };
                        default:               return dir;
                    }
                };

                // ---- Phase 1: collect active players and their intended targets ----
                struct Intent { int pid; Vector2Int target; Vector2Int pdir; };
                Intent intents[MAX_NUM_PLAYERS];
                int    n = 0;
                for (int p = 0; p < num_players; ++p) {
                    int pid = player_ids[p];
                    if (isHidden(pid, &comp_arrays)) continue;
                    GridMover*    gm_p = comp_arrays.grid_mover_arr.Get(pid);
                    if (gm_p && gm_p->moving) continue;
                    GridPosition* gp = comp_arrays.grid_position_arr.Get(pid);
                    if (!gp) continue;
                    Vector2Int pdir = local_dir(pid);
                    intents[n++] = { pid, { gp->position.x + pdir.x, gp->position.y + pdir.y }, pdir };
                }

                // ---- Phase 2: conflict detection ----
                bool blocked[MAX_NUM_PLAYERS] = {};

                // Same-cell: two players targeting the same position
                for (int i = 0; i < n; ++i)
                    for (int j = i + 1; j < n; ++j)
                        if (intents[i].target.x == intents[j].target.x &&
                            intents[i].target.y == intents[j].target.y)
                            blocked[i] = blocked[j] = true;

                // Head-on: A targets B's cell and B targets A's cell
                for (int i = 0; i < n; ++i) {
                    if (blocked[i]) continue;
                    GridPosition* gp_i = comp_arrays.grid_position_arr.Get(intents[i].pid);
                    for (int j = i + 1; j < n; ++j) {
                        if (blocked[j]) continue;
                        GridPosition* gp_j = comp_arrays.grid_position_arr.Get(intents[j].pid);
                        if (!gp_i || !gp_j) continue;
                        if (intents[i].target.x == gp_j->position.x &&
                            intents[i].target.y == gp_j->position.y &&
                            intents[j].target.x == gp_i->position.x &&
                            intents[j].target.y == gp_i->position.y)
                            blocked[i] = blocked[j] = true;
                    }
                }

                // ---- Phase 3: dependency ordering ----
                // dep[i] = index of the player that i must wait for (-1 if none)
                int dep[MAX_NUM_PLAYERS];
                for (int i = 0; i < n; ++i) {
                    dep[i] = -1;
                    if (blocked[i]) continue;
                    for (int j = 0; j < n; ++j) {
                        if (i == j || blocked[j]) continue;
                        GridPosition* gp_j = comp_arrays.grid_position_arr.Get(intents[j].pid);
                        if (!gp_j) continue;
                        if (intents[i].target.x == gp_j->position.x &&
                            intents[i].target.y == gp_j->position.y) {
                            dep[i] = j;
                            break;
                        }
                    }
                }

                // Detect cycles and block every player in them
                for (int i = 0; i < n; ++i) {
                    if (blocked[i] || dep[i] < 0) continue;
                    int chain[MAX_NUM_PLAYERS];
                    int clen = 0;
                    int cur  = i;
                    while (cur >= 0 && !blocked[cur]) {
                        bool seen = false;
                        for (int k = 0; k < clen && !seen; ++k)
                            if (chain[k] == cur) seen = true;
                        if (seen) {
                            for (int k = 0; k < clen; ++k) blocked[chain[k]] = true;
                            break;
                        }
                        chain[clen++] = cur;
                        cur = dep[cur];
                    }
                }

                // Topological sort: repeatedly emit players whose dependency is resolved
                int  order[MAX_NUM_PLAYERS];
                int  ocount = 0;
                bool in_order[MAX_NUM_PLAYERS] = {};
                for (bool progress = true; progress; ) {
                    progress = false;
                    for (int i = 0; i < n; ++i) {
                        if (blocked[i] || in_order[i]) continue;
                        int d = dep[i];
                        if (d < 0 || blocked[d] || in_order[d]) {
                            order[ocount++] = i;
                            in_order[i]     = true;
                            progress        = true;
                        }
                    }
                }

                // ---- Phase 4: commit moves in dependency order ----
                UndoSaveStep();
                bool any_moved = false;
                for (int o = 0; o < ocount; ++o) {
                    int i   = order[o];
                    int pid = intents[i].pid;
                    if (EntityMove(pid, intents[i].pdir, tilemap, entity_map, &comp_arrays, MAX_ENTITIES)) {
                        any_moved = true;
                        GridPlayerControlled* pc = comp_arrays.grid_player_controlled_arr.Get(pid);
                        if (pc) pc->orientation = face;
                    }
                }
                if (!any_moved) undo_ptr -= undo_num_movers;
            };

            if (ks->state.W || ks->state.ARROWUP)    try_move_all({ 0,  1 }, Direction::Up);
            else if (ks->state.S || ks->state.ARROWDOWN)   try_move_all({ 0, -1 }, Direction::Down);
            else if (ks->state.A || ks->state.ARROWLEFT)   try_move_all({-1,  0 }, Direction::Left);
            else if (ks->state.D || ks->state.ARROWRIGHT)  try_move_all({ 1,  0 }, Direction::Right);
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

        if (ks->state.C && !ks->prev_state.C && iris_timer <= 0.f) {
            if (level_info.color_switcher_mode == ColorSwitcherMode::Default && 
                CheckHiddenColorSwitch(&entity_map, &comp_arrays)) 
            {
                uint8_t next_idx      = (curr_hidden_color + 1) % num_hidden_colors;
                iris_overlay_color    = hidden_color_array[next_idx];
                iris_masked_tag_color = hidden_color_array[next_idx];
                iris_revealing_color  = hidden_color_array[curr_hidden_color];
                iris_expanding        = true;
                iris_timer            = IRIS_DURATION;
            }
        }

        {
            int num_endgoals = (int)comp_arrays.endgoal_arr.dense.size();
            int covered      = 0;
            for (int e = 0; e < num_endgoals; ++e) {
                int eg_id        = comp_arrays.endgoal_arr.dense_ids[e];
                GridPosition* eg_gp = comp_arrays.grid_position_arr.Get(eg_id);
                if (!eg_gp) continue;
                ColorTag* eg_ct = comp_arrays.color_tag_arr.Get(eg_id);
                for (int p = 0; p < num_players; ++p) {
                    GridPosition* gp = comp_arrays.grid_position_arr.Get(player_ids[p]);
                    GridMover*    gm = comp_arrays.grid_mover_arr.Get(player_ids[p]);
                    if (gp && gm && !gm->moving &&
                        gp->position.x == eg_gp->position.x &&
                        gp->position.y == eg_gp->position.y) {
                        ColorTag* p_ct = comp_arrays.color_tag_arr.Get(player_ids[p]);
                        if (!eg_ct || !p_ct || !(p_ct->color == eg_ct->color)) continue;
                        ++covered;
                        break;
                    }
                }
            }
            if (num_endgoals > 0 && covered == num_endgoals) {
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
    RenderCutscene(ks, dt);

    // ---- Shake timers ----
    for (int s = 0; s < MAX_SHAKE_ENTRIES; ++s) {
        if (shake_entries[s].timer > 0.f) {
            shake_entries[s].timer -= (float)dt;
            if (shake_entries[s].timer <= 0.f)
                shake_entries[s] = { -1, 0.f };
        }
    }

    // ---- Iris timer ----
    if (iris_timer > 0.f) {
        iris_timer -= (float)dt;
        if (iris_timer <= 0.f) {
            iris_timer            = 0.f;
            CommitHiddenColorSwitch(&entity_map, &comp_arrays);
            SetClearColor(ToFColor(hidden_color_array[curr_hidden_color]));
            iris_masked_tag_color = {};
            iris_revealing_color  = {};
        }
    }

    // ---- Per-frame ECS updates ----
    ClearFrameState(&comp_arrays);
    for (int i = 0; i < level_info.num_entities; ++i) { 
        LaserEmitter* laser_emitter = comp_arrays.laser_emitter_arr.Get(i);
        // NOTE: do not render emissions that are hidden due to the color_switcher hidden color.
        if (laser_emitter != nullptr && laser_emitter->color == hidden_color_array[curr_hidden_color]) continue;
        EntityUpdateEmit(i, &comp_arrays, tilemap, entity_map, emission_map); 
    }
    for (int i = 0; i < level_info.num_entities; ++i) EntityUpdateMover(i, &comp_arrays, (float)dt);
    for (int i = 0; i < level_info.num_entities; ++i) {
        EntityUpdateReceiver(i, &comp_arrays);
        EntityUpdateButton(i, &comp_arrays, entity_map, (float)dt);
    }
    for (int i = 0; i < level_info.num_entities; ++i) EntityUpdateDoor(i, &comp_arrays, entity_map, level_info.num_entities, (float)dt);
    
    global_anim_timer += (float)dt;
    int global_anim_frame = (int)(global_anim_timer / GLOBAL_FRAME_DURATION);
    teleporter_anim_frame = global_anim_frame % 3;
    endgoal_overlay_anim_frame = global_anim_frame % 7;
}
