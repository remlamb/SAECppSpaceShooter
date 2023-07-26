#include <iostream>
#include <cstdlib>
#include <vector>
#include <string>
#include "..\libs\include\MiniFB.h"
#include "..\libs\include\MiniFB_enums.h"

#define SOKOL_IMPL
#include "..\libs\include\sokol_log.h"
#include "..\libs\include\sokol_audio.h"

#define STB_IMAGE_IMPLEMENTATION
#include "..\libs\include\stb_image.h"

#define _USE_MATH_DEFINES
#include <math.h>


#include "draw.cpp"
#include "image.cpp"
#include "font.cpp"
#include "audio.cpp"


#define WINDOW_SIZE_X 400
#define WINDOW_SIZE_Y 400

#define WINDOW_FAC 2


#define FRAME_SX 200
#define FRAME_SY 200


#define WINDOW_SX (FRAME_SX * WINDOW_FAC)
#define WINDOW_SY (FRAME_SY * WINDOW_FAC)

#define COLOR_WHITE 0xFFFFFFFF
#define COLOR_BLACK 0xFF000000

#define COLOR_BG 0xFF0F1226

#define COLOR_BLUE 0xFF0000FF
#define COLOR_CYAN 0xFF00FFFF
#define COLOR_GREEN 0xFF00FF00
#define COLOR_RED 0xFFFF0000

#define COLOR_GAMEOVER 0xFFFF2E2E
#define COLOR_UI 0xFFFFD68F



#define ARR_LEN(arr) ((int) (sizeof(arr)/ sizeof(*arr)))
#define MAX(a, b) ((a > b) ? a : b)
#define MIN(a, b) ((a < b) ? a : b)

uint32_t* buffer;

void resize_bitmap(uint32_t* dest, int dest_sx, int dest_sy, uint32_t* src, int src_sx, int src_sy)
{
    for (int y = 0; y < dest_sy; y++) {
        for (int x = 0; x < dest_sx; x++) {
            int src_x = x * src_sx / dest_sx;
            int src_y = y * src_sy / dest_sy;
            dest[y * dest_sx + x] = src[src_y * src_sx + src_x];
        }
    }
}

//------INPUTS
bool keys_state[KB_KEY_LAST + 1]; // KB_KEY_LAST ->define du dernier Key 
bool keys_state_prev[KB_KEY_LAST + 1];

void tick_input()
{
    memcpy(keys_state_prev, keys_state, sizeof(keys_state));
}

void on_keyboard_event(struct mfb_window* window, mfb_key key, mfb_key_mod mod, bool isPressed)
{
    keys_state[key] = isPressed;
}

bool is_key_down(mfb_key key) {
    return keys_state[key];
}

bool was_key_just_pressed(mfb_key key) {
    return !keys_state_prev[key] && keys_state[key];
}

bool was_key_just_released(mfb_key key) {
    return keys_state_prev[key] && !keys_state[key];
}

//----Star
#pragma region Star
typedef struct star_t {
    int x;
    int y;
}star_t;

star_t stars[20];
int star_count = 0;
#pragma endregion Star

void spawn_star(int x, int y) {
    if (star_count == 20 - 1) {
        return;
    }
    stars[star_count] = (star_t){ .x = x, .y = y };
    star_count++;
}


//----Bullet
#pragma region Bullet
typedef struct bullet_t {
    int x;
    int y;
    bool isReflected = false; 
}bullet_t;

bullet_t bullets[10];
int bullet_count = 0;
#pragma endregion Bullet

void spawn_bullet(int x, int y) {
    if (bullet_count == 10 - 1) {
        return;
    }
    bullets[bullet_count] = (bullet_t){ .x = x, .y = y };
    bullet_count++;
}


#pragma region Enemy
typedef struct enemy_t {
    int x;
    int y;
    bool isHit = false;
}enemy_t;

enemy_t enemies[1];
int enemy_count = 0;
bool reflect_state = true;
#pragma endregion Enemy

void spawn_enemy(int x, int y) {
    if (enemy_count == 1 - 1) {
        return;
    }
    enemies[enemy_count] = (enemy_t){ .x = x, .y = y };
    enemy_count++;
}

void moving_enemy(float enemy_speed, int enemy_count, enemy_t* enemies) {
    for (int i = 0; i <= enemy_count; i++) {
        enemy_t* current_enemy = &enemies[i];
        current_enemy->y += enemy_speed;
    }
}

void enemy_invulnerability_phase(int& frameCounter) {
    //frame Counter to Set up The enemy invulnerability phase
    frameCounter++;
    if (frameCounter > 200) {
        frameCounter = 0;
    }

    if (frameCounter >= 60 && frameCounter <= 140) {
        reflect_state = true;
    }
    else {
        reflect_state = false;
    }
}
void draw_enemy(int enemy_count, enemy_t* enemies, bool reflect_state, img_t shield, img_t enemy) {
    for (int i = 0; i <= enemy_count; i++) {
        enemy_t* current_enemy = &enemies[i];
        if (reflect_state) {
            DrawImageCentered(current_enemy->x, current_enemy->y, shield);
        }
        DrawImageCentered(current_enemy->x, current_enemy->y, enemy);
    }
}

