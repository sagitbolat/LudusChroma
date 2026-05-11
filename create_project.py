import sys
import os

# The directory path where you want to create the file
directory_path = 'Untitled_Project'
if len(sys.argv) > 1:
    directory_path = sys.argv[1]

file_path = os.path.join(directory_path, 'main.cpp')


# Create the directory if it doesn't exist
if not os.path.exists(directory_path):
    os.makedirs(directory_path)

    with open(file_path, 'w') as file:
        source = """#define INLUDE_IMGUI
#include "../Engine/SDL_sky.cpp"

void Init(
    int* w, int* h, 
    float* width_in_world_space, 
    bool* fullscreen,
    fColor* clear_color
) {
    *w = 1280;
    *h = 720;
    *width_in_world_space = 10.f;
    *fullscreen = false;
    *clear_color = {0.0f, 0.0f, 0.0f, 1.0f};
}

void Awake(GameMemory* gm) {

}

void Start(GameState* gs, KeyboardState* ks) {

}

void Update(GameState* gs, KeyboardState* ks, double dt) {

}

void UserFree() {

}
        """
        file.write(source)
