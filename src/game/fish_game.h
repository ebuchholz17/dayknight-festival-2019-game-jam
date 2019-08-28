#ifndef FISH_GAME_H
#define FISH_GAME_H

#define MAX_NUM_ENEMY_BULLETS 1000

#define BULLET_SPEED 150.0f
#define BULLET_LIFETIME 5.0f
#define BULLET_RADIUS 5.0f
#define NUM_BULLETS_PER_CELL 10

#define BOSS_HURT_TIME 0.09f

#define PLAYER_RADIUS 7.0f
#define MAX_NUM_PLAYER_BULLETS 20
#define PLAYER_SHOOTING_FREQUENCY 0.3f
#define PLAYER_BULLET_SPEED 250.0f

#define PLAYER_INVINCIBILITY_TIMER 1.0f
#define PLAYER_HURT_FLASH_TIMER 0.09f

#define FISH_METER_CAPACITY 200.0f

struct fish_bullet {
    vector2 pos;
    vector2 velocity;
    float timeAlive;
};

struct bullet_cell {
    fish_bullet *bullets[NUM_BULLETS_PER_CELL];
    int numBullets;
};

struct bullet_cell_grid{
    bullet_cell *cells;
    int numRows;
    int numCols;
};

struct toy_player {
    vector2 pos;

    bool shooting;
    float shootingTimer;

    int hitPoints;

    bool hurt;
    bool hurtFlash;
    float invincibilityTimer;
    float invincibilityFlashTimer;
};

enum boss_type {
    BOSS_TYPE_BASS,
    BOSS_TYPE_CAR,
    BOSS_TYPE_COUNT
};

struct fish_boss {
    vector2 pos;
    float sineVal;
    float shootingTimer = 0.0f;

    bool hurt;
    float hurtTimer;

    int hitPoints;

    rectangle hitBoxes[5];
    int numHitboxes;
};

enum fish_game_state {
    FISH_GAME_STATE_TITLE_SCREEN,
    FISH_GAME_STATE_BULLET_HELL,
    FISH_GAME_STATE_TRANSITION,
    FISH_GAME_STATE_FISHING
};

enum fish_game_transition_state {
    FISH_GAME_TRANSITION_STATE_MOVING,
    FISH_GAME_TRANSITION_STATE_EATING,
    FISH_GAME_TRANSITION_STATE_SIZING_METERS
};

enum fish_game_fishing_state {
    FISH_GAME_FISHING_STATE_FISHING,
    FISH_GAME_FISHING_STATE_FADE_OUT
};

enum fish_shake_direction {
    FISH_SHAKE_DIRECTION_LEFT,
    FISH_SHAKE_DIRECTION_RIGHT
};

struct fish_game {
    fish_bullet bullets[MAX_NUM_ENEMY_BULLETS];
    int numActiveBullets = 0;

    fish_bullet playerBullets[MAX_NUM_PLAYER_BULLETS];
    int numActivePlayerBullets = 0;

    fish_game_state gameState;
    float transitionT;
    fish_game_transition_state transitionState;

    toy_player player;
    fish_boss boss;
    boss_type bossType;

    float meterSineVal;
    float meterChunkY;
    float meterChunkDY;
    float meterChunkHeight;
    float meterFishY;
    float meterProgress;

    vector2 playerTransitionStartPos;
    vector2 bossTransitionStartPos;

    int currentStrugglePosIndex;
    float struggleT;
    float struggleTimer;
    bool firstStruggle;

    float shakeTimer;
    fish_shake_direction shakeDirection;
};

#endif
