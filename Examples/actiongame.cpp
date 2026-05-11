#include "../Engine/SDL_sky.cpp"
#include "../Engine/bmp.cpp"
#include "../Engine/tilemaps.cpp"
#include "../Engine/collisions.cpp"
#include "../Engine/truetype.cpp"
#include "../Engine/sky_random.cpp"
#include "../Engine/UI/button.cpp"

	
int SCREEN_H = 720;
int SCREEN_W = 1296;

// SECTION: main
void Init(int* w, int* h) {
    *w = SCREEN_W;
    *h = SCREEN_H;
}


ImageData player_sprite = {0};
ImageData bullet_sprite = {0};
ImageData enemy_sprite = {0};

ImageData menu_button_sprite = {0};
ImageData menu_hovered_button_sprite = {0};
RectButton start_button;

FontData font = {0};
FontData med_font = {0};
FontData large_font={0};

void Awake(GameMemory* gm) {
    LoadBitmap(&gm->asset_storage, "ship.bmp", &player_sprite); 
    LoadBitmap(&gm->asset_storage, "bullet.bmp", &bullet_sprite);
    LoadBitmap(&gm->asset_storage, "enemy.bmp", &enemy_sprite);
    LoadBitmap(&gm->asset_storage, "button.bmp", &menu_button_sprite);
    LoadBitmap(&gm->asset_storage, "button_hover.bmp", &menu_hovered_button_sprite);


    font.data = (uint8_t**)malloc(sizeof(uint8_t*)*128); // NOTE: Allocate all the glyph pointers.
    LoadFont(&gm->asset_storage, "pixel.ttf", &font, 64);
    med_font.data = (uint8_t**)malloc(sizeof(uint8_t*)*128); // NOTE: Allocate all the glyph pointers.
    LoadFont(&gm->asset_storage, "pixel.ttf", &med_font, 128);
    large_font.data = (uint8_t**)malloc(sizeof(uint8_t*)*128); // NOTE: Allocate all the glyph pointers.
    LoadFont(&gm->asset_storage, "pixel.ttf", &large_font, 256);
    
    InitButton(&start_button, SCREEN_W/2, 360, 5, menu_button_sprite, menu_hovered_button_sprite, menu_button_sprite);


    sky_srand(69); // NOTE: random seed set.
}

Vector2* bullet_position_pool;
int bullet_pool_size = 15;

Vector2* enemy_position_pool;
int enemy_pool_size = 50;

void Start(GameState* gs, KeyboardState* ks) {
    bullet_position_pool = (Vector2*)malloc(sizeof(Vector2) * bullet_pool_size); 
    for (int b = 0; b < bullet_pool_size; ++b) {
        // NOTE: Set all bullet positions to be negative to start
        bullet_position_pool[b].x = -100;
        bullet_position_pool[b].y = -100;
    }

    enemy_position_pool = (Vector2*)malloc(sizeof(Vector2) * enemy_pool_size);
    for (int e = 0; e < enemy_pool_size; ++e) {
        enemy_position_pool[e].x = -100;
        enemy_position_pool[e].y = -100;
    }
}


enum scene {
    main_menu = 0,
    game = 1,
    game_over = 2
};

