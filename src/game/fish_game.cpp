#include "fish_game.h"

void initFishGame (memory_arena *memory, fish_game* fishGame) {
    //setRNGSeed(0); // TODO(ebuchholz): seed with time?

    *fishGame = {};

    fishGame->playerPos = Vector2(60.0f, 105.0f);
    fishGame->bossPos = Vector2(362.0f, 108.0f);

    for (int i = 0 ; i < MAX_NUM_ENEMY_BULLETS; ++i) {
        fishGame->bullets[i] = {};
    }
    fishGame->numActiveBullets = 0;
    fishGame->spawnBulletTimer = 0.0f;
    fishGame->bossSineVal = 0.0f;
}

inline void spawnBullet (fish_game *fishGame, vector2 spawnPos, vector2 velocity) {
    if (fishGame->numActiveBullets < MAX_NUM_ENEMY_BULLETS) {
        fish_bullet *bullet = fishGame->bullets + fishGame->numActiveBullets;
        ++fishGame->numActiveBullets;

        bullet->pos = spawnPos;
        bullet->velocity = velocity;
        bullet->timeAlive = 0.0f;
    }
}

void updateBoss (fish_game *fishGame) {
    fishGame->bossSineVal += DELTA_TIME;

    fishGame->bossPos.y = 108.0f + sinf(fishGame->bossSineVal * 0.4f) * 75.0f;

    fishGame->spawnBulletTimer += DELTA_TIME;
    float bossSpawnFrequency = 0.4f;
    while (fishGame->spawnBulletTimer > 0.3f) {
        fishGame->spawnBulletTimer -= 0.3f;

        vector2 velocity = BULLET_SPEED * normalize(Vector2(-1.0f, -0.1f));
        vector2 spawnPos = fishGame->bossPos + Vector2(-60.0f, sinf(fishGame->bossSineVal) * 40.0f);
        spawnBullet(fishGame, spawnPos, velocity);
        velocity = BULLET_SPEED * normalize(Vector2(-1.0f, 0.1f));
        spawnPos = fishGame->bossPos + Vector2(-40.0f, -sinf(fishGame->bossSineVal) * 40.0f);
        spawnBullet(fishGame, spawnPos, velocity);
        velocity = BULLET_SPEED * normalize(Vector2(-1.0f, 0.0f));
        spawnPos = fishGame->bossPos + Vector2(-20.0f, 0.0f);
        spawnBullet(fishGame, spawnPos, velocity);
    }
}

inline void checkPointCellOverlap (bullet_cell_grid *bulletCellGrid, int *numCellsCovered, bullet_cell **potentialCells, vector2 corner) {
    int cellCol = (int)((corner.x) / 16.0f);
    int cellRow = (int)((corner.y) / 16.0f);
    bullet_cell *bulletCell = bulletCellGrid->cells + (cellRow * bulletCellGrid->numCols + cellCol);

    bool cellAlreadyAdded = false;
    for (int cellIndex = 0; cellIndex < *numCellsCovered; ++cellIndex) {
        if (bulletCell == potentialCells[cellIndex]) {
            cellAlreadyAdded = true;
            break;
        }
    }
    
    if (!cellAlreadyAdded) {
        potentialCells[*numCellsCovered] = bulletCell;
        ++(*numCellsCovered);
    }
}

void updateBullets (fish_game *fishGame, bullet_cell_grid *bulletCellGrid) {
    bullet_cell *potentialCells[4];
    int numCellsCovered = 0;

    for (int i = 0; i < fishGame->numActiveBullets; ++i) {
        fish_bullet *bullet = fishGame->bullets + i;
        bullet->pos += DELTA_TIME * bullet->velocity;

        bullet->timeAlive += DELTA_TIME;
        bool despawn = false;
        if (bullet->timeAlive > BULLET_LIFETIME) {
            despawn = true;
        }
        else if (bullet->pos.x + BULLET_RADIUS < 0.0f || 
                 bullet->pos.x - BULLET_RADIUS >= 384.0f || 
                 bullet->pos.y + BULLET_RADIUS < 0.0f || 
                 bullet->pos.y - BULLET_RADIUS >= 216.0f) 
        {
            despawn = true;
        }

        if (despawn) {
            fish_bullet *lastBullet = fishGame->bullets + (fishGame->numActiveBullets - 1);
            fishGame->bullets[i] = *lastBullet;
            fishGame->bullets[fishGame->numActiveBullets - 1] = *bullet;

            --fishGame->numActiveBullets;
            --i;
        }
        else {
            // put bullet into grid
            numCellsCovered = 0;

            checkPointCellOverlap(bulletCellGrid, &numCellsCovered, potentialCells, bullet->pos + Vector2(-BULLET_RADIUS, -BULLET_RADIUS));
            checkPointCellOverlap(bulletCellGrid, &numCellsCovered, potentialCells, bullet->pos + Vector2(BULLET_RADIUS, -BULLET_RADIUS));
            checkPointCellOverlap(bulletCellGrid, &numCellsCovered, potentialCells, bullet->pos + Vector2(-BULLET_RADIUS, BULLET_RADIUS));
            checkPointCellOverlap(bulletCellGrid, &numCellsCovered, potentialCells, bullet->pos + Vector2(BULLET_RADIUS, BULLET_RADIUS));

            for (int cellIndex = 0; cellIndex < numCellsCovered; ++cellIndex) {
                bullet_cell *bulletCell = potentialCells[cellIndex];
                if (bulletCell->numBullets < NUM_BULLETS_PER_CELL) {
                    bulletCell->bullets[bulletCell->numBullets] = bullet;
                    ++bulletCell->numBullets;
                }
            }
        }

    }
}

