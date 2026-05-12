// ============================================================
// SECTION: Render helpers
// ============================================================

static void SetEntityZ(int id, float base_z, int y) {
    RenderTransform* rt = comp_arrays.render_transform_arr.Get(id);
    if (rt) rt->transform.position.z = base_z - float(2 * y);
}

static void DrawWires(int door_id) {
    if (!showing_wires) return;
    RenderTransform* rt      = comp_arrays.render_transform_arr.Get(door_id);
    SignalChannel*   door_ch = comp_arrays.signal_channel_arr.Get(door_id);
    if (!rt || !door_ch) return;
    for (int a = 0; a < level_info.num_entities; ++a) {
        SignalChannel* act_ch = comp_arrays.signal_channel_arr.Get(a);
        if (!act_ch || a == door_id) continue;
        if (!comp_arrays.laser_receiver_arr.Get(a) && !comp_arrays.button_arr.Get(a)) continue;
        bool connected = false;
        for (int d = 0; d < MAX_CONNECTIONS && !connected; ++d) {
            if (door_ch->channels[d] < 0) continue;
            for (int ac = 0; ac < MAX_CONNECTIONS && !connected; ++ac) {
                if (act_ch->channels[ac] < 0) continue;
                if (door_ch->channels[d] == act_ch->channels[ac]) connected = true;
            }
        }
        if (!connected) continue;
        RenderTransform* art = comp_arrays.render_transform_arr.Get(a);
        if (!art) continue;
        Vector3 door_pos = rt->transform.position;
        Vector3 act_pos  = art->transform.position;
        Transform wire_t{};
        wire_t.position.x = (door_pos.x + act_pos.x) / 2.f;
        wire_t.position.y = (door_pos.y + act_pos.y) / 2.f;
        wire_t.position.z = 2.f;
        Vector2 dir2      = { act_pos.x - door_pos.x, act_pos.y - door_pos.y };
        wire_t.scale      = { 0.1f, Magnitude(dir2), 1.f };
        wire_t.rotation   = Vector2LookAt({ wire_t.position.x, wire_t.position.y },
                                           { act_pos.x, act_pos.y });
        if (!level_transitioning) {
            LaserReceiver* lr  = comp_arrays.laser_receiver_arr.Get(a);
            Button*        btn = comp_arrays.button_arr.Get(a);
            bool active = (lr && lr->accepted) || (btn && btn->is_pressed);
            ShaderSetVector(shaders, "i_color_multiplier",
                active ? Vector4{ 0.f, 1.f, 0.f, 1.f } : Vector4{ 1.f, 0.f, 0.f, 1.f });
        }
        DrawSprite(wire_sprite, wire_t, main_camera);
        if (!level_transitioning)
            ShaderSetVector(shaders, "i_color_multiplier", Vector4{ 1.f, 1.f, 1.f, 1.f });
    }
}


// ============================================================
// SECTION: Game render
// ============================================================

