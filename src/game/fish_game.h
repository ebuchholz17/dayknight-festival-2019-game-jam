#ifndef FISH_GAME_H
#define FISH_GAME_H

#define MAX_NUM_ENEMY_BULLETS 5000

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

enum toy_type {
    TOY_TYPE_NINJA,
    TOY_TYPE_STICKY_HAND,
    TOY_TYPE_BOUNCY_BALL
};

enum sticky_hand_state {
    STICKY_HAND_STATE_REACHING,
    STICKY_HAND_STATE_SLAPPING,
    STICKY_HAND_STATE_RETURNING
};

struct toy_player {
    vector2 pos;
    vector2 velocity;

    toy_type type;

    float rotation;
    float dRotation;
    float bounceTimer;
    bool bouncedOnBoss;
    bool bouncing;

    vector2 handPos;
    bool slapping;
    bool slapped;

    bool shooting;
    float shootingTimer;

    float stickyHandTimer;
    sticky_hand_state stickyHandState;
    vector2 stickyHandStartPos;
    vector2 stickyHandTargetPos;

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

enum boss_attack {
    BOSS_ATTACK_1,
    BOSS_ATTACK_2,
    BOSS_ATTACK_3
};

struct fish_boss {
    vector2 pos;
    float attackTimer;
    float shootingTimer = 0.0f;

    boss_attack currentAttack;

    bool hurt;
    float hurtTimer;

    int hitPoints;

    rectangle hitBoxes[5];
    int numHitboxes;
};

enum fish_game_state {
    FISH_GAME_STATE_TITLE_SCREEN,
    FISH_GAME_STATE_CAPSULE_MACHINE,
    FISH_GAME_STATE_SCENE,
    FISH_GAME_STATE_OPEN_UP,
    FISH_GAME_STATE_BULLET_HELL,
    FISH_GAME_STATE_TRANSITION,
    FISH_GAME_STATE_FISHING,
    FISH_GAME_STATE_END_SCREEN
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

enum fish_game_open_up_state {
    FISH_GAME_OPEN_UP_STATE_DROP_LURE,
    FISH_GAME_OPEN_UP_STATE_REVEAL_LURE,
    FISH_GAME_OPEN_UP_STATE_BOSS_ENTER
};

enum fish_game_end_screen_state {
    FISH_GAME_END_SCREEN_STATE_FADE_OUT,
    FISH_GAME_END_SCREEN_STATE_SHOWING_RESULTS
};

enum fish_game_capsule_machine_state {
    FISH_GAME_CAPSULE_MACHINE_STATE_WAITING,
    FISH_GAME_CAPSULE_MACHINE_STATE_TURNING,
    FISH_GAME_CAPSULE_MACHINE_STATE_FALLING,
    FISH_GAME_CAPSULE_MACHINE_STATE_WAITING_2,
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
    float chargeTargetY;

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

    bool debug = false;

    float fadeOutProgress;

    float shakeTimer;
    fish_shake_direction shakeDirection;

    fish_game_fishing_state fishingState;
    fish_game_end_screen_state endScreenState;
    fish_game_open_up_state openUpState;
    fish_game_capsule_machine_state capsuleMachineState;
    bool won;
};

#endif
