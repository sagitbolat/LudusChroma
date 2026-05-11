#include "Engine/SDL_sky.cpp"
#include "Engine/bmp.cpp"

void Init(int* w, int* h) {
    *w = 1280;
    *h = 720;
    return;
}

ImageData asteroid_sprite = {0};
ImageData ship_sprite = {0};

// NOTE: An arena allocator with lifetime of one frame.
ArenaAllocator frame_arena = {0};
uint64_t frame_arena_size = Kilobytes(4);

void Awake(GameMemory* gm) {
    LoadBitmap(&gm->asset_storage, "colortile.bmp", &asteroid_sprite);
    LoadBitmap(&gm->asset_storage, "ship.bmp", &ship_sprite);
    InitArena(&frame_arena, frame_arena_size);
    return;
}

void Start(GameState* gs, KeyboardState* ks) {
    return;
}

int asteroid_x = 1280/2;
int asteroid_y = 720/2;
float angle = 0; 

void Update(GameState* gs, KeyboardState* ks, int dt) {
    // SECTION: zero out the frame_arena
    MemsetArena(&frame_arena, 0);


    //printf("%d \n", dt);
    //float delta_time = (float)dt / 1000;
    //asteroid_x += (velocity * delta_time);
    //asteroid_y += (velocity * delta_time);
    if (
        ks->state.SPACE == 1 
        && 
        ks->prev_state.SPACE == 0
    ) { 
        asteroid_x += 1;
        asteroid_y += 1;
    }
    //printf("%f \n", asteroid_x);
    
    // NOTE: What I want out of blitBitmap functionality:
    // Blit centered, default resolution
    // Blit centered, resolution is recalculated based on scale.
    // Blit centered, at a scale, default resolution
    // Blit centered, at a scale, resolution is recalculated based on scale.

    //BlitBitmap(graphics_buffer, &ship_sprite, 300, 300, 3, true, true);
    //BlitBitmap(graphics_buffer, &asteroid_sprite, (int)(asteroid_x), (int)(asteroid_y), 16, false, false);
    //BlitBitmapScaled(graphics_buffer, &ship_sprite, 300, 300, 3, 1, true);
    int scale = 16;
    int pos_scale = 1;
    bool centered = true;
    if (ks->state.W == 1) {
        angle += 1;
    } else if (ks->state.S == 1) {
        angle = 360;
    } else if (ks->state.A == 1 && ks->prev_state.A == 0) {
        angle -= 15;
    } else if (ks->state.D == 1 && ks->prev_state.D == 0) {
        angle += 15;
    }
    //printf("%f\n", angle);
     
    ImageData asteroid_rotated = RotateBitmap(&frame_arena, asteroid_sprite, angle);
    BlitBitmapScaled(graphics_buffer, &asteroid_rotated, (int)(asteroid_x), (int)(asteroid_y), scale, pos_scale, centered);
    BlitBitmapScaled(graphics_buffer, &asteroid_sprite, (int)(0), (int)(0), scale, pos_scale, false);
    //DrawRectangle(graphics_buffer, 255, 255, 255, 0, 0, 10, 10);
    //DrawRectangle(graphics_buffer, 255, 255, 255, 10, 10, 10, 10);
    if (ks->state.I == 1 && ks->prev_state.I == 0) {
	GenerateBitmapImage(asteroid_rotated, "RotatedTest.bmp"); 
    	printf("Exporting Bitmap\n");
    }
    return;

}

void UserFree() {
} 