#pragma region Player
int player_posX = FRAME_SX/2;
bool player_shoot_tuto = true;
int player_life = 3; 
#pragma endregion Player

int score = 0;

void player_control(int& player_posX) {
    if (is_key_down(KB_KEY_RIGHT)) {
        player_posX += 3;
    }

    if (is_key_down(KB_KEY_LEFT)) {
        player_posX -= 3;
    }
}

void DrawAllWindowPixel(uint32_t* buffer, uint32_t color) {
    for (int i = 0; i < FRAME_SX * FRAME_SY; i++)
    {
        buffer[i] = color;
    }
}

void DrawUI(bool player_shoot_tuto, int player_life, img_t font, img_t heart) {
    if (player_shoot_tuto) {
        DrawTextCentered(font, "SPACE to SHOOT \x6", 0, 0, COLOR_UI);
    }

    if (player_life <= 0) {
        DrawTextCentered(font, "GAME OVER", 0, 0, COLOR_GAMEOVER);
    }
    DrawTextCentered(font, "SCORE : " + std::to_string(score), 0, -FRAME_SY/2 + 2, COLOR_UI);
    if (player_life > 0) {
        DrawImageAlpha(5, 2, heart);
        if (player_life > 1) {
            DrawImageAlpha(5 + 10, 2, heart);
            if (player_life > 2) {
                DrawImageAlpha(5 + 16 + 4, 2, heart);
            }
        }
    }
}

void PianoKey() {
    if (was_key_just_pressed(KB_KEY_1)) {
        audio_play_sound(262, 2);
    }

    if (was_key_just_pressed(KB_KEY_2)) {
        audio_play_sound(294, 2);
    }

    if (was_key_just_pressed(KB_KEY_3)) {
        audio_play_sound(329, 2);
    }

    if (was_key_just_pressed(KB_KEY_4)) {
        audio_play_sound(350, 2);
    }

    if (was_key_just_pressed(KB_KEY_5)) {
        audio_play_sound(392, 2);
    }

    if (was_key_just_pressed(KB_KEY_6)) {
        audio_play_sound(440, 2);
    }

    if (was_key_just_pressed(KB_KEY_7)) {
        audio_play_sound(494, 2);
    }

    if (was_key_just_pressed(KB_KEY_8)) {
        audio_play_sound(523, 2);
    }
}

bool game_paused = true; 
bool gameover_sfx = false;

