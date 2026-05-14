#define NO_SPLASH_SCREEN
#define INCLUDE_IMGUI
#include <direct.h>
#include "../Engine/SDL_sky.cpp"
#include "../Engine/skymath.h"
#include "../Engine/scene.h"
#include "../Engine/sprite_anim.h"

#include "../Engine/data_structs/dynamic_array.h"
DEFINE_LIST(int);

#define MAX_ENTITIES    256
#define MAX_NUM_PLAYERS 256

// Declared before game headers because tilemap.h's DrawTile references them as globals.
GL_ID*        shaders               = nullptr;
GPUBufferIDs  gpu_buffers            = {};
Transform     tile_default_transform = {};
SceneManager  scene_manager          = {};
SpriteSheet   door_open_sheet        = {};
SpriteSheet   door_close_sheet       = {};
SpriteSheet   button_down_sheet      = {};
SpriteSheet   button_up_sheet        = {};

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

const int NUM_LEVELS = 15;
int curr_level_index = 0;

char level_names[][64] = {
    "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12", "13", "14"
};

float level_zoom[] = {
    14.f, 14.f, 14.f, 14.f, 14.f, 14.f, 14.f, 14.f, 14.f, 14.f, 20.f, 14.f, 14.f, 14.f, 20.f
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

static bool editor_mode  = false;
static char ed_name[256] = "";
static int  ed_level_idx = -1;

static bool level_transitioning = false;
static bool restarting_level    = false;
static bool showing_wires       = false;
static bool first_load          = true;
static bool played_act2_card    = false;


// ============================================================
// SECTION: Undo system
// ============================================================

const int UNDO_LENGTH = 2048;
struct UndoToken { int entity_id; int x; int y; };

UndoToken* undo_list       = nullptr;
int        undo_num_movers = 0;
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

    for (int i = 0; i < undo_num_movers; ++i) {
        int eid = undo_list[undo_ptr + i].entity_id;
        GridPosition* gp = comp_arrays.grid_position_arr.Get(eid);
        if (!gp) continue;
        entity_map.SetID(gp->position.x, gp->position.y, (int)gp->layer, -1);
    }
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


#include "game_editor.h"
#include "game_render.h"
#include "game_scenes.h"


// ============================================================
// SECTION: Engine callbacks
// ============================================================

#define TITLE_SCENE 0
#define ACT2_SCENE  1
#define GAME_SCENE  2

void Awake(GameMemory* gm) {
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

    tileset_sprite_asset              = LoadSprite("assets/tileset.png",                   shaders, gpu_buffers);
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
        LoadSprite("assets/player.png",               shaders, gpu_buffers), // 0
        LoadSprite("assets/player_up.png",            shaders, gpu_buffers), // 1
        LoadSprite("assets/player_down.png",          shaders, gpu_buffers), // 2
        LoadSprite("assets/player_left.png",          shaders, gpu_buffers), // 3
        LoadSprite("assets/player_right.png",         shaders, gpu_buffers), // 4
        LoadSprite("assets/push_block.png",           shaders, gpu_buffers), // 5
        LoadSprite("assets/static_block.png",         shaders, gpu_buffers), // 6
        LoadSprite("assets/emitter.png",              shaders, gpu_buffers), // 7
        LoadSprite("assets/emitter_nozzle.png",       shaders, gpu_buffers), // 8
        LoadSprite("assets/emitter_indicator.png",    shaders, gpu_buffers), // 9
        LoadSprite("assets/receiver.png",             shaders, gpu_buffers), // 10
        LoadSprite("assets/receiver_nozzle.png",      shaders, gpu_buffers), // 11
        LoadSprite("assets/receiver_indicator.png",   shaders, gpu_buffers), // 12
        LoadSprite("assets/door_open.png",            shaders, gpu_buffers), // 13
        LoadSprite("assets/door_closed.png",          shaders, gpu_buffers), // 14
        LoadSprite("assets/door_open_horiz.png",      shaders, gpu_buffers), // 15
        LoadSprite("assets/door_closed_horiz.png",    shaders, gpu_buffers), // 16
        LoadSprite("assets/endgoal.png",              shaders, gpu_buffers), // 17
        LoadSprite("assets/button_up.png",            shaders, gpu_buffers), // 18
        LoadSprite("assets/button_down.png",          shaders, gpu_buffers), // 19
        LoadSprite("assets/teleporter.png",           shaders, gpu_buffers), // 20
        LoadSprite("assets/color_changer.png",        shaders, gpu_buffers), // 21
        LoadSprite("assets/color_changer_frame.png",  shaders, gpu_buffers), // 22
        LoadSprite("assets/color_changer_overlay.png",shaders, gpu_buffers), // 23
        LoadSprite("assets/color_puddle.png",         shaders, gpu_buffers), // 24
        LoadSprite("assets/player_suit.png",          shaders, gpu_buffers), // 25
        LoadSprite("assets/player_up_suit.png",       shaders, gpu_buffers), // 26
        LoadSprite("assets/player_down_suit.png",     shaders, gpu_buffers), // 27
        LoadSprite("assets/player_left_suit.png",     shaders, gpu_buffers), // 28
        LoadSprite("assets/player_right_suit.png",    shaders, gpu_buffers), // 29
    };
    for (int i = 0; i < 30; ++i) sprites[i] = tmp[i];

    door_open_sheet   = MakeSpriteSheet(LoadSprite("assets/door_open_anim.png",   shaders, gpu_buffers), 9, 1, 9);
    door_close_sheet  = MakeSpriteSheet(LoadSprite("assets/door_close_anim.png",  shaders, gpu_buffers), 9, 1, 9);
    button_down_sheet = MakeSpriteSheet(LoadSprite("assets/button_down_anim.png", shaders, gpu_buffers), 3, 1, 3);
    button_up_sheet   = MakeSpriteSheet(LoadSprite("assets/button_up_anim.png",   shaders, gpu_buffers), 3, 1, 3);

    comp_arrays.Init(MAX_ENTITIES);

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
