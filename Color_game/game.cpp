#define INCLUDE_IMGUI
#define DEBUG_MODE
#define NO_SPLASH_SCREEN
#include "../Engine/SDL_sky.cpp"
#include "../Engine/skymath.h"
#include "../Engine/scene.h"

#define MAX_ENTITIES 256 
#define MAX_NUM_PLAYERS 24
#include "tilemap.h"
#include "entity.h"
#include "level_loader.h"


#ifdef PROFILING
#include <chrono>
#endif


// SECTION: Initialization of stuff...
void Init(int *w, int *h, float *w_in_world_space, bool *fullscreen, fColor *clear_color)
{
    *w = 1280;
    *h = 720;
    *w_in_world_space = 14.0f;
    *fullscreen = false;
    //*clear_color = {0.8f/2, 0.83f/2, 1.0f/2, 1.0f};
    *clear_color = {43.0f/255.0f, 43.0f/255.0f, 39.0f/255.0f, 1.0f};
}


Tileset tileset = {0};
Tilemap tilemap = {0};


const int NUM_LEVELS = 35;
int curr_level_index = 0;
char level_names[][64] = { // NOTE: To calculate array length subtract the line number of the last string from the line number of this line
    "0",
    "1",
    "2",
    "3",   
    "4",      
    "5_w",
    "6_w",         
    "7_w",   
    "29",
    "10_w",         
    "11_w",          
    "12_w", 
    "13_w",
    "14_w",
    "15_w",          
    "16_w",
    "24_w", 
    "25_w", 
    "26_w",
    "17_w",          
    "18_w", 
    "20_w", 
    "27_w",
    "9_w",
    "19_w",
    "23_w", 
    "21_w", 
    "8_w",
    "28_w",
    "22_w2222",
    "32_22",
    "33_w",
    "31_wh",
    "34_l", // (or 34 instead for a tighter level.)
    "nd2"
};
float level_zoom[] = {
    14.0f, // 0
    14.0f,
    14.0f,
    14.0f,
    14.0f,
    14.0f, // 5
    14.0f,
    14.0f,
    14.0f,
    14.0f,
    14.0f, //10
    14.0f,
    14.0f,
    14.0f,
    14.0f,
    14.0f, //15
    14.0f,
    16.0f,
    14.0f,
    14.0f,
    14.0f, //20
    14.0f,
    16.0f,
    14.0f,
    14.0f,
    18.0f, //25
    14.0f,
    16.0f,
    14.0f,
    18.0f,
    18.0f, //30
    14.0f,
    18.0f,
    19.0f,
    25.0f
};



Sprite WASD_controls_sprite;
Sprite reload_controls_sprite;
Sprite undo_controls_sprite;
Sprite wire_view_controls_sprite;


Sprite player_sprite; 
Sprite player_up_sprite; 
Sprite player_down_sprite; 
Sprite player_left_sprite; 
Sprite player_right_sprite; 
Sprite push_block_sprite;        
Sprite static_block_sprite;      
Sprite emitter_sprite;     
Sprite emitter_nozzle_sprite;    
Sprite emitter_indicator_sprite; 
Sprite receiver_sprite;          
Sprite receiver_nozzle_sprite;   
Sprite receiver_indicator_sprite;
Sprite open_door_vertical_sprite;
Sprite closed_door_vertical_sprite;
Sprite open_door_horizontal_sprite;
Sprite closed_door_horizontal_sprite;
Sprite endgoal_sprite;
Sprite button_up_sprite;
Sprite button_down_sprite;
Sprite teleporter_sprite;
Sprite color_changer_sprite;
Sprite color_changer_frame_sprite;
Sprite color_changer_overlay_atlas;
Sprite color_puddle_sprite;
Sprite player_suit_sprite; 
Sprite player_up_suit_sprite; 
Sprite player_down_suit_sprite; 
Sprite player_left_suit_sprite; 
Sprite player_right_suit_sprite; 

Sprite sprites[30]; 

Sprite wire_sprite;

Sprite title_text;
Sprite title_text_line2;
Sprite act1_text;
Sprite act1_1_text;
Sprite act2_text;

EntityMap entity_id_map; // NOTE: acts as a lookup table from tilemap coordinate to an entity. a negative value indicates no entity at coordinate.
Entity* entities_array; // Array of entities in the game. Player is always first element.
EmissionMap emission_map;
Sprite emission_sprite;

GL_ID* shaders = nullptr;
GPUBufferIDs gpu_buffers = {nullptr};
Transform tile_default_transform;

LevelStateInfo level_state_info;

SceneManager scene_manager = {0};
#define NUM_SCENES 3
#define TITLE_SCENE 0
#define ACT2_SCENE 1
#define GAME_SCENE 2
void TitleAwake() {return;}
void Act2Awake() {return;}
void GameAwake() {return;}
void TitleStart(GameState *gs, KeyboardState *ks, double dt) {return;}
void Act2Start(GameState *gs, KeyboardState *ks, double dt) {return;}
void GameStart(GameState *gs, KeyboardState *ks, double dt) {return;} 
void TitleUpdate(GameState *gs, KeyboardState *ks, double dt);
void Act2Update(GameState *gs, KeyboardState *ks, double dt);
void GameUpdate(GameState *gs, KeyboardState *ks, double dt);
void TitleClose(GameState *gs, KeyboardState *ks, double dt) {return;}
void Act2Close(GameState *gs, KeyboardState *ks, double dt) {return;}
void GameClose(GameState *gs, KeyboardState *ks, double dt) {return;}
void TitleFree() {return;}
void Act2Free() {return;}
void GameFree() {return;}
// NOTE: Below is state of the level for the undo feature
const int UNDO_LENGTH = 2048;
struct UndoToken {
    int id;
    int x_pos; 
    int y_pos;
};
UndoToken* undo_list;
int undo_list_entity_num = 1;
int undo_list_pointer = 0;

