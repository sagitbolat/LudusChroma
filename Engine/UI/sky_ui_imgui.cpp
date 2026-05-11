#include <SDL2/SDL.h>
#include <GL/glew.h>
#include <GL/gl.h>

#include "sky_ui.h"
#define IMGUI_IMPL_OPENGL_LOADER_CUSTOM
#include "imgui/imgui.cpp"
#include "imgui/imgui_tables.cpp"
#include "imgui/imgui_widgets.cpp"
#include "imgui/imgui_draw.cpp"
#include "imgui/imgui_demo.cpp"
#include "imgui/imgui_impl_opengl3.cpp"
#include "imgui/imgui_impl_sdl2.cpp"

// SECTION: engine code. Not for user access

// NOTE: Called before gameloop
void InitUI(WindowContext *ws)
{
    ImGui::CreateContext();
    ImGui_ImplSDL2_InitForOpenGL(ws->window, ws->gl_context);
    ImGui_ImplOpenGL3_Init("#version 330 core");
}

// NOTE: Called after gameloop
void DeinitUI()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
}

// NOTE: Called before the game's Update/Start function is called
void UI_FrameStart()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();
}

// NOTE: Called after the game's Update/Start function is called
void UI_FrameRender()
{
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}


// NOTE: Passing a {0, 0} or nothing or negative values for window_size will make window fullscreen
// Passing {0, 0} or nothing for window_pos will make the window start in the top left of the screen. 
void UI_WindowStart(const char* window_name, Vector2Int window_size = {0, 0}, Vector2Int window_pos = {0, 0}, UI_Window_Options* window_options = nullptr) {

    if (window_size.x <= 0 || window_size.y <= 0) {
        ImGuiIO& io = ImGui::GetIO();
        ImVec2 display_size = io.DisplaySize;
        window_size.x = display_size.x;
        window_size.y = display_size.y;
    }
    
    ImGuiWindowFlags window_flags = 0; 
    
    if (window_options != nullptr) {
        window_flags = 
            (window_options->no_title_bar * ImGuiWindowFlags_NoTitleBar) |
            (window_options->no_resize * ImGuiWindowFlags_NoResize) |
            (window_options->no_move * ImGuiWindowFlags_NoMove) |
            (window_options->no_scrollbar * ImGuiWindowFlags_NoScrollbar) |
            (window_options->no_background * ImGuiWindowFlags_NoBackground) |
            (window_options->no_collapse_button * ImGuiWindowFlags_NoCollapse); 
    } else {
        window_flags = 
            (ImGuiWindowFlags_NoTitleBar) | 
            (ImGuiWindowFlags_NoResize) | 
            (ImGuiWindowFlags_NoMove) | 
            (ImGuiWindowFlags_NoScrollbar);
    }
    ImGui::SetNextWindowPos(ImVec2(window_pos.x, window_pos.y));
    ImGui::SetNextWindowSize(ImVec2(window_size.x, window_size.y));
    ImGui::Begin(window_name, nullptr, window_flags);
}
void UI_WindowEnd() {
    ImGui::End();
}



bool SkyUI_ProcessEvent(const SDL_Event* e) {
    return ImGui_ImplSDL2_ProcessEvent(e);
}

// SECTION: user code. Should be called inside the update or start methods of the game.




static inline ImVec4 fColorToImVec4(fColor c) {
    return ImVec4(c.r, c.g, c.b, c.a);
}





// NOTE: The screen_position ans scale should be passed with values between 0.0f and 1.0f.
// A scale of 1.0f, 1.0f will take up the whole screen.
// NOTE: RETURNS true if button is pressed. false otherwise

ImFont* LoadFont(const char* font_path, float font_size) {
    ImFontAtlas* font_atlas = ImGui::GetIO().Fonts;

    ImFontConfig font_config;
    font_config.FontDataOwnedByAtlas = false;

    ImFont* font = font_atlas->AddFontFromFileTTF(font_path, font_size, &font_config);

    return font;

}

