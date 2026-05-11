#define INCLUDE_IMGUI
#define DEBUG_UI
#include "../Engine/SDL_sky.cpp"
#include "../Engine/skymath.h"

#define MAX_NUM_PLAYERS 256
#define MAX_ENTITIES 256 
#include "tilemap.h"
#include "entity.h"
#include "level_loader.h"

#include <chrono>
#include <thread>

const int NUM_LEVELS = 35;
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

// SECTION: Initialization of stuff...
void Init(int *w, int *h, float *w_in_world_space, bool *fullscreen, fColor *clear_color)
{
    *w = 1280;
    *h = 720;
    #ifdef DEBUG_UI
    *h = 720+108; 
    #endif
    *w_in_world_space = 14.0f;
    *fullscreen = false;
    //*clear_color = {43.0f/255, 43.0f/255, 39.0f/255, 1.0f};
    *clear_color = {0.0f, 0.0f, 0.0f, 1.0f};
}


Tileset tileset = {0};
Tilemap tilemap = {0};


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

int player;         // NOTE: entity id of player. Should be 0.
int push_block;     // NOTE: entity id of first push_block.
int static_block;   // NOTE: entity id of first static_block.
int emitter;        // NOTE: entity id of first emitter.
int emission;       // NOTE: entity id of first emission.

#define MAX_NUM_PLAYERS 256
int* player_ids;
int num_players = 0;

EntityMap entity_id_map; // NOTE: acts as a lookup table from tilemap coordinate to an entity. a negative value indicates no entity at coordinate.
Entity* entities_array; // Array of entities in the game. Player is always first element.
EmissionMap emission_map;
Sprite emission_sprite;

GL_ID* shaders = nullptr;
GPUBufferIDs gpu_buffers = {nullptr};
Transform tile_default_transform;



#ifdef DEBUG_UI
#define LEVEL_NAME_MAX 1024
char* level_name;
#endif

void Awake(GameMemory *gm)
{
    #ifdef DEBUG_UI
    level_name = (char*)malloc(sizeof(char) * LEVEL_NAME_MAX);
    #endif
    // SECTION: renderer initializations
    shaders = ShaderInit("shader.vs", "shader.fs");
    gpu_buffers = InitGPUBuffers();
    ShaderSetVector(shaders, "i_color_multiplier", Vector4{1.0f, 1.0f, 1.0f, 1.0f});

    player_ids = (int*)calloc(MAX_NUM_PLAYERS, sizeof(int));

    for (int i = 0; i < MAX_NUM_PLAYERS; ++i) {
        player_ids[i] = -1;
    }

    Sprite tileset_sprite   = LoadSprite("assets/tileset.png", shaders, gpu_buffers);
    tileset.atlas           = tileset_sprite;
    tileset.width_in_tiles  = 5;
    tileset.height_in_tiles = 8;

    tilemap.width   = 15;
    tilemap.height  = 9;
    int map[tilemap.width * tilemap.height] = {
        5,13,13,13,13,13,13,13,13,13,13,13,13,13, 6,
        9, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 7,
        9, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 7,
        9, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 7,
        9, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 7,
        9, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 7,
        9, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 7,
        9, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 7,
        10,3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,11
    };

    tilemap.map = (int*)calloc(tilemap.width * tilemap.height, sizeof(int));
    memcpy(tilemap.map, map, sizeof(int) * tilemap.height * tilemap.width);



    tile_default_transform.position = {0.0f, 0.0f, -1.0f};
    tile_default_transform.rotation = {0.0f, 0.0f, 0.0f};
    tile_default_transform.scale    = {1.0f, 1.0f, 1.0f};



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

    wire_sprite                 = LoadSprite("assets/wire.png", shaders, gpu_buffers);

    entity_id_map.width     = tilemap.width;
    entity_id_map.height    = tilemap.height;
    entity_id_map.depth     = 2;
    entity_id_map.map       = (int*)malloc(sizeof(int) * tilemap.height * tilemap.width * entity_id_map.depth);
    for (int i = 0; i < tilemap.height * tilemap.width * entity_id_map.depth; ++i) entity_id_map.map[i] = -1;

    entities_array = (Entity*)calloc(sizeof(Entity), MAX_ENTITIES);

    emission_map.width  = tilemap.width;
    emission_map.height = tilemap.height;
    emission_map.map    = (EmissionTile*)calloc(emission_map.width * emission_map.height, sizeof(EmissionTile));
    for (int i = 0; i < emission_map.width * emission_map.height; ++i) emission_map.map[i] = {0};





    main_camera.position.x  = float(tilemap.width/2);
    main_camera.position.y  = float(tilemap.height/2);
    main_camera.look_target = {7.0f, 4.0f, 0.0f}; 
    #ifdef DEBUG_UI
    main_camera.position.y  = float(tilemap.height/2) + 0.5f;
    main_camera.look_target = {7.0f, 4.5f, 0.0f}; 
    #endif
}

void Start(GameState *gs, KeyboardState *ks) {
    char level_name_default[] = "";
    strncpy(level_name, level_name_default, sizeof(level_name) - 1);
    level_name[sizeof(level_name) - 1] = '\0'; // Null-terminate the string
}



void RenderLayer(int z) {
    for (int y = tilemap.height-1; y >= 0; --y) {
        for(int x = 0; x < tilemap.width; ++x) {

            
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
                EntityRender(id, entities_array, shaders, sprites);
                // SECTION: Wire rendering
                if (entity.active && entity.door.active) {
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

                            DrawSprite(wire_sprite, transform, main_camera);
                        }
                    }
                }
            }                
            
        }
    
    }
}