void Awake(GameMemory *gm)
{
    // SECTION: renderer initializations
    shaders = ShaderInit("shader.vs", "shader.fs");
    gpu_buffers = InitGPUBuffers();
    ShaderSetVector(shaders, "i_color_multiplier", Vector4{1.0f, 1.0f, 1.0f, 1.0f});


    scene_manager.InitManager(NUM_SCENES);
    scene_manager.AwakeScene(TITLE_SCENE, TitleAwake, TitleStart, TitleUpdate, TitleClose, TitleFree);
    scene_manager.AwakeScene(ACT2_SCENE, Act2Awake, Act2Start, Act2Update, Act2Close, Act2Free);
    scene_manager.AwakeScene(GAME_SCENE, GameAwake, GameStart, GameUpdate, GameClose, GameFree);

    Sprite tileset_sprite   = LoadSprite("assets/tileset.png", shaders, gpu_buffers);
    tileset.atlas           = tileset_sprite;
    tileset.width_in_tiles  = 5;
    tileset.height_in_tiles = 8;



    tile_default_transform.position = {0.0f, 0.0f, -1.0f};
    tile_default_transform.rotation = {0.0f, 0.0f, 0.0f};
    tile_default_transform.scale    = {1.0f, 1.0f, 1.0f};


    WASD_controls_sprite            = LoadSprite("assets/WASD.png", shaders, gpu_buffers);
    reload_controls_sprite          = LoadSprite("assets/restart_control.png", shaders, gpu_buffers);
    undo_controls_sprite            = LoadSprite("assets/undo_control.png", shaders, gpu_buffers);
    wire_view_controls_sprite       = LoadSprite("assets/wire_view_control.png", shaders, gpu_buffers);

    player_sprite               = LoadSprite("assets/player.png", shaders, gpu_buffers);
    player_up_sprite            = LoadSprite("assets/player_up.png", shaders, gpu_buffers);
    player_down_sprite          = LoadSprite("assets/player_down.png", shaders, gpu_buffers);
    player_left_sprite          = LoadSprite("assets/player_left.png", shaders, gpu_buffers);
    player_right_sprite         = LoadSprite("assets/player_right.png", shaders, gpu_buffers);
    push_block_sprite           = LoadSprite("assets/push_block.png", shaders, gpu_buffers);
    static_block_sprite         = LoadSprite("assets/static_block.png", shaders, gpu_buffers);
    emitter_sprite              = LoadSprite("assets/emitter.png", shaders, gpu_buffers);
    emitter_nozzle_sprite       = LoadSprite("assets/emitter_nozzle.png", shaders, gpu_buffers);
    emitter_indicator_sprite    = LoadSprite("assets/emitter_indicator.png", shaders, gpu_buffers);
    receiver_sprite             = LoadSprite("assets/receiver.png", shaders, gpu_buffers);
    receiver_nozzle_sprite      = LoadSprite("assets/receiver_nozzle.png", shaders, gpu_buffers);
    receiver_indicator_sprite   = LoadSprite("assets/receiver_indicator.png", shaders, gpu_buffers);
    emission_sprite             = LoadSprite("assets/emission.png", shaders, gpu_buffers);
    open_door_vertical_sprite   = LoadSprite("assets/door_open.png", shaders, gpu_buffers);
    closed_door_vertical_sprite = LoadSprite("assets/door_closed.png", shaders, gpu_buffers);
    open_door_horizontal_sprite = LoadSprite("assets/door_open_horiz.png", shaders, gpu_buffers);
    closed_door_horizontal_sprite = LoadSprite("assets/door_closed_horiz.png", shaders, gpu_buffers);
    endgoal_sprite              = LoadSprite("assets/endgoal.png", shaders, gpu_buffers);
    button_up_sprite            = LoadSprite("assets/button_up.png", shaders, gpu_buffers);
    button_down_sprite          = LoadSprite("assets/button_down.png", shaders, gpu_buffers);
    teleporter_sprite           = LoadSprite("assets/teleporter.png", shaders, gpu_buffers);
    color_changer_sprite        = LoadSprite("assets/color_changer.png", shaders, gpu_buffers);
    color_changer_frame_sprite  = LoadSprite("assets/color_changer_frame.png", shaders, gpu_buffers);
    color_changer_overlay_atlas = LoadSprite("assets/color_changer_overlay.png", shaders, gpu_buffers);
    color_puddle_sprite         = LoadSprite("assets/color_puddle.png", shaders, gpu_buffers);
    player_suit_sprite          = LoadSprite("assets/player_suit.png", shaders, gpu_buffers);
    player_up_suit_sprite       = LoadSprite("assets/player_up_suit.png", shaders, gpu_buffers);
    player_down_suit_sprite     = LoadSprite("assets/player_down_suit.png", shaders, gpu_buffers);
    player_left_suit_sprite     = LoadSprite("assets/player_left_suit.png", shaders, gpu_buffers);
    player_right_suit_sprite    = LoadSprite("assets/player_right_suit.png", shaders, gpu_buffers);

    wire_sprite                     = LoadSprite("assets/wire.png", shaders, gpu_buffers);

    title_text                      = LoadSprite("assets/title.png", shaders, gpu_buffers);
    title_text_line2                = LoadSprite("assets/title2.png", shaders, gpu_buffers);
    act1_text                       = LoadSprite("assets/act1.png", shaders, gpu_buffers);
    act1_1_text                     = LoadSprite("assets/act1_1.png", shaders, gpu_buffers);
    act2_text                       = LoadSprite("assets/act1.png", shaders, gpu_buffers);

    {
        Sprite temp_sprites[30] = {
            player_sprite,                  //0 
            player_up_sprite,               //1
            player_down_sprite,             //2
            player_left_sprite,             //3
            player_right_sprite,            //4
            push_block_sprite,              //5
            static_block_sprite,            //6
            emitter_sprite,                 //7
            emitter_nozzle_sprite,          //8
            emitter_indicator_sprite,       //9
            receiver_sprite,                //10
            receiver_nozzle_sprite,         //11
            receiver_indicator_sprite,      //12
            open_door_vertical_sprite,      //13
            closed_door_vertical_sprite,    //14
            open_door_horizontal_sprite,    //15
            closed_door_horizontal_sprite,  //16
            endgoal_sprite,                 //17
            button_up_sprite,               //18
            button_down_sprite,             //19
            teleporter_sprite,              //20
            color_changer_sprite,           //21
            color_changer_frame_sprite,     //22
            color_changer_overlay_atlas,    //23
            color_puddle_sprite,            //24
            player_suit_sprite,             //25
            player_up_suit_sprite,          //26
            player_down_suit_sprite,        //27
            player_left_suit_sprite,        //28
            player_right_suit_sprite        //29
        };

        for (int i = 0; i < 30; ++i) {sprites[i] = temp_sprites[i];}
    }
    // TODO: Load the saved data

    level_state_info = ReadLevelState(level_names[curr_level_index], &tilemap, &entities_array, &entity_id_map, &undo_list_entity_num);
    

    ++curr_level_index;

    emission_map.width  = tilemap.width;
    emission_map.height = tilemap.height;
    emission_map.map    = (EmissionTile*)calloc(emission_map.width * emission_map.height, sizeof(EmissionTile));
    for (int i = 0; i < emission_map.width * emission_map.height; ++i) emission_map.map[i] = {0};

    // SECTION: allocate undo list history array
    undo_list = (UndoToken*)calloc(undo_list_entity_num * UNDO_LENGTH, sizeof(UndoToken));

    main_camera.position.x  = float(tilemap.width/2);
    main_camera.position.y  = float(tilemap.height/2);
    main_camera.look_target = {main_camera.position.x, main_camera.position.y, 0.0f}; 
}

