#pragma once
#include <SDL2/SDL.h>


// TODO: Remove if dont need delay on startup
#include <time.h>


// SECTION: Window Size
int SCREEN_WIDTH = 1280;
int SCREEN_HEIGHT = 720;

#include "skyengine.h"
#include "skyengine.cpp"
#include "allocation.cpp" 

#include "sky_renderer2D.h"
#include "sky_sound.h"


float SCREEN_WIDTH_IN_WORLD_SPACE = 10.0f;
bool LAUNCH_FULLSCREEN = 0;
fColor CLEAR_COLOR = {0.05, 0.0, 0.15, 1.0};
const char* startup_img = "startup.png";

SDL_bool ENGINE_MAIN_LOOP_RUNNING = SDL_TRUE;

Camera main_camera = {0};



// NOTE: Returns the x and y of the mouse cursor, in pixels. 0,0 is top left of the window
Vector2Int GetMousePosition() {
    Vector2Int pos = {0};
    SDL_GetMouseState(&pos.x, &pos.y);
    return pos;
}

Vector2 GetMousePositionInWorldCoords() {
    float h = main_camera.height;
    float w = main_camera.width;
    Vector2Int mouse_pos = GetMousePosition();

    Vector2 mouse_pos_normalized = {(float)mouse_pos.x / (float)SCREEN_WIDTH, (float)mouse_pos.y / (float)SCREEN_HEIGHT};
    mouse_pos_normalized.x -= 0.5f;
    mouse_pos_normalized.y -= 0.5f;
    mouse_pos_normalized.y *= -1.0f;
    return {mouse_pos_normalized.x * w + main_camera.position.x, mouse_pos_normalized.y * h + main_camera.position.y};
}

// NOTE: Pass true to enable the mouse cursor and false to disable the mouse cursor.
void SetCursorVisibility(bool toggle) {
    if (toggle) SDL_ShowCursor(SDL_ENABLE);
    else SDL_ShowCursor(SDL_DISABLE);
}


void QuitApp() {
    ENGINE_MAIN_LOOP_RUNNING = SDL_FALSE;
}


