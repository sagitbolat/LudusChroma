#define NO_SPLASH_SCREEN
#define INCLUDE_IMGUI
#include <direct.h>
#include "../Engine/SDL_sky.cpp"
#include "../Engine/skymath.h"
#include "../Engine/scene.h"

#include "../Engine/data_structs/dynamic_array.h"
DEFINE_LIST(int);

#define MAX_ENTITIES    256
#define MAX_NUM_PLAYERS 256

// Declared before game headers because tilemap.h's DrawTile references them as globals.
GL_ID*        shaders               = nullptr;
GPUBufferIDs  gpu_buffers            = {};
Transform     tile_default_transform = {};
SceneManager  scene_manager          = {};

#include "tilemap.h"
#include "entity.h"
#include "entity_render.h"
#include "level_loader.h"


// ============================================================
// SECTION: Window / world setup
// ============================================================

void Init(int* w, int* h, float* w_in_world_space, bool* fullscreen, fColor* clear_color) {
    *w                = 1280;
    *h                = 720;
    *w_in_world_space = 14.0f;
    *fullscreen       = false;
    *clear_color      = { 43.f/255.f, 43.f/255.f, 39.f/255.f, 1.f };
}


// ============================================================
// SECTION: Level list
// ============================================================

const int NUM_LEVELS = 2;
int curr_level_index = 0;

char level_names[][64] = {
    "0",   "1" , 
};

float level_zoom[] = {
    14.f, 14.f
};


// ============================================================
// SECTION: Global game state
// ============================================================

Tileset  tileset = {};
Tilemap  tilemap = {};

ComponentArrays comp_arrays;
EntityMap       entity_map  = {};
EmissionMap     emission_map = {};

int player_ids[MAX_NUM_PLAYERS] = {};
int num_players                 = 0;

LevelStateInfo level_info = {};

Sprite sprites[30];
Sprite emission_sprite;
Sprite wire_sprite;
Sprite tileset_sprite_asset;

Sprite WASD_sprite;
Sprite reload_sprite;
Sprite undo_sprite;
Sprite wire_view_sprite;
Sprite title_text;
Sprite title_text2;
Sprite act1_text;
Sprite act1_1_text;
Sprite act2_text;

static bool editor_mode    = false;
static char ed_name[256]   = "";
static int  ed_level_idx   = -1;

// ============================================================
// SECTION: Undo system
// ============================================================

const int UNDO_LENGTH = 2048;
struct UndoToken { int entity_id; int x; int y; };

UndoToken* undo_list      = nullptr;
int        undo_num_movers = 0; // number of movable entities — one UndoToken per mover per step
int        undo_ptr        = 0;

static void UndoSaveStep() {
    for (int i = 0; i < (int)comp_arrays.grid_mover_arr.dense.size(); ++i) {
        int eid = comp_arrays.grid_mover_arr.dense_ids[i];
        GridPosition* gp = comp_arrays.grid_position_arr.Get(eid);
        if (!gp) continue;
        undo_list[undo_ptr++] = { eid, gp->position.x, gp->position.y };
    }
}

static void UndoRevertStep() {
    if (undo_ptr < undo_num_movers) return;
    undo_ptr -= undo_num_movers;

    // First clear the old positions in entity_map
    for (int i = 0; i < undo_num_movers; ++i) {
        int eid = undo_list[undo_ptr + i].entity_id;
        GridPosition* gp = comp_arrays.grid_position_arr.Get(eid);
        if (!gp) continue;
        entity_map.SetID(gp->position.x, gp->position.y, (int)gp->layer, -1);
    }
    // Then restore positions
    for (int i = 0; i < undo_num_movers; ++i) {
        UndoToken& tok = undo_list[undo_ptr + i];
        GridPosition* gp = comp_arrays.grid_position_arr.Get(tok.entity_id);
        RenderTransform* rt = comp_arrays.render_transform_arr.Get(tok.entity_id);
        if (!gp) continue;
        gp->position      = { tok.x, tok.y };
        gp->prev_position = { tok.x, tok.y };
        if (rt) { rt->transform.position.x = (float)tok.x; rt->transform.position.y = (float)tok.y; }
        entity_map.SetID(tok.x, tok.y, (int)gp->layer, tok.entity_id);
        undo_list[undo_ptr + i] = {};
    }
}

