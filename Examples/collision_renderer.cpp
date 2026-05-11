#include "../Engine/SDL_sky.cpp"
#include "../Engine/skymath.h"
#include "../Engine/sky_random.cpp"
#include "../Engine/collisions.cpp"

#define PIXELS_PER_UNIT 16.0f // NOTE: Number of pixels in one unit of worldspace (used to keep pixel sprites scales consistent)


void Init(int* w, int* h, float* w_in_world_space, bool* fullscreen, fColor* clear_color) {
    //*w = 640;
    //*h = 480;
    *w_in_world_space = 15.0f;
    *clear_color = {0.8f/2, 0.83f/2, 1.0f/2, 1.0f};
}

Sprite sprite = {0};
Transform transform1 = {0.0f};
Transform transform2 = {0.0f};

void Awake(GameMemory* gm) {
    sky_srand(199213);
    // SECTION: Load Ship Sprite
    sprite = LoadSprite("square.bmp");
    transform1.position = {0.0f, 0.0f, 0.0f};
    transform1.rotation = {0.0f, 0.0f, 45.0f};
    // NOTE: Scale the sprite based on its pixel size.
    transform1.scale = {1.0f, 1.0f, 1.0f};

    transform2.position = {0.5f, 0.5f, 0.0f};
    transform2.rotation = {0.0f, 0.0f, 0.0f};
    // NOTE: Scale the sprite based on its pixel size.
    transform2.scale = {1.0f, 1.0f, 1.0f};

}



void Start(GameState* gs, KeyboardState* ks) {

    bool ovlap_y = OverlapOnAxis(transform1, transform2, Vector3{0.0f, 1.0f, 0.0f});
    bool ovlap_x = OverlapOnAxis(transform1, transform2, Vector3{1.0f, 0.0f, 0.0f});
    
    printf("Overlap on y axis: %s\n", ovlap_y ? "true" : "false");
    printf("Overlap on x axis: %s\n", ovlap_x ? "true" : "false");


}

void Update(GameState* gs, KeyboardState* ks, double dt) {
    DrawSprite(sprite, transform1, main_camera);
    DrawSprite(sprite, transform2, main_camera);

   
    if (ks->state.SPACE && !ks->prev_state.SPACE) {
        bool colliding = TestCollision(transform1, transform2);

        printf("Colliding: %s\n", colliding ? "true" : "false");
    } 

    float move_speed = 6.0f;
    float rotation_speed_scaled = 315.0f * ((float)dt / 1000.0f);
    
    Vector3 movement_direction = {};
    
    
    if (ks->state.W) {
        movement_direction.y += 1.0f;
    }
    if (ks->state.A) {
        movement_direction.x -= 1.0f;
    }
    if (ks->state.S) {
        movement_direction.y -= 1.0f;
    }
    if (ks->state.D) {
        movement_direction.x += 1.0f;
    }
    movement_direction = Normalize(movement_direction);
    if (!isnan(movement_direction.x) && !isnan(movement_direction.y)) {
        transform1.position.x = transform1.position.x + (movement_direction.x * move_speed * ((float)dt / 1000.0f));
        transform1.position.y = transform1.position.y + (movement_direction.y * move_speed * ((float)dt / 1000.0f));
    }

    
    if (ks->state.Q) {
        transform1.rotation.z += rotation_speed_scaled;
    }
    if (ks->state.E) {
        transform1.rotation.z -= rotation_speed_scaled;
    }   
    if (ks->state.Z) {
        transform2.rotation.z += rotation_speed_scaled;
    }
    if (ks->state.C) {
        transform2.rotation.z -= rotation_speed_scaled;
    }   

}


void UserFree() {

}