int main(int argc, char* argv[]) {


    const char* window_name = "SkyEngine App";

    // NOTE: The skyengine function that the user implements
    Init(&SCREEN_WIDTH, &SCREEN_HEIGHT, &SCREEN_WIDTH_IN_WORLD_SPACE, &LAUNCH_FULLSCREEN, &CLEAR_COLOR);
    

    // NOTE: Init OpenGL for graphics
    // TODO: pass LAUNCH_FULLSCREEN as an argument to the initializer
    WindowContext* window = InitWindowContext(SCREEN_WIDTH, SCREEN_HEIGHT, window_name, CLEAR_COLOR, LAUNCH_FULLSCREEN);
    SDL_GL_SetSwapInterval(1); 

    main_camera.position = {0.0f, 0.0f, 3.0f};
    main_camera.up_direction = {0.0f, 0.1f, 0.0f};
    main_camera.look_target = {0.0f, 0.0f, 0.0f};
    main_camera.near_plane = 0.1f;
    main_camera.far_plane = 1000.0f;
    main_camera.width = SCREEN_WIDTH_IN_WORLD_SPACE;
    main_camera.height = (float)SCREEN_HEIGHT/(float)SCREEN_WIDTH * main_camera.width;
    main_camera.FOV = 90.0f;
    main_camera.projection = ORTHOGRAPHIC_CAMERA;

    // DRAW STARTUP IMAGE
    // TODO: If you dont need a startup logo, remove this scope
#ifndef NO_SPLASH_SCREEN
    {
        ClearScreen();
        GL_ID* shaders = ShaderInit();
        GPUBufferIDs gpu_buffers = InitGPUBuffers();
        ShaderSetVector(shaders, "i_color_multiplier", Vector4{1.0f, 1.0f, 1.0f, 1.0f});
        Sprite starup_sprite = LoadSprite(startup_img, shaders, gpu_buffers);
        Transform startup_transform = {0};
        startup_transform.position = {0.0f, 0.0f, 0.0f};
        startup_transform.rotation = {0.0f, 0.0f, 0.0f};
        startup_transform.scale = {main_camera.width*0.5f, main_camera.height*0.5f, 1.0f};
        DrawSprite(starup_sprite, startup_transform, main_camera);
        SDL_GL_SwapWindow(window->window);
        

        // NOTE: Waits for 1 second, so the logo is visible even if load times are fast 
        clock_t end_time = clock() + 3.0f * CLOCKS_PER_SEC;
    
        while (clock() < end_time) {
            // Do nothing, just wait
        } 
    }
#endif
    // NOTE: Init OpenAL for sound
    AudioContext* audio_context = InitAudioContext();

    // NOTE: Init the UI
    InitUI(window);


    
    

    // NOTE: Initialize the freelist allocator
    size_t alignment = 64; // TODO: Change to reflect the machine architecture.
#ifdef ALLOCATOR_SIZE
    skymem::init((size_t)ALLOCATOR_SIZE, alignment);
#else
    skymem::init((size_t)Megabytes(128), alignment);
#endif
    
    // NOTE: Initializing game state memory
    GameMemory game_memory = {0};
    game_memory.is_initialized = true;
    game_memory.permanent_storage_size = Megabytes(128);
    game_memory.asset_storage_size = Gigabytes(1);
    game_memory.asset_storage = {0};
    
    InitArena(&game_memory.permanent_storage, game_memory.permanent_storage_size);
    InitArena(&game_memory.asset_storage, game_memory.asset_storage_size);

    // NOTE: Initializing keyboard state.
    KeyboardState keyboard_state = {0};
    keyboard_state.state = {0};

    
    

    SDL_Event e = {0};
    
    
    //Awake routine.
    Awake(&game_memory);


    // NOTE: Timer variables.
    uint64_t frequency = SDL_GetPerformanceFrequency();
    uint64_t previousCounter = SDL_GetPerformanceCounter();
    uint64_t currentCounter;
    uint64_t deltaCounter;


    while(ENGINE_MAIN_LOOP_RUNNING) {
        
        // SECTION: Time keeping
        // Calculate delta counter
        currentCounter = SDL_GetPerformanceCounter();
        deltaCounter = currentCounter - previousCounter;
        previousCounter = currentCounter;

        // Calculate delta time in milliseconds
        double delta_time = (double)deltaCounter / (double)frequency * 1000.0;



        // NOTE: Copying current state into previous state.
        // TODO: Should I use memcpy or assignment here? Answer: assignment is better.
        keyboard_state.prev_state = keyboard_state.state;
        //memcpy(&keyboard_state.prev_state, &keyboard_state.state, sizeof(KeyState));
        
        
        // NOTE: Use SDL2's keyboard state struct:
        const uint8_t* SDL_keyboard_state = SDL_GetKeyboardState(NULL);

        while (SDL_PollEvent(&e)) {

            SkyUI_ProcessEvent(&e);

            switch(e.type) {
                case SDL_QUIT:
                    ENGINE_MAIN_LOOP_RUNNING = SDL_FALSE;
                    break;
                case SDL_MOUSEBUTTONDOWN: {
                    if (e.button.button == SDL_BUTTON_LEFT) keyboard_state.state.MBL = 1;
                    if (e.button.button == SDL_BUTTON_RIGHT) keyboard_state.state.MBR = 1;
                    if (e.button.button == SDL_BUTTON_MIDDLE) keyboard_state.state.MBM = 1;
                } break;
                case SDL_MOUSEBUTTONUP: {
                    if (e.button.button == SDL_BUTTON_LEFT) keyboard_state.state.MBL = 0;
                    if (e.button.button == SDL_BUTTON_RIGHT) keyboard_state.state.MBR = 0;
                    if (e.button.button == SDL_BUTTON_MIDDLE) keyboard_state.state.MBM = 0;
                } break;
                case SDL_KEYDOWN: {
                    if (SDL_keyboard_state[SDL_SCANCODE_0]) keyboard_state.state.NUM0 = 1;    
                    if (SDL_keyboard_state[SDL_SCANCODE_1]) keyboard_state.state.NUM1 = 1;    
                    if (SDL_keyboard_state[SDL_SCANCODE_2]) keyboard_state.state.NUM2 = 1;    
                    if (SDL_keyboard_state[SDL_SCANCODE_3]) keyboard_state.state.NUM3 = 1;    
                    if (SDL_keyboard_state[SDL_SCANCODE_4]) keyboard_state.state.NUM4 = 1;    
                    if (SDL_keyboard_state[SDL_SCANCODE_5]) keyboard_state.state.NUM5 = 1;    
                    if (SDL_keyboard_state[SDL_SCANCODE_6]) keyboard_state.state.NUM6 = 1;    
                    if (SDL_keyboard_state[SDL_SCANCODE_7]) keyboard_state.state.NUM7 = 1;    
                    if (SDL_keyboard_state[SDL_SCANCODE_8]) keyboard_state.state.NUM8 = 1;    
                    if (SDL_keyboard_state[SDL_SCANCODE_9]) keyboard_state.state.NUM9 = 1;   
                    if (SDL_keyboard_state[SDL_SCANCODE_MINUS]) keyboard_state.state.MINUS = 1;
                    if (SDL_keyboard_state[SDL_SCANCODE_EQUALS]) keyboard_state.state.EQUALS = 1; 
                    if (SDL_keyboard_state[SDL_SCANCODE_BACKSPACE]) keyboard_state.state.BACKSPACE = 1;  
                    if (SDL_keyboard_state[SDL_SCANCODE_TAB]) keyboard_state.state.TAB = 1;  
                    if (SDL_keyboard_state[SDL_SCANCODE_Q]) keyboard_state.state.Q = 1;    
                    if (SDL_keyboard_state[SDL_SCANCODE_W]) keyboard_state.state.W = 1;    
                    if (SDL_keyboard_state[SDL_SCANCODE_E]) keyboard_state.state.E = 1;    
                    if (SDL_keyboard_state[SDL_SCANCODE_R]) keyboard_state.state.R = 1;  
                    if (SDL_keyboard_state[SDL_SCANCODE_T]) keyboard_state.state.T = 1;  
                    if (SDL_keyboard_state[SDL_SCANCODE_U]) keyboard_state.state.U = 1;  
                    if (SDL_keyboard_state[SDL_SCANCODE_I]) keyboard_state.state.I = 1;    
                    if (SDL_keyboard_state[SDL_SCANCODE_A]) keyboard_state.state.A = 1;    
                    if (SDL_keyboard_state[SDL_SCANCODE_S]) keyboard_state.state.S = 1;    
                    if (SDL_keyboard_state[SDL_SCANCODE_D]) keyboard_state.state.D = 1;    
                    if (SDL_keyboard_state[SDL_SCANCODE_F]) keyboard_state.state.F = 1;  
                    if (SDL_keyboard_state[SDL_SCANCODE_G]) keyboard_state.state.G = 1; 		  
                    if (SDL_keyboard_state[SDL_SCANCODE_J]) keyboard_state.state.J = 1; 		  
                    if (SDL_keyboard_state[SDL_SCANCODE_K]) keyboard_state.state.K = 1; 		  
                    if (SDL_keyboard_state[SDL_SCANCODE_L]) keyboard_state.state.L = 1; 		  
                    if (SDL_keyboard_state[SDL_SCANCODE_Z]) keyboard_state.state.Z = 1;  
                    if (SDL_keyboard_state[SDL_SCANCODE_X]) keyboard_state.state.X = 1;  
                    if (SDL_keyboard_state[SDL_SCANCODE_C]) keyboard_state.state.C = 1;  
                    if (SDL_keyboard_state[SDL_SCANCODE_V]) keyboard_state.state.V = 1;  
                    if (SDL_keyboard_state[SDL_SCANCODE_B]) keyboard_state.state.B = 1;  
                    if (SDL_keyboard_state[SDL_SCANCODE_N]) keyboard_state.state.N = 1;  
                    if (SDL_keyboard_state[SDL_SCANCODE_SPACE]) keyboard_state.state.SPACE = 1;    
                    if (SDL_keyboard_state[SDL_SCANCODE_RETURN]) keyboard_state.state.ENTER = 1;    
                    if (SDL_keyboard_state[SDL_SCANCODE_ESCAPE]) keyboard_state.state.ESC = 1;    
                    if (SDL_keyboard_state[SDL_SCANCODE_LSHIFT]) keyboard_state.state.LEFTSHIFT = 1;    
                    if (SDL_keyboard_state[SDL_SCANCODE_UP]) keyboard_state.state.ARROWUP = 1;    
                    if (SDL_keyboard_state[SDL_SCANCODE_DOWN]) keyboard_state.state.ARROWDOWN = 1;    
                    if (SDL_keyboard_state[SDL_SCANCODE_LEFT]) keyboard_state.state.ARROWLEFT = 1;    
                    if (SDL_keyboard_state[SDL_SCANCODE_RIGHT]) keyboard_state.state.ARROWRIGHT = 1;    
                } break;
                case SDL_KEYUP: {
                    if (!SDL_keyboard_state[SDL_SCANCODE_0]) keyboard_state.state.NUM0 = 0;    
                    if (!SDL_keyboard_state[SDL_SCANCODE_1]) keyboard_state.state.NUM1 = 0;    
                    if (!SDL_keyboard_state[SDL_SCANCODE_2]) keyboard_state.state.NUM2 = 0;    
                    if (!SDL_keyboard_state[SDL_SCANCODE_3]) keyboard_state.state.NUM3 = 0;    
                    if (!SDL_keyboard_state[SDL_SCANCODE_4]) keyboard_state.state.NUM4 = 0;    
                    if (!SDL_keyboard_state[SDL_SCANCODE_5]) keyboard_state.state.NUM5 = 0;    
                    if (!SDL_keyboard_state[SDL_SCANCODE_6]) keyboard_state.state.NUM6 = 0;    
                    if (!SDL_keyboard_state[SDL_SCANCODE_7]) keyboard_state.state.NUM7 = 0;    
                    if (!SDL_keyboard_state[SDL_SCANCODE_8]) keyboard_state.state.NUM8 = 0;    
                    if (!SDL_keyboard_state[SDL_SCANCODE_9]) keyboard_state.state.NUM9 = 0;    
                    if (!SDL_keyboard_state[SDL_SCANCODE_MINUS]) keyboard_state.state.MINUS = 0;
                    if (!SDL_keyboard_state[SDL_SCANCODE_EQUALS]) keyboard_state.state.EQUALS = 0; 
                    if (!SDL_keyboard_state[SDL_SCANCODE_BACKSPACE]) keyboard_state.state.BACKSPACE = 0;  
                    if (!SDL_keyboard_state[SDL_SCANCODE_TAB]) keyboard_state.state.TAB = 0;  
                    if (!SDL_keyboard_state[SDL_SCANCODE_Q]) keyboard_state.state.Q = 0;    
                    if (!SDL_keyboard_state[SDL_SCANCODE_W]) keyboard_state.state.W = 0;    
                    if (!SDL_keyboard_state[SDL_SCANCODE_E]) keyboard_state.state.E = 0;    
                    if (!SDL_keyboard_state[SDL_SCANCODE_R]) keyboard_state.state.R = 0;    
                    if (!SDL_keyboard_state[SDL_SCANCODE_T]) keyboard_state.state.T = 0;    
                    if (!SDL_keyboard_state[SDL_SCANCODE_U]) keyboard_state.state.U = 0;    
                    if (!SDL_keyboard_state[SDL_SCANCODE_I]) keyboard_state.state.I = 0;    
                    if (!SDL_keyboard_state[SDL_SCANCODE_A]) keyboard_state.state.A = 0;    
                    if (!SDL_keyboard_state[SDL_SCANCODE_S]) keyboard_state.state.S = 0;    
                    if (!SDL_keyboard_state[SDL_SCANCODE_D]) keyboard_state.state.D = 0;    
                    if (!SDL_keyboard_state[SDL_SCANCODE_F]) keyboard_state.state.F = 0;   
                    if (!SDL_keyboard_state[SDL_SCANCODE_G]) keyboard_state.state.G = 0;  
                    if (!SDL_keyboard_state[SDL_SCANCODE_J]) keyboard_state.state.J = 0;  
                    if (!SDL_keyboard_state[SDL_SCANCODE_K]) keyboard_state.state.K = 0;  
                    if (!SDL_keyboard_state[SDL_SCANCODE_L]) keyboard_state.state.L = 0;  
                    if (!SDL_keyboard_state[SDL_SCANCODE_Z]) keyboard_state.state.Z = 0;   
                    if (!SDL_keyboard_state[SDL_SCANCODE_X]) keyboard_state.state.X = 0;   
                    if (!SDL_keyboard_state[SDL_SCANCODE_C]) keyboard_state.state.C = 0;   
                    if (!SDL_keyboard_state[SDL_SCANCODE_V]) keyboard_state.state.V = 0;   
                    if (!SDL_keyboard_state[SDL_SCANCODE_B]) keyboard_state.state.B = 0;   
                    if (!SDL_keyboard_state[SDL_SCANCODE_N]) keyboard_state.state.N = 0;   
                    if (!SDL_keyboard_state[SDL_SCANCODE_SPACE]) keyboard_state.state.SPACE = 0;    
                    if (!SDL_keyboard_state[SDL_SCANCODE_RETURN]) keyboard_state.state.ENTER = 0;    
                    if (!SDL_keyboard_state[SDL_SCANCODE_ESCAPE]) keyboard_state.state.ESC = 0;    
                    if (!SDL_keyboard_state[SDL_SCANCODE_LSHIFT]) keyboard_state.state.LEFTSHIFT = 0;    
                    if (!SDL_keyboard_state[SDL_SCANCODE_UP]) keyboard_state.state.ARROWUP = 0;    
                    if (!SDL_keyboard_state[SDL_SCANCODE_DOWN]) keyboard_state.state.ARROWDOWN = 0;    
                    if (!SDL_keyboard_state[SDL_SCANCODE_LEFT]) keyboard_state.state.ARROWLEFT = 0;    
                    if (!SDL_keyboard_state[SDL_SCANCODE_RIGHT]) keyboard_state.state.ARROWRIGHT = 0;    
                } break;
                default:
                    break;
            }
        }
        
        

        // Clear the screen
        ClearScreen();
        
        GameUpdateAndRender(Vector2Int{SCREEN_WIDTH, SCREEN_HEIGHT}, &game_memory, &keyboard_state, delta_time); 
        
        // Swap buffers
        SDL_GL_SwapWindow(window->window);

    }
    

    UserFree();

#ifdef __linux__ 
    munmap(game_memory.permanent_storage.memory, game_memory.permanent_storage_size);
    munmap(game_memory.asset_storage.memory, game_memory.asset_storage_size);
#endif
#ifdef _WIN32
	free(game_memory.permanent_storage.memory);
    free(game_memory.asset_storage.memory);
#endif

    DeinitUI();
    FreeAudioContext(audio_context);
    FreeWindowContext(window);
    SDL_Quit();
    return 0;
}