static void GameRender() {
    // ---- Tutorial hints ----
    if (!level_transitioning) {
        int lvl = curr_level_index - 1;
        Transform t{};
        t.position.x = float(tilemap.width / 2);
        t.position.y = main_camera.position.y + 2.5f;
        t.scale      = { 8.f, 2.f, 1.f };
        if      (lvl == 0) { DrawSprite(WASD_sprite,    t, main_camera); }
        else if (lvl == 1) { t.scale = { 4.f, 1.f, 1.f }; DrawSprite(reload_sprite,      t, main_camera); }
        else if (lvl == 2) { t.scale = { 5.f, 1.f, 1.f }; DrawSprite(undo_sprite,        t, main_camera); }
        else if (lvl == 3) { t.scale = { 7.f, 1.f, 1.f }; DrawSprite(wire_view_sprite,   t, main_camera); }
    }

    // ---- Pass 1: Tilemap ----
    {
        float uv_w = 1.f / tileset.width_in_tiles;
        float uv_h = 1.f / tileset.height_in_tiles;
        ShaderSetVector(shaders, "bot_left_uv",  Vector2{ 0.f, 0.f });
        ShaderSetVector(shaders, "top_right_uv", Vector2{ uv_w, uv_h });

        for (int y = tilemap.height - 1; y >= 0; --y)
            for (int x = 0; x < tilemap.width; ++x) {
                int idx = tilemap.map[y * tilemap.width + x];
                if (!showing_wires && idx > 14) idx = 1;
                if (idx < 0) continue;
                int ax = idx % tileset.width_in_tiles;
                int ay = idx / tileset.width_in_tiles;
                ShaderSetVector(shaders, "uv_offset", Vector2{ ax * uv_w, ay * uv_h });
                tile_default_transform.position = Vector3{ float(x), float(y), float(-2 - 2*y) };
                DrawSprite(tileset.atlas, tile_default_transform, main_camera);
            }

        ShaderSetVector(shaders, "bot_left_uv",  Vector2{ 0.f, 0.f });
        ShaderSetVector(shaders, "top_right_uv", Vector2{ 1.f, 1.f });
        ShaderSetVector(shaders, "uv_offset",    Vector2{ 0.f, 0.f });
    }

    // ---- Pass 2: Ground layer bottom halves ----
    for (int y = tilemap.height - 1; y >= 0; --y)
        for (int x = 0; x < tilemap.width; ++x) {
            int id = entity_map.GetID(x, y, (int)GridLayer::GroundLayer);
            if (id < 0) continue;
            Door* door = comp_arrays.door_arr.Get(id);
            SetEntityZ(id, 0.f + (door ? 0.25f : 0.f), y);
            EntityRender(id, &comp_arrays, shaders, sprites, false, level_transitioning);
        }

    // ---- Pass 3: Ground layer top halves — skip closed doors ----
    for (int y = tilemap.height - 1; y >= 0; --y)
        for (int x = 0; x < tilemap.width; ++x) {
            int id = entity_map.GetID(x, y, (int)GridLayer::GroundLayer);
            if (id < 0) continue;
            Door* door = comp_arrays.door_arr.Get(id);
            if (door && !door->is_open) continue;
            SetEntityZ(id, 0.f + (door ? 0.25f : 0.f), y);
            EntityRender(id, &comp_arrays, shaders, sprites, true, level_transitioning);
            if (door) DrawWires(id);
        }

    // ---- Pass 4: Entity layer bottom halves — skip closed doors ----
    for (int y = tilemap.height - 1; y >= 0; --y)
        for (int x = 0; x < tilemap.width; ++x) {
            int id = entity_map.GetID(x, y, (int)GridLayer::EntityLayer);
            if (id < 0) continue;
            Door* door = comp_arrays.door_arr.Get(id);
            if (door && !door->is_open) continue;
            SetEntityZ(id, 1.f + (door ? 0.25f : 0.f), y);
            EntityRender(id, &comp_arrays, shaders, sprites, false, level_transitioning);
        }

    // ---- Pass 5: Emission map ----
    for (int y = tilemap.height - 1; y >= 0; --y)
        for (int x = 0; x < tilemap.width; ++x)
            EmissionRender(x, y, emission_map, emission_sprite, shaders, level_transitioning);

    // ---- Pass 6: Bottom wall overlay ----
    {
        float uv_w = 1.f / tileset.width_in_tiles;
        float uv_h = 1.f / tileset.height_in_tiles;
        ShaderSetVector(shaders, "bot_left_uv",  Vector2{ 0.f, 0.f });
        ShaderSetVector(shaders, "top_right_uv", Vector2{ uv_w, uv_h });
        auto DrawWallTile = [&](int x, int idx) {
            int ax = idx % tileset.width_in_tiles;
            int ay = idx / tileset.width_in_tiles;
            ShaderSetVector(shaders, "uv_offset", Vector2{ ax * uv_w, ay * uv_h });
            tile_default_transform.position = Vector3{ float(x), 0.5f, 2.0f };
            DrawSprite(tileset.atlas, tile_default_transform, main_camera);
        };
        for (int x = 0; x < tilemap.width; ++x) {
            if (x == 0)                    DrawWallTile(x, 15);
            else if (x == tilemap.width-1) DrawWallTile(x, 17);
            DrawWallTile(x, 16);
        }
        ShaderSetVector(shaders, "bot_left_uv",  Vector2{ 0.f, 0.f });
        ShaderSetVector(shaders, "top_right_uv", Vector2{ 1.f, 1.f });
        ShaderSetVector(shaders, "uv_offset",    Vector2{ 0.f, 0.f });
    }

    // ---- Pass 7: Entity layer top halves — all including closed doors ----
    for (int y = tilemap.height - 1; y >= 0; --y)
        for (int x = 0; x < tilemap.width; ++x) {
            int id = entity_map.GetID(x, y, (int)GridLayer::EntityLayer);
            if (id < 0) continue;
            Door* door = comp_arrays.door_arr.Get(id);
            SetEntityZ(id, 1.f + (door ? 0.25f : 0.f), y);
            EntityRender(id, &comp_arrays, shaders, sprites, true, level_transitioning);
            if (door) DrawWires(id);
        }
}