void Update(GameState* gs, KeyboardState* ks, int dt) {
    
    // SECTION: Default text colors;
    Color text_color = {0};
    text_color.red = 99;
    text_color.green = 155;
    text_color.blue = 255;

    // SECTION: All The static data
    static int total_time = 0;

    static enum scene curr_scene = main_menu;

    const float INIT_PLAYER_POS_X = SCREEN_W/2; 
    const float INIT_PLAYER_POS_Y = SCREEN_H/2 + 200; 
    static float x_pos = INIT_PLAYER_POS_X;
    static float y_pos = INIT_PLAYER_POS_Y;
    
    const int MAX_HEALTH = 3;
    static int player_health = MAX_HEALTH;
    static int player_score = 0;
    
    static int curr_enemy_pointer = 0;
    static int curr_bullet_pointer = 0; // POINTER OF THE BULLER POOL 
   
    const int INIT_SPAWNRATE = 400; 
    static int SPAWNRATE = INIT_SPAWNRATE;
    static int spawn_timer = 0;
    static int spawn_counter = 0;
    
    static float enemy_speed = 0.7;
    static float bullet_speed = 1.0;
    static float player_speed = 0.5;

    const int FIRERATE = 200; // NOTE: How often the bullet will shoot in ms.
    static int fire_timer = 0;


    // SECTION: Slow motion
    if (ks->state.SPACE) {
        dt /= 2;
        fire_timer -= dt/2; //NOTE: No shooting when in slowmo.
    }
    // SECTION: Fast motion
    if (ks->state.ENTER) {
        dt *= 2;
        fire_timer += dt; //NOTE: Shooting twice as fast when in fast motion.
    }


    // SECTION: Timekeeping and mouse position caching
    total_time += dt;
    Vector2Int mouse_pos = GetMousePosition();

    

    // SECTION: Main Menu Scene
    if (curr_scene == main_menu) {
        if (PointRectCollision(start_button.collider, mouse_pos))
        {
            if (ks->state.SPACE) {
                start_button.state = active;
                curr_scene = game;
                return;
            }
            start_button.state = hovered;
        } else {
            start_button.state = inactive;
        }
        DisplayText(graphics_buffer, "Space Shooter", &med_font, text_color, 325, 20, 1, 0);
        BlitButton(graphics_buffer, &start_button);
        DisplayText(graphics_buffer, "Start", &med_font, text_color, SCREEN_W/2 - 125, 265, 1, 0);
        return;
    } 
    
    
    
    // SECTION: Game Over Scene
    if (curr_scene == game_over) {
        DisplayText(graphics_buffer, "GAME OVER", &large_font, text_color, 150, 100, 1, 0);
        DisplayText(graphics_buffer, "Press [SPACE] to restart", &font, text_color, 400, 600, 1, 0);
        if (ks->state.SPACE && !ks->prev_state.SPACE) {
            player_health = MAX_HEALTH;
            SPAWNRATE = INIT_SPAWNRATE;
            spawn_counter = 0;
            spawn_timer = 0;
            player_score = 0;
            fire_timer = 0;
            curr_enemy_pointer = 0;
            curr_bullet_pointer = 0;
            x_pos = INIT_PLAYER_POS_X;
            y_pos = INIT_PLAYER_POS_Y;
            for (int b = 0; b < bullet_pool_size; ++b) {
                // NOTE: Set all bullet positions to be negative to start
                bullet_position_pool[b].x = -100;
                bullet_position_pool[b].y = -100;
            }
            for (int e = 0; e < enemy_pool_size; ++e) {
                enemy_position_pool[e].x = -100;
                enemy_position_pool[e].y = -100;
            }
            curr_scene = game;
        }
        return;
    }



    // SECTION: Updating enemy pool positions:
    for (int e = 0; e < enemy_pool_size; ++e) {
        // NOTE: increment all enemy positions downward.
        // If bullet is out of screen, set its position to -100.
        // NOTE: the -50 is a value that allows us to spawn enemies offscreen without them being tagged as inactive.
        if (enemy_position_pool[e].x < -50 || enemy_position_pool[e].y < -50) {
            continue;
        }
        enemy_position_pool[e].y += (float)(dt) * enemy_speed; 

        if (enemy_position_pool[e].y < -50 || 
            enemy_position_pool[e].y > SCREEN_H + 10 ||
            enemy_position_pool[e].x < 0 ||
            enemy_position_pool[e].x > SCREEN_W + 10
        ) {
            enemy_position_pool[e].y = -100;
            enemy_position_pool[e].x = -100;
        }
    }
    // SECTION: Resetting enemy pool pointer if its at the end of the pool
    if (curr_enemy_pointer >= enemy_pool_size) {
        curr_enemy_pointer = 0;
    }



    // SECTION: Updating bullet pool positions
    for (int b = 0; b < bullet_pool_size; ++b) {
        // NOTE: increment all bullet positions upwards.
        // If bullet is out of screen, set its position to -100.
        if (bullet_position_pool[b].x < 0 || bullet_position_pool[b].y < 0) {
            continue;
        }
        bullet_position_pool[b].y -= (float)(dt) * bullet_speed;

        if (bullet_position_pool[b].y > SCREEN_H + 10) {
            bullet_position_pool[b].y = -100;
            bullet_position_pool[b].x = -100;
        }
        if (bullet_position_pool[b].y < 0) {
            bullet_position_pool[b].y = -100;
            bullet_position_pool[b].x = -100;
        }
    }
    // SECTION: Resetting bullet pool pointer if its at the end of the pool
    if (curr_bullet_pointer >= bullet_pool_size) {
        curr_bullet_pointer = 0;
    }
    
   

    // SECTION: handling player movement with keystates
    if (ks->state.S && y_pos < SCREEN_H) {
        y_pos += dt * player_speed;
    }
    if (ks->state.D && x_pos < SCREEN_W) {
        x_pos += dt * player_speed;
    }
    if (ks->state.W && y_pos > 0) {
        y_pos -= dt * player_speed;
    }
    if (ks->state.A && x_pos > 0) {
        x_pos -= dt * player_speed;
    }
    
   

    // SECTION: Enemy Spawning
    spawn_timer += dt;
    if (spawn_timer > SPAWNRATE) {
        spawn_timer -= SPAWNRATE;
        enemy_position_pool[curr_enemy_pointer].x = sky_rand() % SCREEN_W; // NOTE: spawns at random x position.
        enemy_position_pool[curr_enemy_pointer].y = 0;
        curr_enemy_pointer += 1;
        ++spawn_counter;
        if (spawn_counter >= 5 && SPAWNRATE >= 120) {
            SPAWNRATE = SPAWNRATE-20;
            spawn_counter = 0;
        }
    }



    // SECTION: Shooting
    fire_timer += dt;
    if (fire_timer > FIRERATE) {
        fire_timer = 0;
        bullet_position_pool[curr_bullet_pointer].y = y_pos;
        bullet_position_pool[curr_bullet_pointer].x = x_pos;
        curr_bullet_pointer += 1;
    }



    // SECTION: Deactivating bullets and enemies if they collide and initiating lose state if player collides with enemies:
    Rect player_collider = {0};
    // NOTE: The player collider is smaller than the sprite to make the game easier
    player_collider.x = x_pos - (6 * 5);
    player_collider.y = y_pos - (6 * 5);
    player_collider.width = (12 * 5);
    player_collider.height = (12 * 5);
    for (int e = 0; e < enemy_pool_size; ++e) {
        if (enemy_position_pool[e].y < -50 || enemy_position_pool[e].x < -50) {
            continue;
        }
        float ex = enemy_position_pool[e].x;
        float ey = enemy_position_pool[e].y;
        Rect enemy_collider = {0};
        enemy_collider.x = ex - (4 * 5);
        enemy_collider.y = ey - (4 * 5);
        enemy_collider.width = (8 * 5);
        enemy_collider.height = (8 * 5);
        
        // NOTE: If player collides with enemy, destroy the enemy.
        if (RectCollision(player_collider, enemy_collider)) {
            player_health -= 1;   
            enemy_position_pool[e].y = -100;
            enemy_position_pool[e].x = -100;
        }

        for (int b = 0; b < bullet_pool_size; ++b) {
            if (bullet_position_pool[b].y < -50 || bullet_position_pool[b].x < -50) {
                continue;
            }
            float bx = bullet_position_pool[b].x;
            float by = bullet_position_pool[b].y;
            Rect bullet_collider = {0};
            bullet_collider.x = bx - (1 * 5);
            bullet_collider.y = by - (1 * 5);
            bullet_collider.width = (2 * 5);
            bullet_collider.height = (2 * 5);
            if (RectCollision(enemy_collider, bullet_collider)) {
                ++player_score;
                enemy_position_pool[e].x = -100;
                enemy_position_pool[e].y = -100;
                bullet_position_pool[b].x = -100;
                bullet_position_pool[b].y = -100;
            }
        }
    }
    
    

    // SECTION: If player dies, switch scene to gameover
    if(player_health <= 0) curr_scene = game_over;


    // SECTION: Rendering enemies
    for (int e = 0; e < enemy_pool_size; ++e) {
        if (enemy_position_pool[e].y < 0 || enemy_position_pool[e].x < 0) {
            continue;
        }
        int enemy_x = (int)enemy_position_pool[e].x;
        int enemy_y = (int)enemy_position_pool[e].y;
        
        BlitBitmapScaled(graphics_buffer, &enemy_sprite, (int)enemy_x, (int)enemy_y, 5, 1, true);
    }
    // SECTION: Rendering bullets
    for (int b = 0; b < bullet_pool_size; ++b) {
        if (bullet_position_pool[b].y < 0 || bullet_position_pool[b].x < 0) {
            continue;
        }
        int bullet_x = (int)bullet_position_pool[b].x;
        int bullet_y = (int)bullet_position_pool[b].y;

        BlitBitmapScaled(graphics_buffer, &bullet_sprite, (int)bullet_x, (int)bullet_y, 5, 1, true);
    }
    // SECTION: Rendering Player
    if(player_health > 0) {
        BlitBitmapScaled(graphics_buffer, &player_sprite, (int)x_pos, (int)y_pos, 5, 1, true);
    }
    
   

    // SECTION: Render Score and health text:
    char score_str[100];
    snprintf(score_str, 100, "Score: %d", player_score);
    DisplayText(graphics_buffer, score_str, &font, text_color, 1000, 0, 1, 0);
    char health_str[100];
    snprintf(health_str, 100, "Health: %d", player_health);
    DisplayText(graphics_buffer, health_str, &font, text_color, 0, 0, 1, 0);


}

void UserFree() {
    free(bullet_position_pool);
    free(enemy_position_pool);    
}