bool checkPlayerBulletCollisions (fish_game *fishGame, bullet_cell_grid *bulletCellGrid) {
    vector2 *playerPos = &fishGame->playerPos;

    bullet_cell *overlappedCells[4];
    int numCellsCovered = 0;
    checkPointCellOverlap(bulletCellGrid, &numCellsCovered, overlappedCells, fishGame->playerPos + Vector2(-PLAYER_RADIUS, -PLAYER_RADIUS));
    checkPointCellOverlap(bulletCellGrid, &numCellsCovered, overlappedCells, fishGame->playerPos + Vector2(PLAYER_RADIUS, -PLAYER_RADIUS));
    checkPointCellOverlap(bulletCellGrid, &numCellsCovered, overlappedCells, fishGame->playerPos + Vector2(-PLAYER_RADIUS, PLAYER_RADIUS));
    checkPointCellOverlap(bulletCellGrid, &numCellsCovered, overlappedCells, fishGame->playerPos + Vector2(PLAYER_RADIUS, PLAYER_RADIUS));

    for (int cellIndex = 0; cellIndex < numCellsCovered; ++cellIndex) {
        bullet_cell *bulletCell = overlappedCells[cellIndex];
        for (int i = 0; i < bulletCell->numBullets; ++i) {
            fish_bullet *bullet = bulletCell->bullets[i];
            float dist = sqrtf(square(bullet->pos.x - playerPos->x) + 
                               square(bullet->pos.y - playerPos->y));
            if (dist < BULLET_RADIUS + PLAYER_RADIUS) {
                return true;
            }
        }
    }
    return false;
}

// game half dims: 192x108

void updateFishGame (memory_arena *memory, memory_arena *tempMemory, game_assets *assets, game_input *input, 
                     fish_game *fishGame, sprite_list *spriteList)
{
    // memory for dynamically created strings
    memory_arena stringMemory = {};
    stringMemory.capacity = 512 * 1024;
    stringMemory.base = allocateMemorySize(tempMemory, stringMemory.capacity);

    // grid partitioning
    bullet_cell_grid bulletCellGrid = {};
    bulletCellGrid.numRows = 14;
    bulletCellGrid.numCols = 24;
    bulletCellGrid.cells = (bullet_cell *)allocateMemorySize(tempMemory, bulletCellGrid.numRows * bulletCellGrid.numCols * sizeof(bullet_cell));

    for (int i = 0; i < bulletCellGrid.numRows; ++i) {
        for (int j = 0; j < bulletCellGrid.numCols; ++j) {
            bullet_cell *bulletCell = bulletCellGrid.cells + (i * bulletCellGrid.numCols + j);
            *bulletCell = {};
        }
    }

    // Move player
    if (input->upKey.down) {
        fishGame->playerPos.y -= 100.0f * DELTA_TIME;
    }
    if (input->downKey.down) {
        fishGame->playerPos.y += 100.0f * DELTA_TIME;
    }
    if (input->leftKey.down) {
        fishGame->playerPos.x -= 100.0f * DELTA_TIME;
    }
    if (input->rightKey.down) {
        fishGame->playerPos.x += 100.0f * DELTA_TIME;
    }

    updateBoss(fishGame);
    updateBullets(fishGame, &bulletCellGrid);

    bool playerHitByBullet = checkPlayerBulletCollisions(fishGame, &bulletCellGrid);

    // debug drawing of how many bullets in each cell
    for (int i = 0; i < bulletCellGrid.numRows; ++i) {
        for (int j = 0; j < bulletCellGrid.numCols; ++j) {
            bullet_cell *bulletCell = bulletCellGrid.cells + (i * bulletCellGrid.numCols + j);
            addText(j * 16.0f + 4.0f, i * 16.0f + 4.0f, numToString(bulletCell->numBullets, &stringMemory), assets, TEXTURE_KEY_FONT, spriteList);
        }
    }

    //pushSpriteTransform(spriteList, Vector2(GAME_WIDTH/2.0f, GAME_HEIGHT/2.0f));

    if (playerHitByBullet) {
        addSprite(fishGame->playerPos.x, fishGame->playerPos.y, assets, TEXTURE_KEY_PLAYER_HURT, spriteList, 0.5f, 0.5f);
    }
    else {
        addSprite(fishGame->playerPos.x, fishGame->playerPos.y, assets, ATLAS_KEY_GAME, "player", spriteList, 0.5f, 0.5f);
    }
    addSprite(fishGame->bossPos.x, fishGame->bossPos.y, assets, ATLAS_KEY_GAME, "boss", spriteList, 0.5f, 0.5f);

    for (int i = 0; i < fishGame->numActiveBullets; ++i) {
        fish_bullet *bullet = fishGame->bullets + i;
        addSprite(bullet->pos.x, bullet->pos.y, assets, TEXTURE_KEY_BULLET, spriteList, 0.5f, 0.5f);
    }

    //popSpriteMatrix(spriteList);
}