// NOTE: For empty level creating
void Update(GameState *gs, KeyboardState *ks, double dt) {

    static double dt_coefficient = 1.0f;
    if (ks->state.MINUS && !ks->prev_state.MINUS) {
        dt_coefficient *= 0.5;
    }
    if (ks->state.EQUALS && !ks->prev_state.EQUALS) {
        dt_coefficient *= 2.0;
    }

    dt *= dt_coefficient;

#ifdef DEBUG_UI

    UI_Window_Options ui_wo = {true, true, true, true, false, true};


    UI_WindowStart("Entity Spawn Select", {1280, 108}, {0,0}, &ui_wo);

    double fps = DeltaTimeToFps(dt);
    char fps_str[64];
    sprintf(fps_str, "FPS: %d", int(fps));
    DrawSimpleText(fps_str, {0.99f, 0.2f}, UI_Alignment::TOP_RIGHT);

    static int entity_array_offset = 0;
    static int entity_type_to_spawn = 0;

    { // NOTE: Creating empty level.
        static Vector2Int tilemap_size = {15, 9};
        if (ks->state.Z && !ks->prev_state.Z) {
            if (tilemap_size.x > 3) tilemap_size.x--;
        } 
        if (ks->state.X && !ks->prev_state.X) {
            tilemap_size.x++;
        } 
        if (ks->state.C && !ks->prev_state.C) {
            if (tilemap_size.y > 3) tilemap_size.y--;
        } 
        if (ks->state.V && !ks->prev_state.V) {
            tilemap_size.y++;
        }
        char width_str[24];
        sprintf(width_str, "Width: %d", tilemap_size.x);
        char height_str[24];
        sprintf(height_str, "Height: %d", tilemap_size.y);
        DrawSimpleText(height_str, {float(108)/float(1280) * 9.2f, 0.3f}, UI_Alignment::TOP_LEFT);
        DrawSimpleText(width_str, {float(108)/float(1280) * 9.2f, 0.5f}, UI_Alignment::TOP_LEFT);
        if (DrawSimpleButton (
                "Emtpy Level",
                {float(108)/float(1280) * 9.1, 0.7},
                {float(108)/float(1280) * 0.8, 0.2},
                nullptr
            )
        ) {
            entity_array_offset = 0;
            tilemap.width = tilemap_size.x;
            tilemap.height = tilemap_size.y;

            if (tilemap.width > 14.0f || tilemap.height > 9) {
                main_camera.width   = 24.0f;
                main_camera.height  = (10.0f/16.0f) * 24.0f;
            }

            tilemap.map = (int*)realloc(tilemap.map, sizeof(int) * tilemap.height * tilemap.width);

            for (int y = 0; y < tilemap.height; ++y) {
                for (int x = 0; x < tilemap.width; ++x) {
                    if      (y == 0 && x == 0)                              tilemap.map[y * tilemap.width + x] = 5;
                    else if (y == 0 && x == tilemap.width-1)                tilemap.map[y * tilemap.width + x] = 6;
                    else if (x == 0 && y == tilemap.height-1)               tilemap.map[y * tilemap.width + x] = 10;
                    else if (x == tilemap.width-1 && y == tilemap.height-1) tilemap.map[y * tilemap.width + x] = 11;
                    
                    else if (y == 0) tilemap.map[y * tilemap.width + x] = 13;
                    else if (x == 0) tilemap.map[y * tilemap.width + x] = 9;
                    else if (y == tilemap.height-1) tilemap.map[y * tilemap.width + x] = 3;
                    else if (x == tilemap.width-1) tilemap.map[y * tilemap.width + x] = 7;
                    
                    else  tilemap.map[y * tilemap.width + x] = 1;
                }
            }

            entity_id_map.width = tilemap_size.x;
            entity_id_map.height = tilemap_size.y;
            entity_id_map.map = (int*)realloc(entity_id_map.map, sizeof(int) * entity_id_map.width * entity_id_map.height * entity_id_map.depth);

            for (int i = 0; i < entity_id_map.width * entity_id_map.height * entity_id_map.depth; ++i) {
                entity_id_map.map[i] = -1;
            }

            free(entities_array);
            entities_array = (Entity*)calloc(sizeof(Entity), MAX_ENTITIES);

             
            emission_map.width  = tilemap.width;
            emission_map.height = tilemap.height;
            free(emission_map.map);
            emission_map.map = (EmissionTile*)calloc(emission_map.width * emission_map.height, sizeof(EmissionTile));

        }
    }

    DrawSimpleTextbox(
        "Level Name", 
        level_name, LEVEL_NAME_MAX, 
        {float(108)/float(1280) * 10.05, 0.4}, 
        {108, 0}
    );

    if (DrawSimpleButton(
            "Save Level",
            {float(108)/float(1280) * 10.05, 0.7},
            {float(108)/float(1280) * 0.8, 0.2},
            nullptr
        )
    ) {
        SaveLevelState(level_name, &tilemap, 2, entities_array, entity_array_offset);
    }
    if (DrawSimpleButton(
            "Load Level",
            {float(108)/float(1280) * 11.0, 0.7},
            {float(108)/float(1280) * 0.8, 0.2},
            nullptr
        )
    ) {
        
        LevelStateInfo level_state_info = ReadLevelState(level_name, &tilemap, &entities_array, &entity_id_map, nullptr, player_ids, &num_players);
        entity_array_offset = level_state_info.num_entities;
        emission_map.width  = tilemap.width;
        emission_map.height = tilemap.height;
        free(emission_map.map);
        emission_map.map = (EmissionTile*)calloc(emission_map.width * emission_map.height, sizeof(EmissionTile));
    }




    if ( DrawSimpleImageButton (
            "PushBlockButton", 
            push_block_sprite, 
            {float(108)/float(1280) * 0.2, 0.2}, 
            {float(108)/float(1280) * 0.6, 0.6}
        )
    ) {
        entity_type_to_spawn = 1;
    } 
    if ( DrawSimpleImageButton (
            "StaticBlockButton", 
            static_block_sprite, 
            {float(108)/float(1280) * 1.0, 0.2}, 
            {float(108)/float(1280) * 0.6, 0.6}
        )
    ) {
        entity_type_to_spawn = 2;
    } 
    if ( DrawSimpleImageButton (
            "EmitterBlockButton", 
            emitter_sprite, 
            {float(108)/float(1280) * 1.8, 0.2}, 
            {float(108)/float(1280) * 0.6, 0.6}
        )
    ) {
        entity_type_to_spawn = 3;
    } 
    if ( DrawSimpleImageButton (
            "RecieverButton", 
            receiver_sprite, 
            {float(108)/float(1280) * 2.6, 0.2}, 
            {float(108)/float(1280) * 0.6, 0.6}
        )
    ) {
        entity_type_to_spawn = 4;
    } 
    if ( DrawSimpleImageButton (
            "DoorButton", 
            closed_door_vertical_sprite, 
            {float(108)/float(1280) * 3.4, 0.2}, 
            {float(108)/float(1280) * 0.6, 0.6}
        )
    ) {
        entity_type_to_spawn = 5;
    } 
    if ( DrawSimpleImageButton (
            "EndgoalButton", 
            endgoal_sprite, 
            {float(108)/float(1280) * 4.2, 0.2}, 
            {float(108)/float(1280) * 0.6, 0.6}
        )
    ) {
        entity_type_to_spawn = 6;
    } 
    if ( DrawSimpleImageButton (
            "ButtonButton", 
            button_up_sprite, 
            {float(108)/float(1280) * 5.0, 0.2}, 
            {float(108)/float(1280) * 0.6, 0.6}
        )
    ) {
        entity_type_to_spawn = 7;
    } 
    if ( DrawSimpleImageButton (
            "TeleporterButton", 
            teleporter_sprite, 
            {float(108)/float(1280) * 5.8, 0.2}, 
            {float(108)/float(1280) * 0.6, 0.6}
        )
    ) {
        entity_type_to_spawn = 8;
    }
    if ( DrawSimpleImageButton (
            "ColorChangerButton", 
            color_changer_sprite, 
            {float(108)/float(1280) * 6.6, 0.2}, 
            {float(108)/float(1280) * 0.6, 0.6}
        )
    ) {
        entity_type_to_spawn = 9;
    }  
    if ( DrawSimpleImageButton (
            "ColorPuddleButton", 
            color_puddle_sprite, 
            {float(108)/float(1280) * 7.4, 0.2}, 
            {float(108)/float(1280) * 0.6, 0.6}
        )
    ) {
        entity_type_to_spawn = 10;
    }  
    if ( DrawSimpleImageButton (
            "PlayerButton",
            player_sprite,
            {float(108)/float(1280) * 8.2, 0.2},
            {float(108)/float(1280) * 0.6, 0.6}
        )
    ) {
        if (num_players < MAX_NUM_PLAYERS) {
            entity_type_to_spawn = 11;
        }
    }
     
    
    if (ks->state.MBL && !ks->prev_state.MBL) {

        int tile_mouse_x = int(GetMousePositionInWorldCoords().x + 0.5f);
        int tile_mouse_y = int(GetMousePositionInWorldCoords().y + 0.5f); 

        switch (entity_type_to_spawn)
        {
            case 1: {
                if (entity_id_map.GetID(tile_mouse_x, tile_mouse_y, 0) >= 0) break;
                if (entity_id_map.GetID(tile_mouse_x, tile_mouse_y, 1) >= 0) break;
                if (TestTileCollide(tilemap, {tile_mouse_x, tile_mouse_y})) break;

                int e = entity_array_offset;
                entity_array_offset++;
                PushblockInit(&entities_array[e], e, {tile_mouse_x, tile_mouse_y});
                int x = entities_array[e].position.x;
                int y = entities_array[e].position.y;
                entity_id_map.SetID(x, y, entities_array[e].entity_layer, entities_array[e].id);
            } break;
            case 2: {
                if (entity_id_map.GetID(tile_mouse_x, tile_mouse_y, 0) >= 0) break;
                if (entity_id_map.GetID(tile_mouse_x, tile_mouse_y, 1) >= 0) break;
                if (TestTileCollide(tilemap, {tile_mouse_x, tile_mouse_y})) break;

                int e = entity_array_offset;
                entity_array_offset++;

                StaticBlockInit(&entities_array[e], e, {tile_mouse_x, tile_mouse_y});
                int x = entities_array[e].position.x;
                int y = entities_array[e].position.y;
                entity_id_map.SetID(x, y, entities_array[e].entity_layer, entities_array[e].id);
            } break;
            case 3: {
                if (entity_id_map.GetID(tile_mouse_x, tile_mouse_y, 0) >= 0) break;
                if (entity_id_map.GetID(tile_mouse_x, tile_mouse_y, 1) >= 0) break;
                if (TestTileCollide(tilemap, {tile_mouse_x, tile_mouse_y})) break;

                int e = entity_array_offset;
                entity_array_offset++;

                EmitterInit(&entities_array[e], e, {tile_mouse_x, tile_mouse_y}, {255, 255, 255, 255}, true, EntityComponentEmitter::DOWN); 
                int x = entities_array[e].position.x;
                int y = entities_array[e].position.y;
                entity_id_map.SetID(x, y, entities_array[e].entity_layer, entities_array[e].id);
            } break;
            case 4: {
                if (entity_id_map.GetID(tile_mouse_x, tile_mouse_y, 0) >= 0) break;
                if (entity_id_map.GetID(tile_mouse_x, tile_mouse_y, 1) >= 0) break;
                if (TestTileCollide(tilemap, {tile_mouse_x, tile_mouse_y})) break;

                int e = entity_array_offset;
                entity_array_offset++;

                ReceiverInit(&entities_array[e], e, {tile_mouse_x, tile_mouse_y}, {255, 255, 255, 255}, true);
                int x = entities_array[e].position.x;
                int y = entities_array[e].position.y;
                entity_id_map.SetID(x, y, entities_array[e].entity_layer, entities_array[e].id);
            } break;
            case 5: {
                if (entity_id_map.GetID(tile_mouse_x, tile_mouse_y, 0) >= 0) break;
                if (entity_id_map.GetID(tile_mouse_x, tile_mouse_y, 1) >= 0) break;
                if (TestTileCollide(tilemap, {tile_mouse_x, tile_mouse_y})) break;

                int e = entity_array_offset;
                entity_array_offset++;

                DoorInit(&entities_array[e], e, {tile_mouse_x, tile_mouse_y}); 
                int x = entities_array[e].position.x;
                int y = entities_array[e].position.y;
                entity_id_map.SetID(x, y, entities_array[e].entity_layer, entities_array[e].id);
            } break;
            case 6: {
                if (entity_id_map.GetID(tile_mouse_x, tile_mouse_y, 0) >= 0) break;
                if (entity_id_map.GetID(tile_mouse_x, tile_mouse_y, 1) >= 0) break;
                if (TestTileCollide(tilemap, {tile_mouse_x, tile_mouse_y})) break;

                int e = entity_array_offset;
                entity_array_offset++;

                EndgoalInit(&entities_array[e], e, {tile_mouse_x, tile_mouse_y}); 
                int x = entities_array[e].position.x;
                int y = entities_array[e].position.y;
                entity_id_map.SetID(x, y, entities_array[e].entity_layer, entities_array[e].id);
            } break;
            case 7: {
                if (entity_id_map.GetID(tile_mouse_x, tile_mouse_y, 0) >= 0) break;
                if (entity_id_map.GetID(tile_mouse_x, tile_mouse_y, 1) >= 0) break;
                if (TestTileCollide(tilemap, {tile_mouse_x, tile_mouse_y})) break;

                int e = entity_array_offset;
                entity_array_offset++;

                ButtonInit(&entities_array[e], e, {tile_mouse_x, tile_mouse_y}); 
                int x = entities_array[e].position.x;
                int y = entities_array[e].position.y;
                entity_id_map.SetID(x, y, entities_array[e].entity_layer, entities_array[e].id);
            } break;
            case 8: {
                if (entity_id_map.GetID(tile_mouse_x, tile_mouse_y, 0) >= 0) break;
                if (entity_id_map.GetID(tile_mouse_x, tile_mouse_y, 1) >= 0) break;
                if (TestTileCollide(tilemap, {tile_mouse_x, tile_mouse_y})) break;

                int e = entity_array_offset;
                entity_array_offset++;

                TeleporterInit(&entities_array[e], e, {255, 250, 230, 255}, -1, {tile_mouse_x, tile_mouse_y}); 
                int x = entities_array[e].position.x;
                int y = entities_array[e].position.y;
                entity_id_map.SetID(x, y, entities_array[e].entity_layer, entities_array[e].id);
            } break;
            case 9: {
                if (entity_id_map.GetID(tile_mouse_x, tile_mouse_y, 0) >= 0) break;
                if (entity_id_map.GetID(tile_mouse_x, tile_mouse_y, 1) >= 0) break;
                if (TestTileCollide(tilemap, {tile_mouse_x, tile_mouse_y})) break;

                int e = entity_array_offset;
                entity_array_offset++;

                ColorChangerInit(&entities_array[e], e, {255, 255, 255, 255}, EntityComponentColorChanger::COLOR_MODE::BLENDED, {tile_mouse_x, tile_mouse_y}); 
                int x = entities_array[e].position.x;
                int y = entities_array[e].position.y;
                entity_id_map.SetID(x, y, entities_array[e].entity_layer, entities_array[e].id);
            } break;
            case 10: {
                if (entity_id_map.GetID(tile_mouse_x, tile_mouse_y, 0) >= 0) break;
                if (entity_id_map.GetID(tile_mouse_x, tile_mouse_y, 1) >= 0) break;
                if (TestTileCollide(tilemap, {tile_mouse_x, tile_mouse_y})) break;

                int e = entity_array_offset;
                entity_array_offset++;

                ColorPuddleInit(&entities_array[e], e, {255, 255, 255, 255}, {tile_mouse_x, tile_mouse_y}); 
                int x = entities_array[e].position.x;
                int y = entities_array[e].position.y;
                entity_id_map.SetID(x, y, entities_array[e].entity_layer, entities_array[e].id);
            } break;
            case 11: {
                if (entity_id_map.GetID(tile_mouse_x, tile_mouse_y, 0) >= 0) break;
                if (entity_id_map.GetID(tile_mouse_x, tile_mouse_y, 1) >= 0) break;
                if (TestTileCollide(tilemap, {tile_mouse_x, tile_mouse_y})) break;

                int e = entity_array_offset;

                PlayerInit(&entities_array[e], e, {tile_mouse_x, tile_mouse_y});
                int x = entities_array[e].position.x;
                int y = entities_array[e].position.y;
                entity_id_map.SetID(x, y, entities_array[e].entity_layer, entities_array[e].id);
            
                player_ids[num_players] = e;
                num_players++;

                entity_array_offset++;
            } break;
            default:
                break;
        }
    }

    // NOTE: Do state changes for entity (entity-type specific)
    if (ks->state.R && !ks->prev_state.R) {
        int tile_mouse_x = int(GetMousePositionInWorldCoords().x + 0.5f);
        int tile_mouse_y = int(GetMousePositionInWorldCoords().y + 0.5f); 

        int entity_id = entity_id_map.GetID(tile_mouse_x, tile_mouse_y, 1);
        if (entity_id >= 0) {
            if (entities_array[entity_id].emitter.active) {
                entities_array[entity_id].emitter.direction = (EntityComponentEmitter::DIRECTION_ENUM)((entities_array[entity_id].emitter.direction + 1) % 4);
            } else if (entities_array[entity_id].receiver.active && entities_array[entity_id].receiver.signal_received) {
                entities_array[entity_id].receiver.accepted_color = entities_array[entity_id].receiver.signal_color;
            } else if (entities_array[entity_id].door.active) {
                entities_array[entity_id].door.open_by_default = !entities_array[entity_id].door.open_by_default;
            }
        } else {
            entity_id = entity_id_map.GetID(tile_mouse_x, tile_mouse_y, 0);
            if (entity_id >= 0) {
                if (entities_array[entity_id].door.active) {
                    entities_array[entity_id].door.open_by_default = !entities_array[entity_id].door.open_by_default;
                }
            }
        }

    }
    if (ks->state.S && ks->state.R && !ks->prev_state.R) {
        for (int i = 0; i < NUM_LEVELS; ++i) {
            
            LevelStateInfo level_state_info = ReadLevelState(level_names[i], &tilemap, &entities_array, &entity_id_map, nullptr, player_ids, &num_players);
            entity_array_offset = level_state_info.num_entities;
            emission_map.width  = tilemap.width;
            emission_map.height = tilemap.height;
            free(emission_map.map);
            emission_map.map = (EmissionTile*)calloc(emission_map.width * emission_map.height, sizeof(EmissionTile));
            SaveLevelState(level_names[i], &tilemap, 2, entities_array, entity_array_offset);
        }
    }
    Color emission_colors[] = {
        Color{255, 255, 255, 255},  // pure white
        Color{0, 0, 0, 255},        // black
        Color{255, 0, 0, 255},      // red
        Color{0, 255, 0, 255},   // green
        Color{0, 0, 255, 255},  // blue
        Color{255, 255, 0, 255},  // blended red-green
        Color{255, 0, 255, 255},  // blended red-blue
        Color{0, 255, 255, 255}  // blended green-blue
    };
    //Color emission_colors[] = {
    //    Color{255, 254, 230, 255},  // stylized white
    //    Color{43, 43, 29, 255},     // stylized black
    //    Color{255, 105, 98, 255},   // stylized red
    //    Color{95, 167, 119, 255},   // stylized green
    //    Color{133, 161, 242, 255},  // stylized blue
    //    Color{175, 136, 109, 255},  // blended red-green
    //    Color{194, 133, 170, 255},  // blended red-blue
    //    Color{114, 164, 181, 255}  // blended green-blue
    //};


    if (ks->state.T && !ks->prev_state.T) {
        int tile_mouse_x = int(GetMousePositionInWorldCoords().x + 0.5f);
        int tile_mouse_y = int(GetMousePositionInWorldCoords().y + 0.5f); 

        int entity_id = entity_id_map.GetID(tile_mouse_x, tile_mouse_y, 1);
        int entity_id_floor = entity_id_map.GetID(tile_mouse_x, tile_mouse_y, 0);
        int color_index = 0;

        if (ks->state.NUM0) {
            color_index = 0;
        } else if (ks->state.NUM1) {
            color_index = 1;
        } else if (ks->state.NUM2) {
            color_index = 2;
        } else if (ks->state.NUM3) {
            color_index = 3;
        } else if (ks->state.NUM4) {
            color_index = 4;
        } else if (ks->state.NUM5) {
            color_index = 5;
        } else if (ks->state.NUM6) {
            color_index = 6;
        } else if (ks->state.NUM7) {
            color_index = 7;
        } 
        

        if (entity_id >= 0) {
            if (entities_array[entity_id].emitter.active) {
                entities_array[entity_id].emitter.emission_color = emission_colors[color_index];
            } else if (entities_array[entity_id].receiver.active) {
                entities_array[entity_id].receiver.accepted_color = emission_colors[color_index];
            } else if (entities_array[entity_id].color_changer.active) {
                entities_array[entity_id].color_changer.color = emission_colors[color_index];
            } else if (entities_array[entity_id].player.active) {
                entities_array[entity_id].main_color = emission_colors[color_index];
            }
        }
        if (entity_id_floor >= 0) {
            if (entities_array[entity_id_floor].teleporter.active) {
                entities_array[entity_id_floor].teleporter.color = emission_colors[color_index];
            }
        }
    }


   
    if (ks->state.Q && !ks->prev_state.Q) {
        int tile_mouse_x = int(GetMousePositionInWorldCoords().x + 0.5f);
        int tile_mouse_y = int(GetMousePositionInWorldCoords().y + 0.5f);
        
        if (ks->state.LEFTSHIFT) {
            if (tilemap.map[(tile_mouse_y * tilemap.width) + tile_mouse_x] > 0) {
                tilemap.map[(tile_mouse_y * tilemap.width) + tile_mouse_x] -= 1;
            }
        }
        else { 
            if (tilemap.map[(tile_mouse_y * tilemap.width) + tile_mouse_x] < (tileset.height_in_tiles * tileset.width_in_tiles) - 1) {
                tilemap.map[(tile_mouse_y * tilemap.width) + tile_mouse_x] += 1;
            }
        }
    } 

    static int selected_receiver_id = -1; 
    // NOTE: When E is first pressed down, select the hovered receiver
    if (ks->state.E && !ks->prev_state.E) {


        int tile_mouse_x = int(GetMousePositionInWorldCoords().x + 0.5f);
        int tile_mouse_y = int(GetMousePositionInWorldCoords().y + 0.5f); 

        int entity_id = entity_id_map.GetID(tile_mouse_x, tile_mouse_y, 1);
        int bottom_entity_id = entity_id_map.GetID(tile_mouse_x, tile_mouse_y, 0);

        if (entity_id >= 0) {
            if (entities_array[entity_id].active && entities_array[entity_id].receiver.active) {
                selected_receiver_id = entity_id;
                printf("Selected activator id %d\n", entity_id);
            } 
        }
        if (bottom_entity_id >= 0) {
            if (entities_array[bottom_entity_id].active && entities_array[bottom_entity_id].button.active) {
                selected_receiver_id = bottom_entity_id;
                printf("Selected activator id %d\n", bottom_entity_id);
            } else if (entities_array[bottom_entity_id].active && entities_array[bottom_entity_id].teleporter.active) {
                selected_receiver_id = bottom_entity_id;
                printf("Selected activator id %d\n", bottom_entity_id);
            }
        }
    }
    // NOTE: When E is released, attach selected recevier the hovered door
    if (!ks->state.E && ks->prev_state.E) {

        int tile_mouse_x = int(GetMousePositionInWorldCoords().x + 0.5f);
        int tile_mouse_y = int(GetMousePositionInWorldCoords().y + 0.5f); 

        int entity_id = entity_id_map.GetID(tile_mouse_x, tile_mouse_y, 0);
        if (entity_id >= 0) { 
            if (entities_array[entity_id].active && (entities_array[entity_id].door.active || entities_array[entity_id].teleporter.active)) {
                if (selected_receiver_id >= 0) {
                    Entity* door = &entities_array[entity_id];
                    if (door->door.num_connected_activators < MAX_CONNECTED_ACTIVATORS && (entities_array[selected_receiver_id].button.active || entities_array[selected_receiver_id].receiver.active)) {
                        printf("Connected id %d to door %d\n", selected_receiver_id, entity_id);
                        door->door.connected_activators_ids[door->door.num_connected_activators] = selected_receiver_id;
                        door->door.num_connected_activators += 1;
                    } else if (door->teleporter.active && entities_array[selected_receiver_id].teleporter.active) {
                        door->teleporter.connected_teleporter_id = selected_receiver_id;
                        (&entities_array[selected_receiver_id])->teleporter.connected_teleporter_id = entity_id;
                    }
                }
            }
        }
        selected_receiver_id = -1;
    }

    if (ks->state.B && !ks->prev_state.B) {
        main_camera.width -= 1.0f; 
        main_camera.height = (float)SCREEN_HEIGHT/(float)SCREEN_WIDTH * main_camera.width;
        main_camera.position.x  = float(tilemap.width/2);
        main_camera.position.y  = float(tilemap.height/2);
        main_camera.look_target = {main_camera.position.x, main_camera.position.y, 0.0f}; 
    } 
    if (ks->state.N && !ks->prev_state.N) {
        main_camera.width += 1.0f; 
        main_camera.height = (float)SCREEN_HEIGHT/(float)SCREEN_WIDTH * main_camera.width;
        main_camera.position.x  = float(tilemap.width/2);
        main_camera.position.y  = float(tilemap.height/2);
        main_camera.look_target = {main_camera.position.x, main_camera.position.y, 0.0f}; 
    } 

    {
        static int level_load_index_curr = -1;

        if (ks->state.TAB && !ks->prev_state.TAB) {
            level_load_index_curr++;
            if (level_load_index_curr > NUM_LEVELS - 1) {
                level_load_index_curr = 0;
            }
            //prev level
            LevelStateInfo level_state_info = ReadLevelState(level_names[level_load_index_curr], &tilemap, &entities_array, &entity_id_map, nullptr, player_ids, &num_players);
            entity_array_offset = level_state_info.num_entities;
            emission_map.width  = tilemap.width;
            emission_map.height = tilemap.height;
            free(emission_map.map);
            emission_map.map = (EmissionTile*)calloc(emission_map.width * emission_map.height, sizeof(EmissionTile));
        }
        if (ks->state.BACKSPACE && !ks->prev_state.BACKSPACE) {
            // next level
            level_load_index_curr--;
            if (level_load_index_curr < 0) {
                level_load_index_curr = NUM_LEVELS-1;
            }
            LevelStateInfo level_state_info = ReadLevelState(level_names[level_load_index_curr], &tilemap, &entities_array, &entity_id_map, nullptr, player_ids, &num_players);
            entity_array_offset = level_state_info.num_entities;
            emission_map.width  = tilemap.width;
            emission_map.height = tilemap.height;
            free(emission_map.map);
            emission_map.map = (EmissionTile*)calloc(emission_map.width * emission_map.height, sizeof(EmissionTile));
        }
    }

    // NOTE: Do toggling movability:
    // TODO:

    // NOTE: Do saving:
    // TODO:

    UI_WindowEnd();
#endif

    const int BLOCK_PUSH_LIMIT = MAX_ENTITIES;

    

    {
        for (int i = 0; i < num_players; ++i) {
            int player_id = player_ids[i];

            if (player_id < 0) continue;


            Vector2Int dir = {0, 0};
            Entity* player_entity = &entities_array[player_id];
            if (!player_entity->movable.moving) {
                player_entity->player.direction = EntityComponentPlayer::DIRECTION_ENUM::NEUTRAL; 
            }
            if (ks->state.W && !player_entity->movable.moving) { 
                EntityComponentPlayer::DIRECTION_ENUM dir_enum = EntityComponentPlayer::DIRECTION_ENUM::UP;
                dir = {0, 1};
                if (player_entity->main_color.r < 127) {
                    dir.y = -1;
                    dir_enum = EntityComponentPlayer::DIRECTION_ENUM::DOWN;
                }
                if (player_entity->main_color.b < 127) {
                    dir.x = dir.y;
                    dir.y = 0;
                    if (dir_enum == EntityComponentPlayer::DIRECTION_ENUM::UP) {
                        dir_enum = EntityComponentPlayer::DIRECTION_ENUM::RIGHT;
                    } else if (dir_enum == EntityComponentPlayer::DIRECTION_ENUM::DOWN) {
                        dir_enum = EntityComponentPlayer::DIRECTION_ENUM::LEFT;
                    }

                }
                EntityMove(player_id, dir, tilemap, entity_id_map, entities_array, BLOCK_PUSH_LIMIT);
                entities_array[player_id].player.direction = dir_enum; 
            }
            if (ks->state.S && !player_entity->movable.moving) {
                EntityComponentPlayer::DIRECTION_ENUM dir_enum = EntityComponentPlayer::DIRECTION_ENUM::DOWN;
                dir = {0, -1};
                if (player_entity->main_color.r < 127) {
                    dir.y = 1;
                    dir_enum = EntityComponentPlayer::DIRECTION_ENUM::UP;
                }
                if (player_entity->main_color.b < 127) {
                    dir.x = dir.y;
                    dir.y = 0;
                    if (dir_enum == EntityComponentPlayer::DIRECTION_ENUM::UP) {
                        dir_enum = EntityComponentPlayer::DIRECTION_ENUM::RIGHT;
                    } else if (dir_enum == EntityComponentPlayer::DIRECTION_ENUM::DOWN) {
                        dir_enum = EntityComponentPlayer::DIRECTION_ENUM::LEFT;
                    }

                }
                EntityMove(player_id, dir, tilemap, entity_id_map, entities_array, BLOCK_PUSH_LIMIT);
                entities_array[player_id].player.direction = dir_enum; 
            }
            if (ks->state.A && !player_entity->movable.moving) {
                EntityComponentPlayer::DIRECTION_ENUM dir_enum = EntityComponentPlayer::DIRECTION_ENUM::LEFT;
                dir = {-1, 0};
                if (player_entity->main_color.g < 127) {
                    dir.x = 1;
                    dir_enum = EntityComponentPlayer::DIRECTION_ENUM::RIGHT;
                }
                if (player_entity->main_color.b < 127) {
                    dir.y = dir.x;
                    dir.x = 0;
                    if (dir_enum == EntityComponentPlayer::DIRECTION_ENUM::LEFT) {
                        dir_enum = EntityComponentPlayer::DIRECTION_ENUM::DOWN;
                    } else if (dir_enum == EntityComponentPlayer::DIRECTION_ENUM::RIGHT) {
                        dir_enum = EntityComponentPlayer::DIRECTION_ENUM::UP;
                    }

                }
                EntityMove(player_id, dir, tilemap, entity_id_map, entities_array, BLOCK_PUSH_LIMIT);
                entities_array[player_id].player.direction = dir_enum; 
            }
            if (ks->state.D && !player_entity->movable.moving) {
                EntityComponentPlayer::DIRECTION_ENUM dir_enum = EntityComponentPlayer::DIRECTION_ENUM::RIGHT;
                dir = {1, 0};
                if (player_entity->main_color.g < 127) {
                    dir.x = -1;
                    dir_enum = EntityComponentPlayer::DIRECTION_ENUM::LEFT;
                }
                if (player_entity->main_color.b < 127) {
                    dir.y = dir.x;
                    dir.x = 0;
                    if (dir_enum == EntityComponentPlayer::DIRECTION_ENUM::LEFT) {
                        dir_enum = EntityComponentPlayer::DIRECTION_ENUM::DOWN;
                    } else if (dir_enum == EntityComponentPlayer::DIRECTION_ENUM::RIGHT) {
                        dir_enum = EntityComponentPlayer::DIRECTION_ENUM::UP;
                    }

                }
                EntityMove(player_id, dir, tilemap, entity_id_map, entities_array, BLOCK_PUSH_LIMIT);
                entities_array[player_id].player.direction = dir_enum; 
            }
        }
    }

    if (ks->state.ENTER && !ks->prev_state.ENTER) {
        if (main_camera.projection == PERSPECTIVE_CAMERA) {
            main_camera.projection = ORTHOGRAPHIC_CAMERA;
            main_camera.position.x  = float(tilemap.width/2);
            main_camera.position.y  = float(tilemap.height/2);
            main_camera.position.z  = 3.0f;
            main_camera.look_target = {7.0f, 4.0f, 0.0f}; 
            #ifdef DEBUG_UI
            main_camera.position.y  = float(tilemap.height/2) + 0.5f;
            main_camera.look_target = {7.0f, 4.5f, 0.0f}; 
            #endif
        }
        else if (main_camera.projection == ORTHOGRAPHIC_CAMERA) {
            main_camera.projection = PERSPECTIVE_CAMERA;
        }
    }

    if (ks->state.ARROWLEFT && main_camera.projection == PERSPECTIVE_CAMERA) {
        main_camera.position.x -= 0.01f * dt;
    }
    if (ks->state.ARROWRIGHT && main_camera.projection == PERSPECTIVE_CAMERA) {
        main_camera.position.x += 0.01f * dt;
    }
    if (ks->state.ARROWUP && main_camera.projection == PERSPECTIVE_CAMERA) {
        main_camera.position.z -= 0.01f * dt;
    }
    if (ks->state.ARROWDOWN && main_camera.projection == PERSPECTIVE_CAMERA) {
        main_camera.position.z += 0.01f * dt;
    }

    for (int y = tilemap.height-1; y >= 0; --y) {
        for(int x = 0; x < tilemap.width; ++x) {
            DrawTile(tileset, {float(x), float(y), float(-2 - (2*y))}, tilemap.map[y*tilemap.width + x]);
        }
    }

    for (int y = tilemap.height-1; y >= 0; --y) {
        for(int x = 0; x < tilemap.width; ++x) {
            int id = entity_id_map.GetID(x, y, 0);
            if (id >= 0) {
                Entity entity = entities_array[id];
                float entity_layer = 0;
                if (entity.door.active) entity_layer += 0.25f;
                entities_array[id].transform.position.z = float((entity_layer) - (2*y));
                EntityRender(id, entities_array, shaders, sprites, false, false);
            }
        }
    }
    RenderLayer(0);

    
    for (int y = tilemap.height-1; y >= 0; --y) {
        for(int x = 0; x < tilemap.width; ++x) {
            int id = entity_id_map.GetID(x, y, 1);
            if (id >= 0) {
                Entity entity = entities_array[id];
                float entity_layer = 1;
                if (entity.door.active) entity_layer += 0.25f;

                if (entity.door.active && !entity.door.is_open) {
                    continue;
                }
                entities_array[id].transform.position.z = float((entity_layer) - (2*y));
                EntityRender(id, entities_array, shaders, sprites, false, false);
            }
        }
    }
    
    for (int y = tilemap.height-1; y >= 0; --y) {
        for(int x = 0; x < tilemap.width; ++x) {
            EmissionRender(x, y, emission_map, emission_sprite, shaders);
        }
    }
    // NOTE: Print debug info of entity
    if (ks->state.SPACE && !ks->prev_state.SPACE) {
        int tile_mouse_x = int(GetMousePositionInWorldCoords().x + 0.5f);
        int tile_mouse_y = int(GetMousePositionInWorldCoords().y + 0.5f); 

        int entity_id = entity_id_map.GetID(tile_mouse_x, tile_mouse_y, 0);
        if (entity_id >=0) {
            Entity entity = entities_array[entity_id];
            PrintEntity(entity);
        } 
        entity_id = entity_id_map.GetID(tile_mouse_x, tile_mouse_y, 1);
        if (entity_id >=0) {
            Entity entity = entities_array[entity_id];
            PrintEntity(entity);
        }
        
    }
    RenderLayer(1);

    // SECTION: Render the bottom wall that overlays the entities (for the 3D-esque look)
    for(int x = 0; x < tilemap.width; ++x) {
        if (x == 0) DrawTile(tileset, {float(x), 0.5, 2.0}, 15);
        else if (x == tilemap.width - 1) DrawTile(tileset, {float(x), 0.5, 2.0}, 17);
        DrawTile(tileset, {float(x), 0.5, 2.0}, 16);
    }


    // NOTE: Render entities through entity_id_map
    //for (int z = 0; z < entity_id_map.depth; z++) {
    //    for (int y = 0; y < entity_id_map.height; y++) {
    //        for (int x = 0; x < entity_id_map.width; x++) {
    //            int id = entity_id_map.GetID(x, y, z);
    //            if (id < 0) continue;
    //            EntityRender(id, entities_array, shaders);
    //        }
    //    }
    //}
    // NOTE: Render Entities through entities_array
    //for (int i = 0; i < entity_array_offset; ++i) {
    //    EntityRender(i, entities_array, shaders);
    //    Entity entity = entities_array[i];
    //    if (entity.active && entity.door.active) {
    //        for (int d = 0; d < MAX_CONNECTED_ACTIVATORS; ++d) {
    //            int id = entity.door.connected_activators_ids[d];
    //            if (id >= 0) {
    //                // NOTE: Render a wire
    //                Vector3 door_pos = entity.transform.position;
    //                Vector3 activator_pos = entities_array[id].transform.position;
    //                Transform transform = {0.0f};
    //                transform.position.x = (door_pos.x + activator_pos.x) / 2.0f;
    //                transform.position.y = (door_pos.y + activator_pos.y) / 2.0f;
    //                transform.position.z = 2.0f;

    //                Vector3 direction;
    //                direction.x = activator_pos.x - door_pos.x;
    //                direction.y = activator_pos.y - door_pos.y;
    //                direction.z = activator_pos.z - door_pos.z;
    //                float distance = Magnitude(direction);

    //                transform.scale.x = 0.1f;
    //                transform.scale.y = distance;
    //                transform.scale.z = 1.0f;

    //                // Use the Vector2LookAt function to calculate the rotation
    //                Vector2 transform_position_2d = {transform.position.x, transform.position.y};
    //                Vector2 target_position_2d = {activator_pos.x, activator_pos.y};
    //                transform.rotation = Vector2LookAt(transform_position_2d, target_position_2d);

    //                DrawSprite(wire_sprite, transform, main_camera);
    //            }
    //        }
    //    }
    //}



    // NOTE: If F is held down, render wiring connections:
    if (ks->state.F) {

    } 

    // NOTE: This is done after rendering so that rendering that depends on the movable.moving flag doesnt flicker every block 
    //printf("############################\n");
    for (int id = 0; id < entity_array_offset; ++id) {
        EntityUpdateEmit(id, tilemap, entity_id_map, emission_map, entities_array);
    }
    for (int id = 0; id < entity_array_offset; ++id) {
        EntityUpdateMover(id, entities_array, dt);
    }
    for (int id = 0; id < entity_array_offset; ++id) {
        EntityUpdateReceiver(id, entities_array);
        EntityUpdateButton(id, entities_array, entity_id_map);
    }
    for (int id = 0; id < entity_array_offset; ++id) {
        EntityUpdateDoor(id, entities_array, entity_id_map);
    }
    
    if (ks->state.U) {
        using namespace std::chrono;
        milliseconds oneSecond(1000);
    }

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
    entities_array[player].active = false;




    FreeGPUBuffers(gpu_buffers);
    FreeShaders(shaders);
}