void Start(GameState *gs, KeyboardState *ks) {
    scene_manager.scenes[0].StartScene(gs, ks, 0.0);
}

void Update (GameState *gs, KeyboardState *ks, double dt) {
    scene_manager.SceneUpdate(gs, ks, dt);
}

bool DisplayTextAnimation( // Returns true when the animation is playing (including both delays), false when the animation is done
    Sprite text_sprite, Transform t, // Sprite to display and the transform of the sprite
    float delay, // the delay in milliseconds before the text fades in
    float fade_in_duration, // how long the fadein is
    float linger_duration, // how long the text lingers at 100% opacity
    float fade_out_duration, // how long the text takes to fade out
    float post_delay,// how long the delay is between the text being fully faded out and the end of the animation
    float timer
) {

    float total_duration = delay + fade_in_duration + linger_duration + fade_out_duration + post_delay;
    if (timer < total_duration) {
        float complement = 0;
        if (timer < delay) {
            complement = 0.0f;
        } else if (timer < delay + fade_in_duration) { 
            complement = FloatClamp((timer - delay) / (fade_in_duration), 0.0f, 1.0f); 
        } else if (timer < total_duration - fade_out_duration - post_delay) {
            complement = 1.0f;
        } else if (timer < total_duration - post_delay) {
            complement =  FloatClamp(1.0f - (((timer - (total_duration - fade_out_duration - post_delay))) / (fade_out_duration)), 0.0f, 1.0f);
        } else if (timer < total_duration) {
            complement = 0.0f;
        }
            

        ShaderSetVector(shaders, "i_color_multiplier", Vec4(fColor{1.0f, 1.0f, 1.0f, complement}));
        DrawSprite(text_sprite, t, main_camera);
        ShaderSetVector(shaders, "i_color_multiplier", Vec4(fColor{1.0f, 1.0f, 1.0f, 1.0f}));
        return true;
    } else {
        return false;
    }
}