// NOTE: Called at button function calls. Can also be called by user to default the same style on multiple buttons
// EX:
//  PushStyle(&my_style);
//  
//  DrawSimpleButton(...)       // No need to pass the my_style variable to this
//  DrawSimpleImageButton(...)  // No need to pass here either
//
//  PopStyle(&my_style);
void PushStyle(const ButtonStyle* style) {
    if (style == nullptr) return;
    ImGui::PushStyleColor(ImGuiCol_Button, fColorToImVec4(style->button_idle_color));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, fColorToImVec4(style->button_hover_color));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, fColorToImVec4(style->button_active_color));
    ImGui::PushStyleColor(ImGuiCol_Border, fColorToImVec4(style->outline_color));
    ImGui::PushStyleColor(ImGuiCol_Text, fColorToImVec4(style->text_color));

    ImGui::PushStyleVar(ImGuiStyleVar_Alpha, style->button_alpha);
    ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, ImVec2(style->text_alignment.x, style->text_alignment.y));
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, style->corner_rounding);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, style->border_size);
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(style->frame_padding.x, style->frame_padding.y));
}

void PopStyle() {
    ImGui::PopStyleColor(5);
    ImGui::PopStyleVar(5);
}

bool DrawSimpleButton(
    const char* label, 
    Vector2 screen_position, Vector2 scale, 
    ImFont* font, 
    const ButtonStyle* style
) {
    int height = ImGui::GetWindowSize().y;
    int width = ImGui::GetWindowSize().x;
    ImVec2 button_size((int)(scale.x * width), (int)(scale.y * height));

    ImVec2 button_position((int)(screen_position.x * width), (int)(screen_position.y * height));

    ImGui::SetCursorPos(button_position);

    ImGui::PushFont(font);

    PushStyle(style); 

    bool r = ImGui::Button(label, button_size);

    if (style != nullptr) {
        PopStyle();
    }

    ImGui::PopFont();
    
    return r;
}


// If You want the image button to preseve it's aspect ratio, 
// then pass the scale vector with one of the parameters as as negative value (eg. -1.0f).
// If scale.x is negative, then it will scale the height according to your scale.
// If scale.y is negative, then it will scale the width according to your scale.
// NOTE: The scale, just like a normal button is between 0.0 and 1.0, where 1.0f takes up the whole screen.
bool DrawSimpleImageButton(
    const char* button_id, Sprite sprite, 
    Vector2 screen_position, Vector2 scale, 
    const ButtonStyle* style,
    fColor background_color, 
    fColor tint_color,
    Vector2 uv0, 
    Vector2 uv1
) {
    int height = ImGui::GetWindowSize().y;
    int width = ImGui::GetWindowSize().x;

    if (scale.y < 0.0f) {
        scale.y = ((float)sprite.pixel_height/(float)sprite.pixel_width) * scale.x * ((float)width/(float)height);
    } else if (scale.x < 0.0f) {
        scale.x = (sprite.pixel_width/sprite.pixel_height) * scale.y * ((float)height/(float)width);
    }

    ImVec2 button_size((int)(scale.x * width), (int)(scale.y * height));

    ImVec2 button_position((int)(screen_position.x * width), (int)(screen_position.y * height));

    ImGui::SetCursorPos(button_position);
    
    PushStyle(style); 

    bool r = ImGui::ImageButton(
        button_id, (void*)SkyGetGLID(sprite.texture_id), 
        button_size, 
        ImVec2(uv0.x, 1.0f - uv0.y), ImVec2(uv1.x, 1.0f - uv1.y), 
        fColorToImVec4(background_color),
        fColorToImVec4(tint_color)
    );

    if (style != nullptr) {
        PopStyle(); 
    }

    return r;
}

