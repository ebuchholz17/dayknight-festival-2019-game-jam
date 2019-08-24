#ifndef FISH_GAME_H
#define FISH_GAME_H

#define MAX_NUM_ENEMY_BULLETS 1000

#define BULLET_SPEED 150.0f
#define BULLET_LIFETIME 5.0f
#define BULLET_RADIUS 5.0f
#define NUM_BULLETS_PER_CELL 10

#define PLAYER_RADIUS 7.0f

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

struct fish_game {
    fish_bullet bullets[MAX_NUM_ENEMY_BULLETS];
    int numActiveBullets = 0;

    vector2 playerPos;

    vector2 bossPos;
    float bossSineVal;

    float spawnBulletTimer = 0.0f;
};

#endif