void TitleUpdate(GameState *gs, KeyboardState *ks, double dt) {
    static float timer = 0.0f;

    Transform t = {0};
    t.position.x = float(tilemap.width/2);
    t.position.y = main_camera.position.y+1.0f;
    t.scale.x = 2.25f;
    t.scale.y = 1.0f;
    t.scale.z = 1.0f;

    timer+=dt;

    DisplayTextAnimation(title_text, t, 1000.0f, 2000.0f, 4000.0f, 1000.0f, 1000.0f, timer);
    t.scale.x = 3.3472f * 2.0f;
    t.scale.y = 2.0f;
    t.position.y -= 2.0f;
    DisplayTextAnimation(title_text_line2, t, 3000.0f, 2000.0f, 2000.0f, 1000.0f, 1000.0f, timer);

    t.position.y += 1.0f;
    t.scale.x = 1.685f * 2.0f;
    t.scale.y = 2.0f;
    t.position.z += 1.0f;
    //if (!DisplayTextAnimation(act1_text, t, 10000.0f, 2000.0f, 2000.0f, 1000.0f, 1000.0f, timer)) {
    DisplayTextAnimation(act1_text, t, 10000.0f, 2000.0f, 2000.0f, 1000.0f, 1000.0f, timer);
    t.position.y -= 1.5f;
    t.scale.x = 15.0f * 0.5f;
    t.scale.y = 0.5f;
    t.position.z += 1.0f;
    if (!DisplayTextAnimation(act1_1_text, t, 12000.0f, 2000.0f, 3000.0f, 1000.0f, 1000.0f, timer)) {
        scene_manager.SwitchScene(GAME_SCENE, gs, ks, dt);
    }

    
    if (ks->state.SPACE) {
        scene_manager.SwitchScene(GAME_SCENE, gs, ks, dt);
    }

}

void Act2Update(GameState *gs, KeyboardState *ks, double dt) {
    static float timer = 0.0f;

    Transform t = {0};
    t.position.x = float(tilemap.width/2);
    t.position.y = main_camera.position.y+0.5f;
    t.scale.x = 1.685f * 2.0f;
    t.scale.y = 2.0f;
    t.scale.z = 1.0f;

    timer+=dt;

    DisplayTextAnimation(act2_text, t, 1000.0f, 2000.0f, 2000.0f, 1000.0f, 1000.0f, timer);
    
    t.position.y -= 1.5f;
    t.scale.x = 15.0f * 0.5f;
    t.scale.y = 0.5f;
    t.position.z += 1.0f;
    if (!DisplayTextAnimation(act1_1_text, t, 3000.0f, 2000.0f, 3000.0f, 1000.0f, 1000.0f, timer)) {
        scene_manager.SwitchScene(GAME_SCENE, gs, ks, dt);
    }

    
    if (ks->state.SPACE) {
        scene_manager.SwitchScene(GAME_SCENE, gs, ks, dt);
    }

}