// NOTE: Currently, vertical alignment is broken. Its close enough at medium sizes.
void DrawSimpleText(
    const char* text, 
    Vector2 screen_position, 
    UI_Alignment alignment, 
    ImFont* font,
    fColor text_color 
) {
    int height = ImGui::GetWindowSize().y;
    int width = ImGui::GetWindowSize().x;

    if(font != nullptr) ImGui::PushFont(font);

    ImFont* defaultFont = ImGui::GetIO().Fonts->Fonts[0]; // Get the default font

    
    ImVec2 text_position((int)(screen_position.x * width), (int)(screen_position.y * height));
    ImVec2 text_size = ImGui::CalcTextSize(text);

    // Calculate the baseline offset
    float baseline_offset = defaultFont->Ascent;
    
    switch(alignment) {
        case TOP_LEFT: {
            // NOTE: Already aligned on Top left corner
        } break;
        case TOP_CENTER: {
            text_position.x -= text_size.x * 0.5f;
        } break;
        case TOP_RIGHT: {
            text_position.x -= text_size.x; 
        } break;
        
        case CENTER_LEFT: {
            text_position.y -= text_size.y * 0.5f;
        } break;
        case CENTER_CENTER: {
            text_position.x -= text_size.x * 0.5f;
            text_position.y -= text_size.y * 0.5f;
        } break;
        case CENTER_RIGHT: {
            text_position.x -= text_size.x;
            text_position.y -= text_size.y * 0.5f;
        } break;
        
        case BOTTOM_LEFT: {
            text_position.y -= text_size.y + baseline_offset;
        } break;
        case BOTTOM_CENTER: {
            text_position.x -= text_size.x * 0.5f;
            text_position.y -= text_size.y + baseline_offset;
        } break;
        case BOTTOM_RIGHT: {
            text_position.x -= text_size.x;
            text_position.y -= text_size.y + baseline_offset;
        } break;
        
        default: {
        }break;
    }

    ImGui::SetCursorPos(text_position);

    ImGui::PushStyleColor(ImGuiCol_Text, fColorToImVec4(text_color));
    ImGui::Text(text);
    ImGui::PopStyleColor();
    if (font != nullptr) ImGui::PopFont();
    return;
}

// If You want the image to preseve it's aspect ratio, 
// then pass the scale vector with one of the parameters as as negative value (eg. -1.0f).
// If scale.x is negative, then it will scale the height according to your scale.
// If scale.y is negative, then it will scale the width according to your scale.
// NOTE: The scale, just like a normal button is between 0.0 and 1.0, where 1.0f takes up the whole screen.
void DrawSimpleImage(
    Sprite sprite, 
    Vector2 screen_position, 
    Vector2 scale, 
    fColor tint_color,
    fColor border_color,
    Vector2 uv0, 
    Vector2 uv1
) {
    int height = ImGui::GetWindowSize().y;
    int width = ImGui::GetWindowSize().x;

    if (scale.y < 0.0f) {
        scale.y = ((float)sprite.pixel_height/(float)sprite.pixel_width) * scale.x * ((float)width/(float)height);
    } else if (scale.x < 0.0f) {
        scale.x = (sprite.pixel_width/sprite.pixel_height) * scale.y * ((float)height/(float)width);
    }

    ImVec2 image_size((int)(scale.x * width), (int)(scale.y * height));

    ImVec2 image_position((int)(screen_position.x * width), (int)(screen_position.y * height));

    ImGui::SetCursorPos(image_position);

    ImGui::Image(
        (void*)SkyGetGLID(sprite.texture_id),
        image_size,
        ImVec2(uv0.x, 1.0f - uv0.y),
        ImVec2(uv1.x, 1.0f - uv1.y),
        fColorToImVec4(tint_color),
        fColorToImVec4(border_color)
    );

}


// NOTE: Textbox input
// Returns if the buffer was changed since last time the call was made.
bool DrawSimpleTextbox(
    const char* label, 
    char* buffer, 
    size_t buffer_size,
    Vector2 screen_position,
    Vector2 scale,
    ImFont* font
) {
    int height = ImGui::GetWindowSize().y;
    int width = ImGui::GetWindowSize().x;

    if(font != nullptr) ImGui::PushFont(font);

    ImFont* defaultFont = ImGui::GetIO().Fonts->Fonts[0]; // Get the default font
    
    ImVec2 text_position((int)(screen_position.x * width), (int)(screen_position.y * height));

    ImGui::SetCursorPos(text_position);
    
    ImGui::Text(label); // Display the label on the left
    ImGui::SameLine();
    ImGui::PushItemWidth(scale.x);
    bool r =  ImGui::InputText(label, buffer, buffer_size);
    ImGui::PopItemWidth();

    if (font != nullptr) ImGui::PopFont();

    return r;
}

// NOTE: Float slider
void DrawSimpleFloatSlider(const char* label, float* float_val, float min, float max) {

}
