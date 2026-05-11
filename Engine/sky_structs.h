#pragma once
#include <stdint.h>
#include <stddef.h>
// NOTE: stddef.h can be removed, but it will cause compilation to fail if SDL2.h 
// is not included prior. This is beacause the stddef.h header is included in
// SDL2. This is included here so the file can be included without the main engine file (mostly for testing).

// SECTION: Utils
union Vector2 {
    float E[2];
    struct {
        float x, y;
    };
};
union Vector3 {
    float E[3];
    struct {
        float x, y, z;
    };
};
union Vector4 {
    float E[4];
    struct {
        float x, y, z, w;
    };
};
struct Vector2Int {
    int x, y;
};
struct Vector3Int {
    int x, y, z;
};
struct Vector4Int {
    int x, y, z, w;
};
union Matrix3 {
	//NOTE: Stored ROW MAJOR. (mat.E[ROW][COLUMN] OR mat.D[ROW * 4 + COLUMN).
	float E[3][3];
	float D[9];
    Vector3 rows[3];
    struct {
        Vector3 a;
        Vector3 b;
        Vector3 c;
    };
};

struct Rect {
    float x, y, width, height;
};
struct RectInt {
    int x, y;
    int width, height;
};
struct Circle {
    float x, y; //NOTE: Center of circle
    float radius;
};
struct CircleInt {
    int x, y;
    float radius;
};


struct Color {
    uint8_t r, g, b, a;
};
struct fColor {
    float r, g, b, a;
};




struct Transform {
    Vector3 position;
    Vector3 rotation;
    Vector3 scale;
};
enum CameraProjectionType {
    ORTHOGRAPHIC_CAMERA, 
    PERSPECTIVE_CAMERA
};
struct Camera {
    Vector3 position;
    Vector3 up_direction;
    Vector3 look_target;
    float near_plane;
    float far_plane;
    float width;
    float height;
    float FOV;

    CameraProjectionType projection;
};

// SECTION: UI
struct ButtonStyle {
    // SECTION: Coloring
    fColor button_idle_color;
    fColor button_hover_color;
    fColor button_active_color;
    fColor outline_color;
    fColor text_color;

    // SECTION: Styling
    float button_alpha;
    Vector2 text_alignment; // NOTE: {0.5, 0.5} will center the text
    float corner_rounding;
    float border_size;
    Vector2 frame_padding; // How much padding around the button there is. Useful for imagebutton for the image to take up all the space
};


#ifndef STB_IMAGE_IMPLEMENTATION
// SECTION: Image Loading 
// NOTE: Not needed if using stb_image
struct ImageData {
    uint16_t width;
    uint16_t height;
    uint8_t bytes_per_pixel;
    uint8_t* data;
};
#endif


// SECTION: Input system
// NOTE: KeyboardState data
struct KeyState {
    unsigned int MBL : 1; // NOTE: left mouse button
    unsigned int MBR : 1; // NOTE: right mouse button
    unsigned int MBM : 1; // NOTE: middle mouse button
    unsigned int Q : 1;                   
    unsigned int W : 1;
    unsigned int E : 1; 
    unsigned int R : 1; 
    unsigned int T : 1; 
    unsigned int Y : 1; 
    unsigned int U : 1; 
    unsigned int I : 1; 
    unsigned int O : 1; 
    unsigned int P : 1; 
    unsigned int A : 1; 
    unsigned int S : 1; 
    unsigned int D : 1; 
    unsigned int F : 1; 
    unsigned int G : 1; 
    unsigned int H : 1; 
    unsigned int J : 1; 
    unsigned int K : 1; 
    unsigned int L : 1; 
    unsigned int Z : 1; 
    unsigned int X : 1; 
    unsigned int C : 1; 
    unsigned int V : 1; 
    unsigned int B : 1; 
    unsigned int N : 1; 
    unsigned int M : 1; 
    unsigned int BRACE_LEFT : 1; 
    unsigned int BRACE_RIGHT : 1; 
    unsigned int BACKSLASH : 1; 
    unsigned int COLON : 1; 
    unsigned int QUOTE : 1; 
    unsigned int COMMA : 1; 
    unsigned int PERIOD : 1; 
    unsigned int FORWARDSLASH : 1;
    unsigned int NUM1 : 1; 
    unsigned int NUM2 : 1; 
    unsigned int NUM3 : 1; 
    unsigned int NUM4 : 1; 
    unsigned int NUM5 : 1; 
    unsigned int NUM6 : 1; 
    unsigned int NUM7 : 1; 
    unsigned int NUM8 : 1; 
    unsigned int NUM9 : 1; 
    unsigned int NUM0 : 1; 
    unsigned int TILDA : 1;
    unsigned int MINUS : 1; 
    unsigned int EQUALS : 1; 
    unsigned int BACKSPACE : 1;   
    unsigned int TAB : 1; 
    unsigned int CAPS : 1;
    unsigned int LEFTSHIFT : 1; 
    unsigned int LEFTCTRL : 1; 
    unsigned int MODKEY : 1;
    unsigned int LEFTALT : 1;
    unsigned int RIGHTALT : 1;
    unsigned int FUNCTION : 1;
    unsigned int RIGHTCTRL : 1;
    unsigned int RIGHTSHIFT : 1;
    unsigned int ENTER : 1;
    unsigned int ESC : 1;
    unsigned int SPACE : 1;
    unsigned int F1 : 1;
    unsigned int F2 : 1;
    unsigned int F3 : 1;
    unsigned int F4 : 1;
    unsigned int F5 : 1;
    unsigned int F6 : 1;
    unsigned int F7 : 1;
    unsigned int F8 : 1;
    unsigned int F9 : 1;
    unsigned int F10 : 1;
    unsigned int F11 : 1;
    unsigned int F12 : 1;
    unsigned int ARROWUP : 1;
    unsigned int ARROWDOWN : 1;
    unsigned int ARROWLEFT : 1;
    unsigned int ARROWRIGHT : 1;
    unsigned int PRINTSCREEN : 1;
    unsigned int SCREENLOCK : 1;
    unsigned int PAUSE : 1; 
    unsigned int INSERT : 1;
    unsigned int HOME : 1;
    unsigned int PAGEUP : 1;
    unsigned int DELETE_BUTTON : 1;
    unsigned int END : 1;
    unsigned int PAGEDOWN : 1;  
};


struct KeyboardState {
    KeyState state;
    KeyState prev_state;
};




struct ArenaAllocator {
    uint64_t offset_pointer;
    void* memory;
    size_t total_size; 
};
// SECTION: Memory. This is the persistant memory that
// has to be passed from the platform layer to the game
// or vice versa. 
// Using this memory should be done with the Arena Allocator functions.
struct GameMemory {
    bool is_initialized;
    uint64_t permanent_storage_size;
    ArenaAllocator permanent_storage;            // Gamestate stuff


    uint64_t asset_storage_size;    // Asset loading.
    ArenaAllocator asset_storage;
};

// NOTE: The game state representation.
struct GameState {
    void* state;
};



// SECTION: Platform-specific
struct WindowContext {
    SDL_Window* window;
    SDL_GLContext gl_context;
};

// NOTE: OpenGL implementation
struct GL_ID;

struct GPUBufferIDs {
    GL_ID* vao;
    GL_ID* vbo;
    GL_ID* ebo;
    GL_ID* instance_vbo;
};

struct Sprite {
    unsigned int pixel_width;
    unsigned int pixel_height;
    GL_ID* shader_id;
    GL_ID* texture_id;
    GPUBufferIDs buffers;
};