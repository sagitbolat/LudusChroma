#pragma once
#include "../sky_structs.h"

struct ImFont;

// NOTE: Goes in order Y_X. Used for UI alignment 
enum UI_Alignment {
    TOP_LEFT,
    TOP_CENTER,
    TOP_RIGHT,
    CENTER_LEFT,
    CENTER_CENTER,
    CENTER_RIGHT,
    BOTTOM_LEFT,
    BOTTOM_CENTER,
    BOTTOM_RIGHT
};

// NOTE: includes all the possible options for window creation.
struct UI_Window_Options {
    bool no_title_bar : 1;
    bool no_resize : 1;
    bool no_move : 1;
    bool no_scrollbar : 1;
    bool no_background : 1;
    bool no_collapse_button : 1;
};

void InitUI(WindowContext *ws);
void DeinitUI();
void UI_FrameStart();
void UI_FrameRender();

void UI_WindowStart(const char* window_name, Vector2Int window_size, Vector2Int window_pos, UI_Window_Options* window_options); 
void UI_WindowEnd();


ImFont* LoadFont(const char* font_path, float font_size);
void PushStyle(const ButtonStyle* style);
void PopStyle();

bool DrawSimpleButton(
    const char* label, 
    Vector2 screen_position, Vector2 scale, 
    ImFont* font, 
    const ButtonStyle* style = nullptr
);
bool DrawSimpleImageButton(
    const char* button_id, Sprite sprite, 
    Vector2 screen_position, Vector2 scale, 
    const ButtonStyle* style = nullptr,
    fColor background_color = {0.0f, 0.0f, 0.0f, 0.0f}, 
    fColor tint_color = {1.0f, 1.0f, 1.0f, 1.0f},
    Vector2 uv0 = {0.0f, 0.0f}, 
    Vector2 uv1 = {1.0f, 1.0f}
);
void DrawSimpleText(
    const char* text, 
    Vector2 screen_position, 
    UI_Alignment alignment, 
    ImFont* font = nullptr,
    fColor text_color = {1.0f, 1.0f, 1.0f, 1.0f} 
);
void DrawSimpleImage(
    Sprite sprite, 
    Vector2 screen_position, 
    Vector2 scale, 
    fColor tint_color = {1.0f, 1.0f, 1.0f, 1.0f},
    fColor border_color = {0.0f, 0.0f, 0.0f, 0.0f},
    Vector2 uv0 = {0.0f, 0.0f}, 
    Vector2 uv1 = {1.0f, 1.0f}
);
bool DrawSimpleTextbox( // FIXME: scaling this component doesnt work for some reason. idk.
    const char* label, 
    char* buffer, 
    size_t buffer_size,
    Vector2 screen_pos,
    Vector2 scale,
    ImFont* font = nullptr
);

// SECTION: For engine use

bool SkyUI_ProcessEvent(const SDL_Event* e);

unsigned int SkyGetGLID(GL_ID* id);
