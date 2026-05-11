#include "SDL_sky.cpp"
#include "sky_random.cpp"
#include "bmp.cpp"
#include <iostream>

#define WIDTH 1920
#define HEIGHT 1080


struct HuegeneState {
    Color cells[WIDTH * HEIGHT];
};


static void InitHuegene(GameBitmapBuffer* graphics_buffer, HuegeneState* hue_state);
static void RenderHuegene(GameBitmapBuffer* graphics_buffer, HuegeneState* hue_state);



static void Init(int* w, int* h) {
    *w = WIDTH;
    *h =  HEIGHT;
    return;
}

static void Awake(GameMemory* game_memory) {
    return;
}

static void Start(GameState* game_state, KeyboardState* keyboard_state) {
    HuegeneState* huegene_state = (HuegeneState*)game_state;
    InitHuegene(graphics_buffer, huegene_state);
    return;
}

static void Update(GameState* game_state, KeyboardState* keyboard_state, int delta_time) {
    HuegeneState* huegene_state = (HuegeneState*)game_state;
    if ((keyboard_state->state & KEY_STATE_SPACE) && !(keyboard_state->prev_state & KEY_STATE_SPACE)) {
        GenerateBitmapImage(huegene_state->cells, WIDTH, HEIGHT, (char*)"Huegene_output.bmp"); 
        printf("attempting bmp generation");
    }
    RenderHuegene(graphics_buffer, huegene_state);
    printf("Frame took %d ms to update. \n", delta_time);
    return;
}

static void UserFree() {
    return;
}

// SECTION: Huegene
static Color GetColorOfRandNeighbor(Color* cells, int x, int y, int max_x, int max_y) {
    Color color = {0};
    for (int c = x - 1; c <= x + 1; ++c) {
        for (int r = y - 1; r <= y + 1; ++r) {
            
            if ((c == x && r == y) || 
                (r < 0 || c < 0) ||
                (r >= max_y || c >= max_x)) {
                continue;
            }
            int i = (max_x * r) + c;
            if (cells[i].red != 0 || cells[i].green != 0 || cells[i].blue != 0) {
                color = cells[i];
                return color;
            }
    
        }
    }
    return color;
}

static void InitHuegene(GameBitmapBuffer* graphics_buffer, HuegeneState* hue_state) {
    //srand(123456789);
    
    Color color = {0};
    int x = WIDTH/2;
    int y = HEIGHT/2;

        
    color.red = 101;
    color.green = 124;
    color.blue = 255;
    

    int i = (WIDTH * y) + x;
    hue_state->cells[i] = color;    
}

static void RenderHuegene(GameBitmapBuffer* graphics_buffer, HuegeneState* hue_state) {
    
    int max_x = WIDTH;    
    int max_y = HEIGHT;    
    int square_size = 1; 
    int jaggedness = 5;
    int fade_speed = 2;
    int flip_chance = 70; //in percentages
    
    if (jaggedness < 1) {
        jaggedness = 1;
    }
    if (fade_speed < 1) {
        fade_speed = 1;
    }

     
    // update the cell_state
    for (int x = 0; x < max_x; ++x) {
        for (int y = 0; y < max_y; ++y) {
            int i = (max_x * y) + x;
            Color color = hue_state->cells[i];
            int to_mutate = sky_rand() % jaggedness;
            if (color.red == 0 && color.green == 0 && color.blue == 0 && to_mutate == 0) {
                color = GetColorOfRandNeighbor(hue_state->cells, x, y, max_x, max_y);
                uint8_t red_dec = sky_rand() % fade_speed;
                uint8_t green_dec = sky_rand() % fade_speed;
                uint8_t blue_dec = sky_rand() % fade_speed;
                
                // SECTION: MORE RANDOMIZATION. CAN DELETE.
                uint8_t red_flip = sky_rand() % 100;
                uint8_t green_flip = sky_rand() % 100;
                uint8_t blue_flip = sky_rand() % 100;
                if (red_flip < flip_chance) {
                   red_dec *= -1; 
                }
                if (green_flip < flip_chance) {
                   green_dec *= -1; 
                }
                if (blue_flip < flip_chance) {
                   blue_dec *= -1; 
                }
                // END OF SECTION

                if (color.red > red_dec) {
                    color.red -= red_dec;
                }
                if (color.green > green_dec) {
                    color.green -= green_dec;
                }
                if (color.blue > blue_dec) {
                    color.blue -= blue_dec;
                }
            }
            hue_state->cells[i] = color;    
            uint8_t red = hue_state->cells[i].red;
            uint8_t green = hue_state->cells[i].green;       
            uint8_t blue = hue_state->cells[i].blue;       
            
            
            DrawRectangle(red, green, blue, x, y, square_size, square_size);
        }
    }
}