static void UndoReallocate() {
    undo_num_movers = (int)comp_arrays.grid_mover_arr.dense.size();
    undo_list = undo_list
        ? (UndoToken*)realloc(undo_list, UNDO_LENGTH * undo_num_movers * sizeof(UndoToken))
        : (UndoToken*)calloc(UNDO_LENGTH * undo_num_movers, sizeof(UndoToken));
    undo_ptr = 0;
}


// ============================================================
// SECTION: Level helpers
// ============================================================

static void ResetEmissionMap() {
    emission_map.width  = tilemap.width;
    emission_map.height = tilemap.height;
    if (!emission_map.map)
        emission_map.map = (EmissionTile*)calloc(tilemap.width * tilemap.height, sizeof(EmissionTile));
    else {
        free(emission_map.map);
        emission_map.map = (EmissionTile*)calloc(tilemap.width * tilemap.height, sizeof(EmissionTile));
    }
}

static void CenterCamera() {
    main_camera.position.x  = float(tilemap.width  / 2);
    main_camera.position.y  = float(tilemap.height / 2);
    main_camera.look_target = { main_camera.position.x, main_camera.position.y, 0.f };
}

static void LoadLevel(int index) {
    level_info = ReadLevelState(level_names[index], &tilemap, &comp_arrays, &entity_map,
                                player_ids, &num_players);
    ResetEmissionMap();
    CenterCamera();
    UndoReallocate();
    main_camera.width  = level_zoom[index];
    main_camera.height = (float)SCREEN_HEIGHT / (float)SCREEN_WIDTH * main_camera.width;
}


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

    // Count only live entities (gaps exist when entities were deleted)
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
    // WriteEntityRecord returns early for deleted (no GridPosition) entities — writes nothing
    for (int e = 0; e < level_info.num_entities; ++e)
        WriteEntityRecord(e, &comp_arrays, f);
    fclose(f);
    printf("EditorSaveLevel: saved %s (%d entities)\n", filepath, num_live);
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
// SECTION: Editor update
// ============================================================