bool first_load = true; // NOTE: Flag for triggering the transition animation during first loadin
bool level_transitioning = false;
bool fast_reload = false;
bool restarting_level = false; // NOTE: Only used to differentiating between transitions from one level to another and actual restarting.
void GameUpdate(GameState *gs, KeyboardState *ks, double dt) {

    if (first_load) {
        static float load_transition_time = 0.0f;
        static const float TRANSITION_DURATION = 1.0f * 1000.0f;
        static const float TRANSITION_ZOOM = 3.0f;

        load_transition_time += dt;

        if (load_transition_time < TRANSITION_DURATION) {
            float float_val = (load_transition_time / (TRANSITION_DURATION));
            main_camera.width = (level_zoom[curr_level_index-1] + TRANSITION_ZOOM) - (float_val * TRANSITION_ZOOM);
            main_camera.height = (float)SCREEN_HEIGHT/(float)SCREEN_WIDTH * main_camera.width;
            ShaderSetVector(shaders, "i_color_multiplier", Vec4(fColor{1.0f, 1.0f, 1.0f, float_val}));
        } else {
            first_load = false;
        }

         
    
    }
    
    static bool fading_in = true;

    static bool played_act_2_card = false;
    if (curr_level_index == 13 && !played_act_2_card) {
        played_act_2_card = true;

        first_load = true;
        scene_manager.SwitchScene(ACT2_SCENE, gs, ks, dt);
        return;
    }


    if (level_transitioning) {
    
        static const float TRANSITION_DURATION = 0.5f * 1000.0f;
        static const float TRANSITION_ZOOM = 3.0f;
        static float transition_time = 0.0f;
        
        transition_time += dt;

        if (fading_in) {
            if (transition_time < TRANSITION_DURATION) {
                main_camera.width = level_zoom[curr_level_index-1] + (transition_time / (TRANSITION_DURATION)) * TRANSITION_ZOOM;
                if (restarting_level) {
                    main_camera.width = level_zoom[curr_level_index] + (transition_time / (TRANSITION_DURATION)) * TRANSITION_ZOOM;
                }
                main_camera.height = (float)SCREEN_HEIGHT/(float)SCREEN_WIDTH * main_camera.width;
                float complement = 1.0f - (transition_time / (TRANSITION_DURATION/2));
                ShaderSetVector(shaders, "i_color_multiplier", Vec4(fColor{1.0f, 1.0f, 1.0f, complement}));
            }
            else {
                level_state_info = ReadLevelState(level_names[curr_level_index], &tilemap, &entities_array, &entity_id_map, &undo_list_entity_num);
                ++curr_level_index;
                restarting_level = false; 
                 
                // NOTE: Undo Feaute
                undo_list = (UndoToken*)realloc(undo_list, UNDO_LENGTH * sizeof(UndoToken) * undo_list_entity_num);
                undo_list_pointer = 0; 
                


                emission_map.width  = tilemap.width;
                emission_map.height = tilemap.height;
                free(emission_map.map);
                emission_map.map = (EmissionTile*)calloc(emission_map.width * emission_map.height, sizeof(EmissionTile));
                ShaderSetVector(shaders, "i_color_multiplier", Vec4(fColor{1.0f, 1.0f, 1.0f, 1.0f}));
                main_camera.width = level_zoom[curr_level_index-1];
                main_camera.height = (float)SCREEN_HEIGHT/(float)SCREEN_WIDTH * main_camera.width;
                main_camera.position.x  = float(tilemap.width/2);
                main_camera.position.y  = float(tilemap.height/2);
                main_camera.look_target = {main_camera.position.x, main_camera.position.y, 0.0f}; 
                transition_time = 0.0f;
                fading_in = false;
                return;
            }
        }
        else {
            if (transition_time < TRANSITION_DURATION) {
                float float_val = (transition_time / (TRANSITION_DURATION));
                main_camera.width = (level_zoom[curr_level_index-1] + TRANSITION_ZOOM) - (float_val * TRANSITION_ZOOM);
                main_camera.height = (float)SCREEN_HEIGHT/(float)SCREEN_WIDTH * main_camera.width;
                ShaderSetVector(shaders, "i_color_multiplier", Vec4(fColor{1.0f, 1.0f, 1.0f, float_val}));
            } else {
                fading_in = true;
                level_transitioning = false;
                transition_time = 0.0f;
            }
        }
    }



    const int BLOCK_PUSH_LIMIT = MAX_ENTITIES;

#ifdef PROFILING
    auto start = std::chrono::high_resolution_clock::now();
#endif

    if (!entities_array[0].movable.moving) {
        entities_array[0].player.direction = EntityComponentPlayer::DIRECTION_ENUM::NEUTRAL; 
    }
    
    if ((ks->state.W || ks->state.ARROWUP) && !entities_array[0].movable.moving) {
        // NOTE: Undo feature state
        for (int e = 0; e < level_state_info.num_entities; ++e) {
            if (!entities_array[e].active || !entities_array[e].movable.active) continue;
            int x_pos = entities_array[e].position.x;
            int y_pos = entities_array[e].position.y;
            undo_list[undo_list_pointer] = UndoToken({e, x_pos, y_pos});
            undo_list_pointer++;
        }
        
        bool moved = EntityMove(0, {0, 1}, tilemap, entity_id_map, entities_array, BLOCK_PUSH_LIMIT);
        entities_array[0].player.direction = EntityComponentPlayer::DIRECTION_ENUM::UP;
        printf("moved: %d\n", moved); 
        
        if (!moved) {
            undo_list_pointer -= undo_list_entity_num;
        }
    }
    if ((ks->state.S || ks->state.ARROWDOWN) && !entities_array[0].movable.moving) {
        // NOTE: Undo feature state
        for (int e = 0; e < level_state_info.num_entities; ++e) {
            if (!entities_array[e].active || !entities_array[e].movable.active) continue;
            int x_pos = entities_array[e].position.x;
            int y_pos = entities_array[e].position.y;
            undo_list[undo_list_pointer] = UndoToken({e, x_pos, y_pos});
            undo_list_pointer++;
        }
        
        bool moved = EntityMove(0, {0,-1}, tilemap, entity_id_map, entities_array, BLOCK_PUSH_LIMIT);
        entities_array[0].player.direction = EntityComponentPlayer::DIRECTION_ENUM::DOWN; 
        printf("moved: %d\n", moved);

        if (!moved) {
            undo_list_pointer -= undo_list_entity_num;
        }
    }
    if ((ks->state.A || ks->state.ARROWLEFT) && !entities_array[0].movable.moving) {
        // NOTE: Undo feature state
        for (int e = 0; e < level_state_info.num_entities; ++e) {
            if (!entities_array[e].active || !entities_array[e].movable.active) continue;
            int x_pos = entities_array[e].position.x;
            int y_pos = entities_array[e].position.y;
            undo_list[undo_list_pointer] = UndoToken({e, x_pos, y_pos});
            undo_list_pointer++;
        }
        
        bool moved = EntityMove(0, {-1,0}, tilemap, entity_id_map, entities_array, BLOCK_PUSH_LIMIT);
        entities_array[0].player.direction = EntityComponentPlayer::DIRECTION_ENUM::LEFT; 
        printf("moved: %d\n", moved); 
        
        if (!moved) {
            undo_list_pointer -= undo_list_entity_num;
        }
    }
    if ((ks->state.D || ks->state.ARROWRIGHT) && !entities_array[0].movable.moving) {
        // NOTE: Undo feature state
        for (int e = 0; e < level_state_info.num_entities; ++e) {
            if (!entities_array[e].active || !entities_array[e].movable.active) continue;
            int x_pos = entities_array[e].position.x;
            int y_pos = entities_array[e].position.y;
            undo_list[undo_list_pointer] = UndoToken({e, x_pos, y_pos});
            undo_list_pointer++;
        }
        
        bool moved = EntityMove(0, {1, 0}, tilemap, entity_id_map, entities_array, BLOCK_PUSH_LIMIT);
        entities_array[0].player.direction = EntityComponentPlayer::DIRECTION_ENUM::RIGHT; 
        printf("moved: %d\n", moved); 
        
        if (!moved) {
            undo_list_pointer -= undo_list_entity_num;
        }
    }
    
    
    static bool showing_wires = false;
    if (ks->state.F && !ks->prev_state.F) {
        showing_wires = !showing_wires;
    }


    { // NOTE: if player standing on the endgoal, begin transition 
        Vector2Int player_pos = entities_array[0].position;
        int entity_id_at_player_pos = entity_id_map.GetID(player_pos.x, player_pos.y, 0);
        if (entity_id_at_player_pos >= 0) {
            Entity entity = entities_array[entity_id_at_player_pos];
            if (entity.active && entity.endgoal.active && !entities_array[0].movable.moving && curr_level_index < NUM_LEVELS) {
                // NOTE: Load Next Level
                showing_wires = false;
                level_transitioning = true;
                
            } else if (entity.active && entity.endgoal.active && !entities_array[0].movable.moving && curr_level_index >= NUM_LEVELS) {

                // TODO: This is the end of the game. Trigger the ending and roll credits.
                showing_wires = false;
                level_transitioning = true;
                curr_level_index = 0;
            }
        }
    }


    // NOTE: undo last move
    if (ks->state.U && !ks->prev_state.U && !level_transitioning) {
        //memcpy(entities_array, prev_entities_array, level_state_info.num_entities * sizeof(entities_array[0]));
        //entity_id_map.map       = (int*)realloc(entity_id_map.map, sizeof(int) * entity_id_map.height * entity_id_map.width * entity_id_map.depth);
        //for (int i = 0; i < entity_id_map.height * entity_id_map.width * entity_id_map.depth; ++i) entity_id_map.map[i] = -1;
        //for (int i = 0; i < level_state_info.num_entities; ++i) {
        //    Entity entity = entities_array[i];
        //    entity_id_map.SetID(entity.position.x, entity.position.y, entity.entity_layer, entity.id);
        //}
        printf("Undoing: %d, %d\n", undo_list_pointer, undo_list_entity_num);
        if (undo_list_pointer - undo_list_entity_num >= 0) {
            undo_list_pointer -= undo_list_entity_num;
            for (int i = 0; i < undo_list_entity_num; ++i) {
                printf("UNDO %d\n", i);
                int e = undo_list[undo_list_pointer + i].id;
                
                int old_x = entities_array[e].position.x;
                int old_y = entities_array[e].position.y;
                entity_id_map.SetID(old_x, old_y, 1, -1);
            }

            for (int i = 0; i < undo_list_entity_num; ++i) {
                int e = undo_list[undo_list_pointer + i].id;
                int x = undo_list[undo_list_pointer + i].x_pos;
                int y = undo_list[undo_list_pointer + i].y_pos;
                printf("UNDO %d (%d, %d)\n", e, x, y);
                undo_list[undo_list_pointer + i] = UndoToken({0, 0, 0});
                (&entities_array[e])->position.x = x;
                (&entities_array[e])->position.y = y;
                (&entities_array[e])->prev_position.x = x;
                (&entities_array[e])->prev_position.y = y;
                (&entities_array[e])->transform.position.x = x;
                (&entities_array[e])->transform.position.y = y;
                entity_id_map.SetID(x, y, 1, e);
            }
        }
    }

    // NOTE: Restarting level
    if (ks->state.R && !ks->prev_state.R && !level_transitioning) {
        if (fast_reload) {
            printf("Reloading level %s (index %d)\n", level_names[curr_level_index-1], curr_level_index-1);
            level_state_info = ReadLevelState(level_names[curr_level_index-1], &tilemap, &entities_array, &entity_id_map, &undo_list_entity_num);
        } else {
            showing_wires = false;
            level_transitioning = true;
            curr_level_index--;
            restarting_level = true;
            printf("Reloading level %s (index %d)\n", level_names[curr_level_index], curr_level_index);
            return;
        }
    }

    // NOTE: Skipping through levels:
#ifdef DEBUG_MODE
    if (ks->state.Q && !ks->prev_state.Q) {
        --curr_level_index;
        if (curr_level_index <= 0) curr_level_index = NUM_LEVELS;
        level_state_info = ReadLevelState(level_names[curr_level_index-1], &tilemap, &entities_array, &entity_id_map, &undo_list_entity_num);
        printf("Restarting level: %s (index %d)\n", level_names[curr_level_index-1], curr_level_index-1);
        
        undo_list = (UndoToken*)realloc(undo_list, UNDO_LENGTH * sizeof(UndoToken) * undo_list_entity_num);

        emission_map.width  = tilemap.width;
        emission_map.height = tilemap.height;
        free(emission_map.map);
        emission_map.map = (EmissionTile*)calloc(emission_map.width * emission_map.height, sizeof(EmissionTile));
        
        main_camera.position.x  = float(tilemap.width/2);
        main_camera.position.y  = float(tilemap.height/2);
        main_camera.width = level_zoom[curr_level_index-1];
        main_camera.height = (float)SCREEN_HEIGHT/(float)SCREEN_WIDTH * main_camera.width;
        main_camera.look_target = {main_camera.position.x, main_camera.position.y, 0.0f}; 
    }
    if (ks->state.E && !ks->prev_state.E) {
        ++curr_level_index;
        if (curr_level_index > NUM_LEVELS) curr_level_index = 1;
        level_state_info = ReadLevelState(level_names[curr_level_index-1], &tilemap, &entities_array, &entity_id_map, &undo_list_entity_num);
        printf("Restarting level: %s (index %d)\n", level_names[curr_level_index-1], curr_level_index-1);

        undo_list = (UndoToken*)realloc(undo_list, UNDO_LENGTH * sizeof(UndoToken) * undo_list_entity_num);

        emission_map.width  = tilemap.width;
        emission_map.height = tilemap.height;
        free(emission_map.map);
        emission_map.map = (EmissionTile*)calloc(emission_map.width * emission_map.height, sizeof(EmissionTile));
        
        main_camera.position.x  = float(tilemap.width/2);
        main_camera.position.y  = float(tilemap.height/2);
        main_camera.width = level_zoom[curr_level_index-1];
        main_camera.height = (float)SCREEN_HEIGHT/(float)SCREEN_WIDTH * main_camera.width;
        main_camera.look_target = {main_camera.position.x, main_camera.position.y, 0.0f}; 
    }
    if (ks->state.SPACE && !ks->prev_state.SPACE) {
        printf("undo entities: %d\n", undo_list_entity_num);
        printf("undo list ptr: %d\n", undo_list_pointer);
        for (int i = 0; i < 10; ++i) {
            for (int u = 0; u < undo_list_entity_num; ++u) {
                int id = undo_list[i*undo_list_entity_num + u].id;
                int x = undo_list[i*undo_list_entity_num + u].x_pos;
                int y = undo_list[i*undo_list_entity_num + u].y_pos;
                printf("%d: (%d, %d) | ", id, x, y);
            }
            printf("\n");
        }
    }
#endif



#ifdef PROFILING
    auto move_end = std::chrono::high_resolution_clock::now();
#endif

    if (!level_transitioning) {
        if (curr_level_index-1 == 0) {
            Transform t = {0}; 
            t.position.x = float(tilemap.width/2);
            t.position.y = main_camera.position.y + 2.5f;
            t.scale.x = 8.0f;
            t.scale.y = 2.0f;
            t.scale.z = 1.0f;
            DrawSprite(WASD_controls_sprite, t, main_camera);
        } else if (curr_level_index-1 == 1) {
            Transform t = {0}; 
            t.position.x = float(tilemap.width/2);
            t.position.y = main_camera.position.y + 3.0f;
            t.scale.x = 4.0f;
            t.scale.y = 1.0f;
            t.scale.z = 1.0f;
            DrawSprite(reload_controls_sprite, t, main_camera);
        } else if (curr_level_index-1 == 2) {
            Transform t = {0}; 
            t.position.x = float(tilemap.width/2);
            t.position.y = main_camera.position.y + 3.0f;
            t.scale.x = 5.0f;
            t.scale.y = 1.0f;
            t.scale.z = 1.0f;
            DrawSprite(undo_controls_sprite, t, main_camera);
        } else if (curr_level_index-1 == 3) {
            Transform t = {0}; 
            t.position.x = float(tilemap.width/2);
            t.position.y = main_camera.position.y + 3.0f;
            t.scale.x = 7.0f;
            t.scale.y = 1.0f;
            t.scale.z = 1.0f;
            DrawSprite(wire_view_controls_sprite, t, main_camera);
        }
    }   



    { // NOTE: Tilemap Rendering Code
        float uv_width    = float(1)/float(tileset.width_in_tiles);
        float uv_height   = float(1)/float(tileset.height_in_tiles);
        ShaderSetVector(shaders, "bot_left_uv", Vector2{0.0f, 0.0f});
        ShaderSetVector(shaders, "top_right_uv", Vector2{uv_width, uv_height});
        for(int x = 0; x < tilemap.width; ++x) {
            for (int y = 0; y < tilemap.height; ++y) {

                int atlas_index = tilemap.map[y * tilemap.width + x];             
                
                if (!showing_wires && atlas_index > 14) {
                    atlas_index = 1;
                }

                if (atlas_index < 0) continue;

                int atlas_x = atlas_index % tileset.width_in_tiles;
                int atlas_y = atlas_index / tileset.width_in_tiles;

                float uv_x_offset = atlas_x * uv_width;
                float uv_y_offset = atlas_y * uv_height;
                ShaderSetVector(shaders, "uv_offset", Vector2{uv_x_offset, uv_y_offset});
                tile_default_transform.position = Vector3{float(x), float(y), float(-2 - (2*y))};
                DrawSprite(tileset.atlas, tile_default_transform, main_camera);
            }
        }
        
        ShaderSetVector(shaders, "bot_left_uv", Vector2{0.0f, 0.0f});
        ShaderSetVector(shaders, "top_right_uv", Vector2{1.0f, 1.0f});
        ShaderSetVector(shaders, "uv_offset", Vector2{0.0f, 0.0f});
    }

#ifdef PROFILING
    auto tile_render_end = std::chrono::high_resolution_clock::now();
#endif

    for (int z = 0; z < entity_id_map.depth; z++) {
        for (int y = tilemap.height-1; y >= 0; --y) {
            for(int x = 0; x < tilemap.width; ++x) {

                
                if (z == 1) {
                    EmissionRender(x, y, emission_map, emission_sprite, shaders, level_transitioning);
                }
                // SECTION: Entity Rendering
                int id = entity_id_map.GetID(x, y, z);
                if (id >= 0) {
                    Entity entity = entities_array[id];
                    float entity_layer = z;
                    if (entity.door.active) entity_layer += 0.25f;

                    if (entity.door.active && z == 0 && !entity.door.is_open) {
                        continue;
                    }
                    entities_array[id].transform.position.z = float((entity_layer) - (2*y));
                    EntityRender(id, entities_array, shaders, sprites, true, level_transitioning);
                    // SECTION: Wire rendering
                    if (entity.active && entity.door.active && showing_wires) {
                        for (int d = 0; d < MAX_CONNECTED_ACTIVATORS; ++d) {
                            int id_activator = entity.door.connected_activators_ids[d];
                            if (id_activator >= 0) {
                                // NOTE: Render a wire
                                Vector3 door_pos = entity.transform.position;
                                Vector3 activator_pos = entities_array[id_activator].transform.position;
                                Transform transform = {0.0f};
                                transform.position.x = (door_pos.x + activator_pos.x) / 2.0f;
                                transform.position.y = (door_pos.y + activator_pos.y) / 2.0f;
                                transform.position.z = 2.0f;

                                Vector2 direction;
                                direction.x = activator_pos.x - door_pos.x;
                                direction.y = activator_pos.y - door_pos.y;
                                float distance = Magnitude(direction);

                                transform.scale.x = 0.1f;
                                transform.scale.y = distance;
                                transform.scale.z = 1.0f;

                                // Use the Vector2LookAt function to calculate the rotation
                                Vector2 transform_position_2d = {transform.position.x, transform.position.y};
                                Vector2 target_position_2d = {activator_pos.x, activator_pos.y};
                                transform.rotation = Vector2LookAt(transform_position_2d, target_position_2d);

                                if (!level_transitioning) {
                                    Entity activator = entities_array[id_activator];
                                    
                                    if (activator.button.active && activator.button.is_pressed) {
                                        ShaderSetVector(shaders, "i_color_multiplier", Vector4{0.0f, 1.0f, 0.0f, 1.0f});
                                    } else if (activator.receiver.active && activator.receiver.signal_accepted) {
                                        ShaderSetVector(shaders, "i_color_multiplier", Vector4{0.0f, 1.0f, 0.0f, 1.0f});
                                    } else {
                                        ShaderSetVector(shaders, "i_color_multiplier", Vector4{1.0f, 0.0f, 0.0f, 1.0f});
                                    }
                                }
                                DrawSprite(wire_sprite, transform, main_camera);
                                if (!level_transitioning) ShaderSetVector(shaders, "i_color_multiplier", Vector4{1.0f, 1.0f, 1.0f, 1.0f});
                            }
                        }
                    }
                }                
                
            }
        
        }
    }

#ifdef PROFILING
    auto entity_render_end = std::chrono::high_resolution_clock::now();
#endif

    

#ifdef PROFILING
    auto emission_render_end = std::chrono::high_resolution_clock::now();
#endif
    
    
    // NOTE: This is done after rendering so that rendering that depends on the movable.moving flag doesnt flicker every block 
    //printf("############################\n");
    for (int id = 0; id < level_state_info.num_entities; ++id) {
        EntityUpdateEmit(id, tilemap, entity_id_map, emission_map, entities_array);
    }
    for (int id = 0; id < level_state_info.num_entities; ++id) {
        EntityUpdateMover(id, entities_array, dt);
    }
    for (int id = 0; id < level_state_info.num_entities; ++id) {
        EntityUpdateReceiver(id, entities_array);
        EntityUpdateButton(id, entities_array, entity_id_map);
    }
    for (int id = 0; id < level_state_info.num_entities; ++id) {
        EntityUpdateDoor(id, entities_array, entity_id_map);
    }
    
#ifdef PROFILING
    auto entity_update_end = std::chrono::high_resolution_clock::now();

    printf("%lld\n", std::chrono::duration_cast<std::chrono::microseconds>(tile_render_end - move_end).count());
    //if(std::chrono::duration_cast<std::chrono::microseconds>(entity_update_end - start).count() > 7000) {
    //printf("Player Move Took:           %lld microseconds\n", std::chrono::duration_cast<std::chrono::microseconds>(move_end - start).count());
    //printf("Tile Rendering Took:        %lld microseconds\n", std::chrono::duration_cast<std::chrono::microseconds>(tile_render_end - move_end).count());
    //printf("Entity Rendering Took:      %lld microseconds\n", std::chrono::duration_cast<std::chrono::microseconds>(entity_render_end - tile_render_end).count());
    //printf("Emission Rendering Took:    %lld microseconds\n", std::chrono::duration_cast<std::chrono::microseconds>(emission_render_end - entity_render_end).count());
    //printf("Enitty Updating Took:       %lld microseconds\n", std::chrono::duration_cast<std::chrono::microseconds>(entity_update_end - emission_render_end).count());
    //}
#endif
}

void UserFree()
{

    free(tilemap.map);
    tilemap.map = nullptr;

    free(entity_id_map.map);
    entity_id_map.map = nullptr;

    free(emission_map.map);
    emission_map.map = nullptr;


    FreeSprite(tileset.atlas);
    entities_array[0].active = false;


    free(undo_list);

    FreeGPUBuffers(gpu_buffers);
    FreeShaders(shaders);
}