int main()
{
    //Audio Desc 
    saudio_desc audio_desc = {};
    audio_desc.stream_cb = audio_callback;//stream callback
    audio_desc.logger.func = slog_func;

    saudio_setup(audio_desc);

    //load assets
    img_t font;
    LoadImage(font, "assets/font_map.png");

    img_t laser;
    LoadImage(laser, "assets/laser.png");

    img_t shield;
    LoadImage(shield, "assets/shield.png");

    img_t enemy;
    LoadImage(enemy, "assets/enemy.png");

    img_t player;
    LoadImage(player, "assets/player.png");

    img_t laserReverbe;
    LoadImage(laserReverbe, "assets/laserReverbe.png");

    img_t heart;
    LoadImage(heart, "assets/heart.png");

    //set up pos for the 1st enemy
    enemy_t* firstEnemy = &enemies[0];
    firstEnemy->x = FRAME_SX / 2;



    struct mfb_window* window = mfb_open_ex("Game", WINDOW_SIZE_X, WINDOW_SIZE_Y, WF_RESIZABLE);
    if (!window) {
        return 0;
    }
        

	buffer = (uint32_t*)malloc(WINDOW_SIZE_X * WINDOW_SIZE_Y * 4);
    resizeBuffer = (uint32_t*)malloc(FRAME_SX * FRAME_SY * 4);

    //background
    DrawAllWindowPixel(resizeBuffer, COLOR_BG);

    //spawn Stars
    for (int starsidx = 0; starsidx < (int) sizeof(stars); starsidx++) {
        spawn_star(std::rand() % FRAME_SX - 1, std::rand() % FRAME_SY - 1);
    }


    DrawImageCentered(player_posX, FRAME_SY - player.height, player);

    int frameCounter = 0;
    mfb_set_keyboard_callback(window, on_keyboard_event);
    sound_clip_t song = load_sound_clip("assets/song.wav");
    audio_play_sound_clip(song);
    sound_clip_t hit = load_sound_clip("assets/hit.wav");
    sound_clip_t gameover = load_sound_clip("assets/gameover.wav");

    do {
        int state;



        //Quit with Esc
        if (was_key_just_pressed(KB_KEY_ESCAPE))
        {
            return 0;
        }

        //Shooting Laser
        if (was_key_just_pressed(KB_KEY_SPACE) && player_life > 0)
        {
            spawn_bullet(player_posX, FRAME_SY - 18);
            audio_play_sound(440 * 2 + std::rand() % 80, 2, true);


            if (player_shoot_tuto) {
                player_shoot_tuto = false;
            }
        }


        if (player_shoot_tuto || player_life <= 0) {
            game_paused = true;
        }
        else {
            game_paused = false;
        }

        if (!game_paused) {
            //background // clear window each frame
            DrawAllWindowPixel(resizeBuffer, COLOR_BG);

            //Collision
            //laser - enemy
            for (int bulletIdx = 0; bulletIdx < bullet_count; bulletIdx++) {
                for (int idx = bullets[bulletIdx].x; idx <= bullets[bulletIdx].x + laser.width; idx++) {
                    for (int i = enemies[0].x - enemy.width / 2; i <= enemies[0].x + enemy.width / 2; i++) {
                        for (int idx_Y = bullets[bulletIdx].y; idx_Y <= bullets[bulletIdx].y + laser.height; idx_Y++) {
                            for (int i_Y = enemies[0].y; i_Y <= enemies[0].y + enemy.height; i_Y++) {
                                if (idx_Y == i_Y && idx == i) {
                                    if (!reflect_state) {
                                        //std::cout << "Collision";
                                        score += 20;
                                        bullets[bulletIdx] = bullets[bullet_count - 1];
                                        bullet_count--;

                                        enemy_t* pblock = &enemies[0];
                                        pblock->isHit = true;
                                        bulletIdx--;
                                        audio_play_sound(200 * 2 - std::rand() % 100, 2, true);                                    
                                    }
                                    else {
                                        bullet_t* pBull = &bullets[bulletIdx];
                                        pBull->isReflected = true;
                                        audio_play_sound(880 * 2 + std::rand() % 80, 2, true);
                                        //std::cout << "Collision";
                                    }
                                }
                            }
                        }
                    }
                }
            }

            //laser - player
            for (int bulletIdx = 0; bulletIdx < bullet_count; bulletIdx++) {
                for (int idx = bullets[bulletIdx].x; idx <= bullets[bulletIdx].x + laser.width; idx++) {
                    for (int i = player_posX - player.width / 2; i <= player_posX + player.width / 2; i++) {
                        for (int idx_Y = bullets[bulletIdx].y; idx_Y <= bullets[bulletIdx].y + laser.height; idx_Y++) {
                            if (idx_Y == FRAME_SY - 18 && idx == i) {
                                if (bullets[bulletIdx].isReflected) {
                                    //std::cout << "Collision";
                                    audio_play_sound_clip(hit);
                                    //audio_play_sound(220 - std::rand() % 100, 2);
                                    bullets[bulletIdx] = bullets[bullet_count - 1];
                                    bullet_count--;
                                    bulletIdx--;

                                    //Degat

                                    player_life -= 1;
                                }
                            }
                        }
                    }
                }
            }

            spawn_star(std::rand() % FRAME_SX - 1, std::rand() % FRAME_SY/4 - 1); // respawn a un endroit aleatoire en x 

            for (auto& star : stars) {
                if (star.y < 0 || star.y > FRAME_SY) {
                    star = stars[star_count - 1];
                    star_count--;
                }
                star.y += 1;
                DrawPixel(star.x, star.y, 0xFFC4B382);
            }

            moving_enemy(1, enemy_count, enemies);
            enemy_invulnerability_phase(frameCounter);

            for (auto& block : enemies) {

                if (reflect_state) {
                    DrawImageCentered(block.x, block.y, shield);
                }
                DrawImageCentered(block.x, block.y, enemy);

                if (block.y < 0 || block.y > FRAME_SY || block.isHit) {
                    block = enemies[enemy_count - 1];
                    enemy_count--;
                    spawn_enemy(std::rand() % FRAME_SX + 1, 1); // respawn a un endroit aleatoire en x 

                    if (!block.isHit) {
                        score -= 2;
                    }
                }
            }



            //---Player
            player_control(player_posX);
            //Draw Player
            DrawImageCentered(player_posX, FRAME_SY - player.height, player);

            //test Piano
            PianoKey();

            for (int i = 0; i < bullet_count; i++) {
                bullet_t* bullet = &bullets[i];
                if (bullet->isReflected) {
                    bullet->y += 4;
                    DrawImageCentered(bullet->x, bullet->y, laserReverbe);
                }
                else {
                    bullet->y -= 4;
                    DrawImageCentered(bullet->x, bullet->y, laser);
                }
                if (bullet->y < 0 || bullet->y > FRAME_SY) {
                    bullets[i] = bullets[bullet_count - 1];
                    bullet_count--;
                    i--;
                }
            }
        }
        if (player_life <= 0 && !gameover_sfx) {
            audio_play_sound_clip(gameover);
            gameover_sfx = true;
        }

        //--- Draw Text
        DrawUI(player_shoot_tuto, player_life, font, heart);

        tick_input();

        resize_bitmap(buffer, WINDOW_SIZE_X, WINDOW_SIZE_Y, resizeBuffer, FRAME_SX, FRAME_SY);
        state = mfb_update_ex(window, buffer, WINDOW_SIZE_X, WINDOW_SIZE_Y);
        if (state < 0) {
            window = NULL;
            break;
        }

    } while (mfb_wait_sync(window));
}