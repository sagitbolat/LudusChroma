// ============================================================
// SECTION: Editor helpers
// ============================================================

static void EditorDeleteEntity(int id) {
    // Clear entity_map slots for this entity
    for (int z = 0; z < 2; ++z) {
        GridPosition* gp = comp_arrays.grid_position_arr.Get(id);
        if (gp) {
            if (entity_map.GetID(gp->position.x, gp->position.y, z) == id)
                entity_map.SetID(gp->position.x, gp->position.y, z, -1);
        }
    }
    comp_arrays.grid_position_arr.Delete(id);
    comp_arrays.grid_player_controlled_arr.Delete(id);
    comp_arrays.grid_mover_arr.Delete(id);
    comp_arrays.render_transform_arr.Delete(id);
    comp_arrays.laser_emitter_arr.Delete(id);
    comp_arrays.laser_surface_arr.Delete(id);
    comp_arrays.laser_receiver_arr.Delete(id);
    comp_arrays.signal_channel_arr.Delete(id);
    comp_arrays.door_arr.Delete(id);
    comp_arrays.endgoal_arr.Delete(id);
    comp_arrays.button_arr.Delete(id);
    comp_arrays.teleporter_arr.Delete(id);
    comp_arrays.color_changer_arr.Delete(id);
    comp_arrays.color_tag_arr.Delete(id);
}

static void EditorNewLevel(int w, int h) {
    comp_arrays.Clear();
    num_players = 0;
    for (int i = 0; i < MAX_NUM_PLAYERS; ++i) player_ids[i] = -1;
    level_info = { 2, 0 };

    tilemap.width  = w;
    tilemap.height = h;
    tilemap.map    = (int*)realloc(tilemap.map, w * h * sizeof(int));
    for (int y = 0; y < h; ++y)
        for (int x = 0; x < w; ++x) {
            int& t = tilemap.map[y * w + x];
            if      (y == 0   && x == 0  ) t = 5;
            else if (y == 0   && x == w-1) t = 6;
            else if (y == h-1 && x == 0  ) t = 10;
            else if (y == h-1 && x == w-1) t = 11;
            else if (y == 0)               t = 13;
            else if (x == 0)               t = 9;
            else if (y == h-1)             t = 3;
            else if (x == w-1)             t = 7;
            else                           t = 1;
        }

    entity_map.width  = w;  entity_map.height = h;  entity_map.depth = 2;
    entity_map.map    = (int*)realloc(entity_map.map, w * h * 2 * sizeof(int));
    for (int i = 0; i < w * h * 2; ++i) entity_map.map[i] = -1;
    ResetEmissionMap();
    CenterCamera();
}

static void EditorLoadLevel(const char* name) {
    level_info = ReadLevelState(name, &tilemap, &comp_arrays, &entity_map, player_ids, &num_players);
    ResetEmissionMap();
    CenterCamera();
}

static void EditorSaveLevel(const char* name) {
    char filepath[256]{};
    snprintf(filepath, sizeof(filepath), "%s/%s.%s", level_directory, name, level_filetype_extension);
    FILE* f = fopen(filepath, "wb");
    if (!f) { printf("EditorSaveLevel: cannot open %s\n", filepath); return; }

    int num_live = 0;
    for (int e = 0; e < level_info.num_entities; ++e)
        if (comp_arrays.grid_position_arr.Get(e)) ++num_live;

    WriteU32((uint32_t)tilemap.width,  f);
    WriteU32((uint32_t)tilemap.height, f);
    WriteU32((uint32_t)level_info.num_floor_tile_types, f);
    WriteU32((uint32_t)num_live, f);
    for (int y = 0; y < tilemap.height; ++y)
        for (int x = 0; x < tilemap.width; ++x) {
            uint16_t tile = (uint16_t)tilemap.map[y * tilemap.width + x];
            fwrite(&tile, sizeof(uint16_t), 1, f);
        }
    for (int e = 0; e < level_info.num_entities; ++e)
        WriteEntityRecord(e, &comp_arrays, f);
    fclose(f);
    printf("EditorSaveLevel: saved %s (%d entities)\n", filepath, num_live);
}


// ============================================================
// SECTION: Editor update
// ============================================================