void EditorUpdate(KeyboardState* ks, double dt) {
    static int  ed_type   = 1;   // entity type to spawn (1=PushBlock ... 10=Player)
    static int  ed_sel_id = -1;  // activator held for wiring (E key)
    static int  ed_w = 15, ed_h = 9;
    // ed_name and ed_level_idx are file-scope so they can be seeded from GameUpdate on toggle

    // ---- ImGui panel (108px strip at top of screen) ----
    UI_Window_Options wo = { true, true, true, true, false, true };
    UI_WindowStart("Editor", { 1280, 108 }, { 0, 0 }, &wo);

    {
        char fps_buf[32]; sprintf(fps_buf, "FPS: %d | EDITOR", (int)DeltaTimeToFps(dt));
        DrawSimpleText(fps_buf, { 0.99f, 0.1f }, UI_Alignment::TOP_RIGHT);
    }

    // Entity type image buttons
    const float U = float(108) / float(1280); // one icon-width unit in normalized UI space
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

    // New level size display (Z/X = width, C/V = height)
    { char s[16]; sprintf(s, "W:%d", ed_w); DrawSimpleText(s, { U*9.3f, 0.25f }, UI_Alignment::TOP_LEFT); }
    { char s[16]; sprintf(s, "H:%d", ed_h); DrawSimpleText(s, { U*9.3f, 0.55f }, UI_Alignment::TOP_LEFT); }
    if (DrawSimpleButton("New Level", { U*9.1f, 0.75f }, { U*0.95f, 0.22f }, nullptr)) EditorNewLevel(ed_w, ed_h);

    // Level name textbox + save/load
    DrawSimpleTextbox("##lvl", ed_name, 255, { U*10.15f, 0.35f }, { 108, 0 });
    if (DrawSimpleButton("Save", { U*10.05f, 0.72f }, { U*0.75f, 0.23f }, nullptr)) EditorSaveLevel(ed_name);
    if (DrawSimpleButton("Load", { U*11.0f,  0.72f }, { U*0.75f, 0.23f }, nullptr)) EditorLoadLevel(ed_name);

    UI_WindowEnd();

    // ---- Size controls (hotkeys, run outside UI block) ----
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
                case 9:  ColorChangerInit(id, &comp_arrays, {mx,my}, {255,255,255,255}, ColorBlendMode::Blended, true); entity_map.SetID(mx,my,1,id); break;
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

    // ---- R: rotate emitter direction / toggle door orientation ----
    if (ks->state.R && !ks->prev_state.R) {
        Vector2 wp = GetMousePositionInWorldCoords();
        int mx = (int)(wp.x + 0.5f), my = (int)(wp.y + 0.5f);
        int id = entity_map.GetID(mx, my, 1);
        if (id >= 0) {
            LaserEmitter* le = comp_arrays.laser_emitter_arr.Get(id);
            if (le) le->dir = (Direction)(((int)le->dir + 1) % 4);
        } else {
            id = entity_map.GetID(mx, my, 0);
            if (id >= 0) {
                Door* door = comp_arrays.door_arr.Get(id);
                if (door) door->open_by_default = !door->open_by_default;
            }
        }
    }

    // ---- T + 0-7: set color of entity under cursor ----
    if (ks->state.T && !ks->prev_state.T) {
        static const Color ed_colors[8] = {
            {255,255,255,255},{0,0,0,255},{255,0,0,255},{0,255,0,255},
            {0,0,255,255},{255,255,0,255},{255,0,255,255},{0,255,255,255}
        };
        int ci = ks->state.NUM0?0 : ks->state.NUM1?1 : ks->state.NUM2?2 : ks->state.NUM3?3 :
                 ks->state.NUM4?4 : ks->state.NUM5?5 : ks->state.NUM6?6 : ks->state.NUM7?7 : 0;
        Color c = ed_colors[ci];
        Vector2 wp = GetMousePositionInWorldCoords();
        int mx = (int)(wp.x + 0.5f), my = (int)(wp.y + 0.5f);
        int id1 = entity_map.GetID(mx, my, 1);
        int id0 = entity_map.GetID(mx, my, 0);
        if (id1 >= 0) {
            if (auto* le = comp_arrays.laser_emitter_arr.Get(id1))         le->color          = c;
            if (auto* lr = comp_arrays.laser_receiver_arr.Get(id1))        lr->accepted_color  = c;
            if (auto* cc = comp_arrays.color_changer_arr.Get(id1))         cc->main_color      = c;
            if (auto* pc = comp_arrays.grid_player_controlled_arr.Get(id1)) pc->color           = c;
        }
        if (id0 >= 0) {
            if (auto* tp = comp_arrays.teleporter_arr.Get(id0)) tp->color = c;
        }
    }

    // ---- E (hold = pick activator, release on door = wire them) ----
    if (ks->state.E && !ks->prev_state.E) {
        Vector2 wp = GetMousePositionInWorldCoords();
        int mx = (int)(wp.x + 0.5f), my = (int)(wp.y + 0.5f);
        int id1 = entity_map.GetID(mx, my, 1);
        int id0 = entity_map.GetID(mx, my, 0);
        ed_sel_id = -1;
        if (id1 >= 0 && comp_arrays.laser_receiver_arr.Get(id1)) ed_sel_id = id1;
        else if (id0 >= 0 && comp_arrays.button_arr.Get(id0))    ed_sel_id = id0;
        else if (id0 >= 0 && comp_arrays.teleporter_arr.Get(id0)) ed_sel_id = id0;
    }
    if (!ks->state.E && ks->prev_state.E && ed_sel_id >= 0) {
        Vector2 wp = GetMousePositionInWorldCoords();
        int mx = (int)(wp.x + 0.5f), my = (int)(wp.y + 0.5f);
        int id0 = entity_map.GetID(mx, my, 0);
        if (id0 >= 0) {
            Door* door = comp_arrays.door_arr.Get(id0);
            Teleporter* tp = comp_arrays.teleporter_arr.Get(id0);
            if (door) {
                SignalChannel* sc = comp_arrays.signal_channel_arr.Get(id0);
                SignalChannel* ac = comp_arrays.signal_channel_arr.Get(ed_sel_id);
                if (sc && ac) {
                    int32_t chan = ac->channels[0]; // activator's channel value is its own id
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
            int& tile = tilemap.map[my * tilemap.width + mx];
            int  max_t = tileset.width_in_tiles * tileset.height_in_tiles - 1;
            if (ks->state.LEFTSHIFT) { if (tile > 0)     --tile; }
            else                     { if (tile < max_t) ++tile; }
        }
    }

    // ---- B/N: zoom out/in ----
    if (ks->state.B && !ks->prev_state.B) { main_camera.width -= 1.f; main_camera.height = (float)SCREEN_HEIGHT/(float)SCREEN_WIDTH * main_camera.width; CenterCamera(); }
    if (ks->state.N && !ks->prev_state.N) { main_camera.width += 1.f; main_camera.height = (float)SCREEN_HEIGHT/(float)SCREEN_WIDTH * main_camera.width; CenterCamera(); }

    // ---- TAB/Backspace: cycle levels in editor ----
    if (ks->state.ENTER && !ks->prev_state.ENTER) {
        if (++ed_level_idx >= NUM_LEVELS) ed_level_idx = 0;
        strncpy(ed_name, level_names[ed_level_idx], 255);
        EditorLoadLevel(ed_name);
        curr_level_index = ed_level_idx + 1;
    }
    if (ks->state.BACKSPACE && !ks->prev_state.BACKSPACE) {
        if (--ed_level_idx < 0) ed_level_idx = NUM_LEVELS - 1;
        strncpy(ed_name, level_names[ed_level_idx], 255);
        EditorLoadLevel(ed_name);
        curr_level_index = ed_level_idx + 1;
    }
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
        scene_manager.SwitchScene(2, gs, ks, dt); // switch to GAME_SCENE
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

static bool level_transitioning  = false;
static bool restarting_level     = false;
static bool showing_wires        = false;
static bool first_load           = true;
static bool played_act2_card     = false;

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
        scene_manager.SwitchScene(1, gs, ks, dt); // ACT2_SCENE
        return;
    }

    // ---- Editor mode (Tab toggles) ----
    if (ks->state.TAB && !ks->prev_state.TAB) {
        editor_mode = !editor_mode;
        if (editor_mode) {
            // Seed the editor name/index from the currently loaded level
            int idx = curr_level_index - 1;
            if (idx >= 0 && idx < NUM_LEVELS) {
                strncpy(ed_name, level_names[idx], 255);
                ed_name[255] = '\0';
                ed_level_idx = idx;
            }
        } else {
            // Re-derive player list and undo buffer from current ECS state
            num_players = 0;
            for (int i = 0; i < level_info.num_entities; ++i)
                if (comp_arrays.grid_player_controlled_arr.Get(i)) player_ids[num_players++] = i;
            UndoReallocate();
        }
    }
    // When TAB just toggled editor on, skip EditorUpdate this frame to avoid double-firing TAB
    if (editor_mode) {
        bool just_toggled = ks->state.TAB && !ks->prev_state.TAB;
        if (!just_toggled) EditorUpdate(ks, dt);
        goto render;
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
                // Screen is fully dark — load the new level
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
                    // Discard the undo tokens we just pushed since nothing moved
                    undo_ptr -= undo_num_movers;
                }
            };

            if (!gm->moving && (ks->state.W || ks->state.ARROWUP))    try_move({ 0,  1 }, Direction::Up);
            if (!gm->moving && (ks->state.S || ks->state.ARROWDOWN))   try_move({ 0, -1 }, Direction::Down);
            if (!gm->moving && (ks->state.A || ks->state.ARROWLEFT))   try_move({-1,  0 }, Direction::Left);
            if (!gm->moving && (ks->state.D || ks->state.ARROWRIGHT))  try_move({ 1,  0 }, Direction::Right);
        }

        // Undo
        if (ks->state.U && !ks->prev_state.U) {
            UndoRevertStep();
        }

        // Wire view toggle
        if (ks->state.F && !ks->prev_state.F) {
            showing_wires = !showing_wires;
        }

        // Restart level
        if (ks->state.R && !ks->prev_state.R) {
            showing_wires       = false;
            level_transitioning = true;
            restarting_level    = true;
            --curr_level_index;
        }

        // Endgoal check
        if (gp && gm && !gm->moving) {
            int floor_id = entity_map.GetID(gp->position.x, gp->position.y, (int)GridLayer::GroundLayer);
            if (floor_id >= 0 && comp_arrays.endgoal_arr.Get(floor_id)) {
                showing_wires       = false;
                level_transitioning = true;
                if (curr_level_index >= NUM_LEVELS) curr_level_index = 0; // loop back
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

    // ---- Tutorial hints ----
    if (!level_transitioning) {
        int lvl = curr_level_index - 1;
        Transform t{};
        t.position.x = float(tilemap.width / 2);
        t.position.y = main_camera.position.y + 2.5f;
        t.scale      = { 8.f, 2.f, 1.f };
        if      (lvl == 0) { DrawSprite(WASD_sprite,    t, main_camera); }
        else if (lvl == 1) { t.scale = { 4.f, 1.f, 1.f }; DrawSprite(reload_sprite, t, main_camera); }
        else if (lvl == 2) { t.scale = { 5.f, 1.f, 1.f }; DrawSprite(undo_sprite,   t, main_camera); }
        else if (lvl == 3) { t.scale = { 7.f, 1.f, 1.f }; DrawSprite(wire_view_sprite, t, main_camera); }
    }

render:
    // ---- Pass 1: Tilemap (high y first for painter's algorithm) ----
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

    // Helper: write entity's render-space z from its layer base and y position
    auto SetEntityZ = [&](int id, float base_z, int y) {
        RenderTransform* rt = comp_arrays.render_transform_arr.Get(id);
        if (rt) rt->transform.position.z = base_z - float(2 * y);
    };

    // Helper: draw wire connections between a door and its linked activators
    auto DrawWires = [&](int door_id) {
        if (!showing_wires) return;
        RenderTransform* rt    = comp_arrays.render_transform_arr.Get(door_id);
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
    };

    // ---- Pass 2: Ground layer bottom halves — all entities including closed doors ----
    for (int y = tilemap.height - 1; y >= 0; --y)
        for (int x = 0; x < tilemap.width; ++x) {
            int id = entity_map.GetID(x, y, (int)GridLayer::GroundLayer);
            if (id < 0) continue;
            Door* door = comp_arrays.door_arr.Get(id);
            SetEntityZ(id, 0.f + (door ? 0.25f : 0.f), y);
            EntityRender(id, &comp_arrays, shaders, sprites, false, level_transitioning);
        }

    // ---- Pass 3: Ground layer top halves — skip closed doors, draw wires ----
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

    // ---- Pass 4: Entity layer bottom halves — skip closed doors (ground layer drew them) ----
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

    // ---- Pass 6: Bottom wall overlay — front face of the lowest row ----
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

    // ---- Pass 7: Entity layer top halves — all entities including closed doors, draw wires ----
    for (int y = tilemap.height - 1; y >= 0; --y)
        for (int x = 0; x < tilemap.width; ++x) {
            int id = entity_map.GetID(x, y, (int)GridLayer::EntityLayer);
            if (id < 0) continue;
            Door* door = comp_arrays.door_arr.Get(id);
            SetEntityZ(id, 1.f + (door ? 0.25f : 0.f), y);
            EntityRender(id, &comp_arrays, shaders, sprites, true, level_transitioning);
            if (door) DrawWires(id);
        }

    // ---- Per-frame ECS updates (order matches v1) ----
    ClearFrameState(&comp_arrays);
    for (int i = 0; i < level_info.num_entities; ++i) EntityUpdateEmit(i, &comp_arrays, tilemap, entity_map, emission_map);
    for (int i = 0; i < level_info.num_entities; ++i) EntityUpdateMover(i, &comp_arrays, (float)dt);
    for (int i = 0; i < level_info.num_entities; ++i) {
        EntityUpdateReceiver(i, &comp_arrays);
        EntityUpdateButton(i, &comp_arrays, entity_map);
    }
    for (int i = 0; i < level_info.num_entities; ++i) EntityUpdateDoor(i, &comp_arrays, entity_map, level_info.num_entities);
}


// ============================================================
// SECTION: Engine callbacks
// ============================================================

#define TITLE_SCENE 0
#define ACT2_SCENE  1
#define GAME_SCENE  2

void Awake(GameMemory* gm) {
    // Set working directory to the folder containing the executable so all
    // relative asset paths (shaders, textures, levels) resolve correctly
    // regardless of where the process was launched from.
    {
        char* base = SDL_GetBasePath();
        if (base) { _chdir(base); SDL_free(base); }
    }

    shaders     = ShaderInit("shader.vs", "shader.fs");
    gpu_buffers = InitGPUBuffers();
    ShaderSetVector(shaders, "i_color_multiplier", Vector4{ 1.f, 1.f, 1.f, 1.f });

    scene_manager.InitManager(3);
    scene_manager.AwakeScene(TITLE_SCENE, SceneNoOpVoid, SceneNoOp, TitleUpdate, SceneNoOp, SceneNoOpVoid);
    scene_manager.AwakeScene(ACT2_SCENE,  SceneNoOpVoid, SceneNoOp, Act2Update,  SceneNoOp, SceneNoOpVoid);
    scene_manager.AwakeScene(GAME_SCENE,  SceneNoOpVoid, SceneNoOp, GameUpdate,  SceneNoOp, SceneNoOpVoid);

    // Load sprites
    tileset_sprite_asset              = LoadSprite("assets/tileset.png",                  shaders, gpu_buffers);
    tileset.atlas                     = tileset_sprite_asset;
    tileset.width_in_tiles            = 5;
    tileset.height_in_tiles           = 8;

    tile_default_transform.rotation = { 0.f, 0.f, 0.f };
    tile_default_transform.scale    = { 1.f, 1.f, 1.f };

    WASD_sprite       = LoadSprite("assets/WASD.png",             shaders, gpu_buffers);
    reload_sprite     = LoadSprite("assets/restart_control.png",  shaders, gpu_buffers);
    undo_sprite       = LoadSprite("assets/undo_control.png",     shaders, gpu_buffers);
    wire_view_sprite  = LoadSprite("assets/wire_view_control.png",shaders, gpu_buffers);
    title_text        = LoadSprite("assets/title.png",            shaders, gpu_buffers);
    title_text2       = LoadSprite("assets/title2.png",           shaders, gpu_buffers);
    act1_text         = LoadSprite("assets/act1.png",             shaders, gpu_buffers);
    act1_1_text       = LoadSprite("assets/act1_1.png",           shaders, gpu_buffers);
    act2_text         = LoadSprite("assets/act1.png",             shaders, gpu_buffers);

    emission_sprite   = LoadSprite("assets/emission.png",         shaders, gpu_buffers);
    wire_sprite       = LoadSprite("assets/wire.png",             shaders, gpu_buffers);

    Sprite tmp[30] = {
        LoadSprite("assets/player.png",              shaders, gpu_buffers), // 0
        LoadSprite("assets/player_up.png",           shaders, gpu_buffers), // 1
        LoadSprite("assets/player_down.png",         shaders, gpu_buffers), // 2
        LoadSprite("assets/player_left.png",         shaders, gpu_buffers), // 3
        LoadSprite("assets/player_right.png",        shaders, gpu_buffers), // 4
        LoadSprite("assets/push_block.png",          shaders, gpu_buffers), // 5
        LoadSprite("assets/static_block.png",        shaders, gpu_buffers), // 6
        LoadSprite("assets/emitter.png",             shaders, gpu_buffers), // 7
        LoadSprite("assets/emitter_nozzle.png",      shaders, gpu_buffers), // 8
        LoadSprite("assets/emitter_indicator.png",   shaders, gpu_buffers), // 9
        LoadSprite("assets/receiver.png",            shaders, gpu_buffers), // 10
        LoadSprite("assets/receiver_nozzle.png",     shaders, gpu_buffers), // 11
        LoadSprite("assets/receiver_indicator.png",  shaders, gpu_buffers), // 12
        LoadSprite("assets/door_open.png",           shaders, gpu_buffers), // 13
        LoadSprite("assets/door_closed.png",         shaders, gpu_buffers), // 14
        LoadSprite("assets/door_open_horiz.png",     shaders, gpu_buffers), // 15
        LoadSprite("assets/door_closed_horiz.png",   shaders, gpu_buffers), // 16
        LoadSprite("assets/endgoal.png",             shaders, gpu_buffers), // 17
        LoadSprite("assets/button_up.png",           shaders, gpu_buffers), // 18
        LoadSprite("assets/button_down.png",         shaders, gpu_buffers), // 19
        LoadSprite("assets/teleporter.png",          shaders, gpu_buffers), // 20
        LoadSprite("assets/color_changer.png",       shaders, gpu_buffers), // 21
        LoadSprite("assets/color_changer_frame.png", shaders, gpu_buffers), // 22
        LoadSprite("assets/color_changer_overlay.png",shaders, gpu_buffers),// 23
        LoadSprite("assets/color_puddle.png",        shaders, gpu_buffers), // 24
        LoadSprite("assets/player_suit.png",         shaders, gpu_buffers), // 25
        LoadSprite("assets/player_up_suit.png",      shaders, gpu_buffers), // 26
        LoadSprite("assets/player_down_suit.png",    shaders, gpu_buffers), // 27
        LoadSprite("assets/player_left_suit.png",    shaders, gpu_buffers), // 28
        LoadSprite("assets/player_right_suit.png",   shaders, gpu_buffers), // 29
    };
    for (int i = 0; i < 30; ++i) sprites[i] = tmp[i];

    // Initialise component arrays
    comp_arrays.Init(MAX_ENTITIES);

    // Load first level
    LoadLevel(curr_level_index);
    ++curr_level_index;
}

void Start(GameState* gs, KeyboardState* ks) {
    scene_manager.scenes[TITLE_SCENE].StartScene(gs, ks, 0.0);
}

void Update(GameState* gs, KeyboardState* ks, double dt) {
    scene_manager.SceneUpdate(gs, ks, dt);
}

void UserFree() {
    free(tilemap.map);       tilemap.map       = nullptr;
    free(entity_map.map);    entity_map.map    = nullptr;
    free(emission_map.map);  emission_map.map  = nullptr;
    free(undo_list);         undo_list         = nullptr;
    FreeGPUBuffers(gpu_buffers);
    FreeShaders(shaders);
}
