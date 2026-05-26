// ============================================================
// SECTION: Render helpers
// ============================================================
#include <GL/glew.h>

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


static bool SetEntityIrisMask(int id) {
    if (iris_timer <= 0.f) return false;
    ColorTag* ct = comp_arrays.color_tag_arr.Get(id);
    if (!ct) return false;
    if (!(ct->color == iris_masked_tag_color)) return false;
    float progress = iris_timer / IRIS_DURATION;
    float radius   = 1.f - progress;
    ShaderSetFloat(shaders, "iris_mask_enabled", 1.f);
    ShaderSetFloat(shaders, "iris_mask_radius",  radius);
    ShaderSetFloat(shaders, "iris_mask_aspect",  (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT);
    ShaderSetFloat(shaders, "iris_mask_invert",  0.f);
    return true;
}


static void DrawTextOnSurface(SDL_Surface* surf, const char* text, int px, int py,
                               int scale, uint8_t r, uint8_t g, uint8_t b) {
    static const uint8_t FONT[10][7] = {
        { 0x0E, 0x11, 0x13, 0x15, 0x19, 0x11, 0x0E },  // 0
        { 0x04, 0x0C, 0x04, 0x04, 0x04, 0x04, 0x0E },  // 1
        { 0x0E, 0x11, 0x01, 0x02, 0x04, 0x08, 0x1F },  // 2
        { 0x1F, 0x02, 0x04, 0x02, 0x01, 0x11, 0x0E },  // 3
        { 0x02, 0x06, 0x0A, 0x12, 0x1F, 0x02, 0x02 },  // 4
        { 0x1F, 0x10, 0x1E, 0x01, 0x01, 0x11, 0x0E },  // 5
        { 0x06, 0x08, 0x10, 0x1E, 0x11, 0x11, 0x0E },  // 6
        { 0x1F, 0x01, 0x02, 0x04, 0x08, 0x08, 0x08 },  // 7
        { 0x0E, 0x11, 0x11, 0x0E, 0x11, 0x11, 0x0E },  // 8
        { 0x0E, 0x11, 0x11, 0x0F, 0x01, 0x02, 0x0C },  // 9
    };
    int bpp = surf->format->BytesPerPixel;
    for (int ci = 0; text[ci]; ++ci) {
        char c = text[ci];
        if (c < '0' || c > '9') { px += (5 + 1) * scale; continue; }
        const uint8_t* glyph = FONT[c - '0'];
        for (int row = 0; row < 7; ++row)
            for (int col = 0; col < 5; ++col) {
                if (!(glyph[row] & (1 << (4 - col)))) continue;
                for (int sy = 0; sy < scale; ++sy)
                    for (int sx = 0; sx < scale; ++sx) {
                        int fx = px + col * scale + sx;
                        int fy = py + row * scale + sy;
                        if (fx < 0 || fy < 0 || fx >= surf->w || fy >= surf->h) continue;
                        uint8_t* p = (uint8_t*)surf->pixels + fy * surf->pitch + fx * bpp;
                        p[0] = r; p[1] = g; p[2] = b;
                    }
            }
        px += (5 + 1) * scale;
    }
}


// ============================================================
// SECTION: Game render
// ============================================================

static void GameRender() {
    // ---- Tutorial hints ----
    if (!level_transitioning && !exporting_screenshots) {
        int lvl = curr_level_index - 1;
        Transform t{};
        t.position.x = float(tilemap.width / 2);
        t.position.y = main_camera.position.y + 2.25f;
        t.scale      = { 8.f, 2.f, 1.f };
        if      (lvl == 0) { DrawSprite(WASD_sprite,    t, main_camera); }
        else if (lvl == 1) { t.scale = { 4.f, 1.f, 1.f }; DrawSprite(reload_sprite,      t, main_camera); }
        else if (lvl == 2) { t.scale = { 5.f, 1.f, 1.f }; t.position.y += 0.75f; DrawSprite(undo_sprite,        t, main_camera); }
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
            bool im = SetEntityIrisMask(id);
            EntityRender(id, &comp_arrays, shaders, sprites, false, level_transitioning);
            if (im) ShaderSetFloat(shaders, "iris_mask_enabled", 0.f);
        }

    // ---- Pass 3: Ground layer top halves — skip closed doors ----
    for (int y = tilemap.height - 1; y >= 0; --y)
        for (int x = 0; x < tilemap.width; ++x) {
            int id = entity_map.GetID(x, y, (int)GridLayer::GroundLayer);
            if (id < 0) continue;
            Door* door = comp_arrays.door_arr.Get(id);
            if (door && !door->is_open) continue;
            SetEntityZ(id, 0.f + (door ? 0.25f : 0.f), y);
            bool im = SetEntityIrisMask(id);
            EntityRender(id, &comp_arrays, shaders, sprites, true, level_transitioning);
            if (im) ShaderSetFloat(shaders, "iris_mask_enabled", 0.f);
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
            bool im = SetEntityIrisMask(id);
            EntityRender(id, &comp_arrays, shaders, sprites, false, level_transitioning);
            if (im) ShaderSetFloat(shaders, "iris_mask_enabled", 0.f);
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
            bool im = SetEntityIrisMask(id);
            EntityRender(id, &comp_arrays, shaders, sprites, true, level_transitioning);
            if (im) ShaderSetFloat(shaders, "iris_mask_enabled", 0.f);
            if (door) DrawWires(id);
        }

    // ---- Iris overlay: draw new background color inside the growing circle ----
    if (iris_timer > 0.f) {
        float progress = iris_timer / IRIS_DURATION;
        float radius   = 1.f - progress;
        float aspect   = (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT;
        ShaderUse(iris_shaders);
        ShaderSetVector(iris_shaders, "iris_color",   Vec4(iris_overlay_color));
        ShaderSetFloat (iris_shaders, "iris_radius",  radius);
        ShaderSetFloat (iris_shaders, "aspect_ratio", aspect);
        ShaderSetFloat (iris_shaders, "iris_invert",  0.f);
        Transform iris_t{};
        iris_t.scale = { 1.f, 1.f, 1.f };
        DrawSprite(iris_sprite, iris_t, main_camera);
        ShaderUse(shaders);
        ShaderSetVector(shaders, "i_color_multiplier", Vector4{ 1.f, 1.f, 1.f, 1.f });
        UVReset(shaders);

        // ---- Ghost pass: render entities being revealed, visible only inside the circle ----
        for (int i = 0; i < level_info.num_entities; ++i) {
            ColorTag* ct = comp_arrays.color_tag_arr.Get(i);
            if (!ct || !(ct->color == iris_revealing_color)) continue;
            GridPosition* gp = comp_arrays.grid_position_arr.Get(i);
            if (!gp) continue;
            if (entity_map.GetID(gp->position, (int)gp->layer) == i) continue;
            float base_z = (gp->layer == GridLayer::EntityLayer) ? 1.f : 0.f;
            ShaderSetFloat(shaders, "iris_mask_enabled", 1.f);
            ShaderSetFloat(shaders, "iris_mask_radius",  radius);
            ShaderSetFloat(shaders, "iris_mask_aspect",  aspect);
            ShaderSetFloat(shaders, "iris_mask_invert",  1.f);
            SetEntityZ(i, base_z, gp->position.y);
            EntityRender(i, &comp_arrays, shaders, sprites, false, level_transitioning, true);
            SetEntityZ(i, base_z, gp->position.y);
            EntityRender(i, &comp_arrays, shaders, sprites, true, level_transitioning, true);
            ShaderSetFloat(shaders, "iris_mask_enabled", 0.f);
        }
    }

    // ---- Screenshot export ----
    if (exporting_screenshots) {
        _mkdir("levels");
        _mkdir("levels/screenshots");
        int w = SCREEN_WIDTH, h = SCREEN_HEIGHT;
        unsigned char* pixels = (unsigned char*)malloc(w * h * 3);
        glReadPixels(0, 0, w, h, GL_RGB, GL_UNSIGNED_BYTE, pixels);
        SDL_Surface* surf = SDL_CreateRGBSurfaceWithFormat(0, w, h, 24, SDL_PIXELFORMAT_RGB24);
        if (surf) {
            SDL_LockSurface(surf);
            unsigned char* dst = (unsigned char*)surf->pixels;
            for (int row = 0; row < h; row++)
                memcpy(dst + row * surf->pitch, pixels + (h - 1 - row) * w * 3, w * 3);
            DrawTextOnSurface(surf, levels[export_idx].name, 17, 17, 16, 0,   0,   0  );
            DrawTextOnSurface(surf, levels[export_idx].name, 16, 16, 16, 255, 255, 255);
            SDL_UnlockSurface(surf);
            char path[256];
            sprintf(path, "levels/screenshots/%d.bmp", export_idx);
            SDL_SaveBMP(surf, path);
            SDL_FreeSurface(surf);
        }
        free(pixels);
        export_idx++;
        if (export_idx < NUM_LEVELS)
            LoadLevel(export_idx);
        else
            exporting_screenshots = false;
    }
}