void EditorUpdate(KeyboardState* ks, double dt) {
    static int  ed_type   = 1;
    static int  ed_sel_id = -1;
    static int  ed_w = 15, ed_h = 9;

    // ---- ImGui panel (108px strip at top of screen) ----
    UI_Window_Options wo = { true, true, true, true, false, true };
    UI_WindowStart("Editor", { 1280, 108 }, { 0, 0 }, &wo);

    {
        char fps_buf[32]; sprintf(fps_buf, "FPS: %d | EDITOR", (int)DeltaTimeToFps(dt));
        DrawSimpleText(fps_buf, { 0.99f, 0.1f }, UI_Alignment::TOP_RIGHT);
    }

    const float U = float(108) / float(1280);
    if (DrawSimpleImageButton("PB",  sprites[SPR_PUSH_BLOCK],    { U*0.2f, 0.2f }, { U*0.6f, 0.6f })) ed_type = 1;
    if (DrawSimpleImageButton("SB",  sprites[SPR_STATIC_BLOCK],  { U*1.0f, 0.2f }, { U*0.6f, 0.6f })) ed_type = 2;
    if (DrawSimpleImageButton("EM",  sprites[SPR_EMITTER],       { U*1.8f, 0.2f }, { U*0.6f, 0.6f })) ed_type = 3;
    if (DrawSimpleImageButton("RC",  sprites[SPR_RECEIVER],      { U*2.6f, 0.2f }, { U*0.6f, 0.6f })) ed_type = 4;
    if (DrawSimpleImageButton("DR",  sprites[SPR_DOOR_CLOSED_V], { U*3.4f, 0.2f }, { U*0.6f, 0.6f })) ed_type = 5;
    if (DrawSimpleImageButton("EG",  sprites[SPR_ENDGOAL],       { U*4.2f, 0.2f }, { U*0.6f, 0.6f })) ed_type = 6;
    if (DrawSimpleImageButton("BTN", sprites[SPR_BUTTON_UP],     { U*5.0f, 0.2f }, { U*0.6f, 0.6f })) ed_type = 7;
    if (DrawSimpleImageButton("TP",  sprites[SPR_TELEPORTER],    { U*5.8f, 0.2f }, { U*0.6f, 0.6f })) ed_type = 8;
    if (DrawSimpleImageButton("CC",  sprites[SPR_COLOR_CHANGER], { U*6.6f, 0.2f }, { U*0.6f, 0.6f })) ed_type = 9;
    if (DrawSimpleImageButton("PL",  sprites[SPR_PLAYER_NEUTRAL],{ U*7.4f, 0.2f }, { U*0.6f, 0.6f })) ed_type = 10;

    { char s[16]; sprintf(s, "W:%d", ed_w); DrawSimpleText(s, { U*9.3f, 0.25f }, UI_Alignment::TOP_LEFT); }
    { char s[16]; sprintf(s, "H:%d", ed_h); DrawSimpleText(s, { U*9.3f, 0.55f }, UI_Alignment::TOP_LEFT); }
    if (DrawSimpleButton("New Level", { U*9.1f, 0.75f }, { U*0.95f, 0.22f }, nullptr)) EditorNewLevel(ed_w, ed_h);

    DrawSimpleTextbox("##lvl", ed_name, 255, { U*10.15f, 0.35f }, { 108, 0 });
    if (DrawSimpleButton("Save", { U*10.05f, 0.72f }, { U*0.75f, 0.23f }, nullptr)) EditorSaveLevel(ed_name);
    if (DrawSimpleButton("Load", { U*11.0f,  0.72f }, { U*0.75f, 0.23f }, nullptr)) EditorLoadLevel(ed_name);

    UI_WindowEnd();

    // ---- Size controls ----
    if (ks->state.Z && !ks->prev_state.Z && ed_w > 3) ed_w--;
    if (ks->state.X && !ks->prev_state.X)              ed_w++;
    if (ks->state.C && !ks->prev_state.C && ed_h > 3)  ed_h--;
    if (ks->state.V && !ks->prev_state.V)               ed_h++;

    // ---- Left click: place entity ----
    if (ks->state.MBL && !ks->prev_state.MBL) {
        Vector2 wp = GetMousePositionInWorldCoords();
        int mx = (int)(wp.x + 0.5f), my = (int)(wp.y + 0.5f);
        if (mx >= 0 && my >= 0 && mx < tilemap.width && my < tilemap.height &&
            entity_map.GetID(mx, my, 0) < 0 && entity_map.GetID(mx, my, 1) < 0 &&
            !TestTileCollide(tilemap, { mx, my }))
        {
            int id = level_info.num_entities++;
            int32_t self_ch[MAX_CONNECTIONS]; for (int i=0;i<MAX_CONNECTIONS;++i) self_ch[i]=-1; self_ch[0]=id;
            int32_t empty_ch[MAX_CONNECTIONS]; for (int i=0;i<MAX_CONNECTIONS;++i) empty_ch[i]=-1;
            switch (ed_type) {
                case 1:  PushblockInit(id, &comp_arrays, {mx,my}); entity_map.SetID(mx,my,1,id); break;
                case 2:  StaticBlockInit(id, &comp_arrays, {mx,my}); entity_map.SetID(mx,my,1,id); break;
                case 3:  EmitterInit(id, &comp_arrays, {mx,my}, {255,255,255,255}, Direction::Down); entity_map.SetID(mx,my,1,id); break;
                case 4:  ReceiverInit(id, &comp_arrays, {mx,my}, {255,255,255,255}, self_ch); entity_map.SetID(mx,my,1,id); break;
                case 5:  DoorInit(id, &comp_arrays, {mx,my}, false, empty_ch); entity_map.SetID(mx,my,0,id); break;
                case 6:  EndgoalInit(id, &comp_arrays, {mx,my}); entity_map.SetID(mx,my,0,id); break;
                case 7:  ButtonInit(id, &comp_arrays, {mx,my}, self_ch); entity_map.SetID(mx,my,0,id); break;
                case 8:  TeleporterInit(id, &comp_arrays, {mx,my}, {255,250,230,255}, -1); entity_map.SetID(mx,my,0,id); break;
                case 9:  ColorChangerInit(id, &comp_arrays, {mx,my}, {255,255,255,255}, ColorBlendMode::Additive, true); entity_map.SetID(mx,my,1,id); break;
                case 10:
                    PlayerInit(id, &comp_arrays, {mx,my}, Direction::Up, {255,255,255,255});
                    entity_map.SetID(mx,my,1,id);
                    player_ids[num_players++] = id;
                    break;
                default: level_info.num_entities--; break;
            }
        }
    }

    // ---- Right click: delete entity ----
    if (ks->state.MBR && !ks->prev_state.MBR) {
        Vector2 wp = GetMousePositionInWorldCoords();
        int mx = (int)(wp.x + 0.5f), my = (int)(wp.y + 0.5f);
        int id = entity_map.GetID(mx, my, 1);
        if (id < 0) id = entity_map.GetID(mx, my, 0);
        if (id >= 0) EditorDeleteEntity(id);
    }

    // ---- R: rotate emitter / toggle door default state ----
    if (ks->state.R && !ks->prev_state.R) {
        Vector2 wp = GetMousePositionInWorldCoords();
        int mx = (int)(wp.x + 0.5f), my = (int)(wp.y + 0.5f);
        int id = entity_map.GetID(mx, my, 1);
        if (id >= 0) {
            LaserEmitter* le = comp_arrays.laser_emitter_arr.Get(id);
            if (le) le->dir = (Direction)(((int)le->dir + 1) % 4);
            
            ColorChanger* cc = comp_arrays.color_changer_arr.Get(id);
            if (cc) {
                //if (cc->mode == ColorBlendMode::Additive) cc->mode = ColorBlendMode::Subtractive;
                //else if (cc->mode == ColorBlendMode::Subtractive) cc->mode = ColorBlendMode::Blended;
                //else if (cc->mode == ColorBlendMode::Blended) cc->mode = ColorBlendMode::Additive;
                cc->mode = ColorBlendMode::Additive;
            }
            
        } else {
            id = entity_map.GetID(mx, my, 0);
            if (id >= 0) {
                Door* door = comp_arrays.door_arr.Get(id);
                if (door) door->open_by_default = !door->open_by_default;
            }
        }
    }

    // ---- T + 0-7: set color ----
    if (ks->state.T && !ks->prev_state.T) {
        static const Color ed_colors[8] = {
            DEFAULT_WHITE,DEFAULT_BLACK,DEFAULT_RED,DEFAULT_GREEN,
            DEFAULT_BLUE,DEFAULT_YELLOW,DEFAULT_MAGENTA,DEFAULT_CYAN
        };
        int ci = ks->state.NUM0?0 : ks->state.NUM1?1 : ks->state.NUM2?2 : ks->state.NUM3?3 :
                 ks->state.NUM4?4 : ks->state.NUM5?5 : ks->state.NUM6?6 : ks->state.NUM7?7 : 0;
        Color c = ed_colors[ci];
        Vector2 wp = GetMousePositionInWorldCoords();
        int mx = (int)(wp.x + 0.5f), my = (int)(wp.y + 0.5f);
        int id1 = entity_map.GetID(mx, my, 1);
        int id0 = entity_map.GetID(mx, my, 0);
        if (id1 >= 0) {
            if (auto* le = comp_arrays.laser_emitter_arr.Get(id1))          le->color          = c;
            if (auto* lr = comp_arrays.laser_receiver_arr.Get(id1))         lr->accepted_color  = c;
            if (auto* cc = comp_arrays.color_changer_arr.Get(id1))          cc->main_color      = c;
            if (auto* pc = comp_arrays.grid_player_controlled_arr.Get(id1)) pc->color           = c;
            if (auto* ct = comp_arrays.color_tag_arr.Get(id1))              ct->color           = c;
        }
        if (id0 >= 0) {
            if (auto* tp = comp_arrays.teleporter_arr.Get(id0)) tp->color = c;
        }
    }

    // ---- E hold/release: wire activator to door or link teleporters ----
    if (ks->state.E && !ks->prev_state.E) {
        Vector2 wp = GetMousePositionInWorldCoords();
        int mx = (int)(wp.x + 0.5f), my = (int)(wp.y + 0.5f);
        int id1 = entity_map.GetID(mx, my, 1);
        int id0 = entity_map.GetID(mx, my, 0);
        ed_sel_id = -1;
        if (id1 >= 0 && comp_arrays.laser_receiver_arr.Get(id1)) ed_sel_id = id1;
        else if (id0 >= 0 && comp_arrays.button_arr.Get(id0))     ed_sel_id = id0;
        else if (id0 >= 0 && comp_arrays.teleporter_arr.Get(id0)) ed_sel_id = id0;
    }
    if (!ks->state.E && ks->prev_state.E && ed_sel_id >= 0) {
        Vector2 wp = GetMousePositionInWorldCoords();
        int mx = (int)(wp.x + 0.5f), my = (int)(wp.y + 0.5f);
        int id0 = entity_map.GetID(mx, my, 0);
        if (id0 >= 0) {
            Door*       door = comp_arrays.door_arr.Get(id0);
            Teleporter* tp   = comp_arrays.teleporter_arr.Get(id0);
            if (door) {
                SignalChannel* sc = comp_arrays.signal_channel_arr.Get(id0);
                SignalChannel* ac = comp_arrays.signal_channel_arr.Get(ed_sel_id);
                if (sc && ac) {
                    int32_t chan = ac->channels[0];
                    for (int i = 0; i < MAX_CONNECTIONS; ++i)
                        if (sc->channels[i] < 0) { sc->channels[i] = chan; break; }
                }
            } else if (tp && id0 != ed_sel_id && comp_arrays.teleporter_arr.Get(ed_sel_id)) {
                tp->partner_entity_id = ed_sel_id;
                comp_arrays.teleporter_arr.Get(ed_sel_id)->partner_entity_id = id0;
            }
        }
        ed_sel_id = -1;
    }

    // ---- Q (+Shift): cycle tile index under cursor ----
    if (ks->state.Q && !ks->prev_state.Q) {
        Vector2 wp = GetMousePositionInWorldCoords();
        int mx = (int)(wp.x + 0.5f), my = (int)(wp.y + 0.5f);
        if (mx >= 0 && my >= 0 && mx < tilemap.width && my < tilemap.height) {
            int& tile  = tilemap.map[my * tilemap.width + mx];
            int  max_t = tileset.width_in_tiles * tileset.height_in_tiles - 1;
            if (ks->state.LEFTSHIFT) { if (tile > 0)     --tile; }
            else                     { if (tile < max_t) ++tile; }
        }
    }

    // ---- B/N: zoom out/in ----
    if (ks->state.B && !ks->prev_state.B) { main_camera.width -= 1.f; main_camera.height = (float)SCREEN_HEIGHT/(float)SCREEN_WIDTH * main_camera.width; CenterCamera(); }
    if (ks->state.N && !ks->prev_state.N) { main_camera.width += 1.f; main_camera.height = (float)SCREEN_HEIGHT/(float)SCREEN_WIDTH * main_camera.width; CenterCamera(); }

    // ---- Enter/Backspace: cycle levels in editor ----
    if (ks->state.ENTER && !ks->prev_state.ENTER) {
        if (++ed_level_idx >= NUM_LEVELS) ed_level_idx = 0;
        strncpy(ed_name, level_names[ed_level_idx], 255);
        EditorLoadLevel(ed_name);
        curr_level_index = ed_level_idx + 1;
    }
}
