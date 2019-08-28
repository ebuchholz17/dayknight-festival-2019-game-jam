#include "fish_game.h"

void initBoss (fish_game *fishGame) {
    fish_boss *boss = &fishGame->boss;
    boss->numHitboxes = 0;
    switch (fishGame->bossType) {
        default:
        case BOSS_TYPE_BASS: {
            boss->hitPoints = 250;

            // relative to boss center
            rectangle hitBox;
            hitBox.min = Vector2(-92.0f, -35.0f);
            hitBox.max = Vector2(92.0f, 35.0f);
            boss->hitBoxes[boss->numHitboxes++] = hitBox;
            hitBox.min = Vector2(-75.0f, -50.0f);
            hitBox.max = Vector2(75.0f, 50.0f);
            boss->hitBoxes[boss->numHitboxes++] = hitBox;
            hitBox.min = Vector2(-50.0f, -58.0f);
            hitBox.max = Vector2(50.0f, 58.0f);
            boss->hitBoxes[boss->numHitboxes++] = hitBox;
        } break;
    }
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

inline void spawnPlayerBullet (fish_game *fishGame, vector2 spawnPos, vector2 velocity) {
    if (fishGame->numActivePlayerBullets < MAX_NUM_PLAYER_BULLETS) {
        fish_bullet *bullet = fishGame->playerBullets + fishGame->numActivePlayerBullets;
        ++fishGame->numActivePlayerBullets;

        bullet->pos = spawnPos;
        bullet->velocity = velocity;
        bullet->timeAlive = 0.0f;
    }
}

void updateBoss (fish_game *fishGame) {
    fishGame->boss.sineVal += DELTA_TIME;

    fishGame->boss.pos.y = 108.0f + sinf(fishGame->boss.sineVal * 0.4f) * 75.0f;

    fishGame->boss.shootingTimer += DELTA_TIME;
    float bossSpawnFrequency = 0.4f;
    while (fishGame->boss.shootingTimer > 0.3f) {
        fishGame->boss.shootingTimer -= 0.3f;

        vector2 velocity = BULLET_SPEED * normalize(Vector2(-1.0f, -0.1f));
        vector2 spawnPos = fishGame->boss.pos + Vector2(-60.0f, sinf(fishGame->boss.sineVal) * 40.0f);
        spawnBullet(fishGame, spawnPos, velocity);
        velocity = BULLET_SPEED * normalize(Vector2(-1.0f, 0.1f));
        spawnPos = fishGame->boss.pos + Vector2(-40.0f, -sinf(fishGame->boss.sineVal) * 40.0f);
        spawnBullet(fishGame, spawnPos, velocity);
        velocity = BULLET_SPEED * normalize(Vector2(-1.0f, 0.0f));
        spawnPos = fishGame->boss.pos + Vector2(-20.0f, 0.0f);
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

void updatePlayer (fish_game *fishGame, game_input *input) {
    toy_player *player = &fishGame->player;
    if (input->upKey.down) {
        player->pos.y -= 100.0f * DELTA_TIME;
    }
    if (input->downKey.down) {
        player->pos.y += 100.0f * DELTA_TIME;
    }
    if (input->leftKey.down) {
        player->pos.x -= 100.0f * DELTA_TIME;
    }
    if (input->rightKey.down) {
        player->pos.x += 100.0f * DELTA_TIME;
    }

    if (player->pos.x - PLAYER_RADIUS < 0.0f) {
        player->pos.x = PLAYER_RADIUS;
    }
    if (player->pos.x + PLAYER_RADIUS >= 384.0f) {
        player->pos.x = 384.0f - PLAYER_RADIUS;
    }
    if (player->pos.y - PLAYER_RADIUS < 0.0f) {
        player->pos.y = PLAYER_RADIUS;
    }
    if (player->pos.y + PLAYER_RADIUS >= 216.0f) {
        player->pos.y = 216.0f - PLAYER_RADIUS;
    }

    if (input->zKey.down) {
        player->shooting = true;
    }
    else {
        player->shooting = false;
    }
    if (input->xKey.down) {
        //player->pos.x -= 3000.0f * DELTA_TIME;
    }

    player->shootingTimer += DELTA_TIME;
    if (player->shooting) {
        if (player->shootingTimer > PLAYER_SHOOTING_FREQUENCY) {
            player->shootingTimer = 0.0f;

            vector2 velocity = PLAYER_BULLET_SPEED * normalize(Vector2(1.0f, 0.0f));
            vector2 spawnPos = player->pos + Vector2(3.0f, 0.0f);
            spawnPlayerBullet(fishGame, spawnPos, velocity);
        }
    }
}

inline void despawnPlayerBullet(fish_game *fishGame, fish_bullet *bullet, int index) {
    fish_bullet *lastBullet = fishGame->playerBullets + (fishGame->numActivePlayerBullets - 1);
    fishGame->playerBullets[index] = *lastBullet;
    fishGame->playerBullets[fishGame->numActivePlayerBullets - 1] = *bullet;

    --fishGame->numActivePlayerBullets;
}

inline void despawnPlayerBullet(fish_game *fishGame, int index) {
    fish_bullet *bullet = fishGame->playerBullets + index;
    despawnPlayerBullet(fishGame, bullet, index);
}

void updatePlayerBullets (fish_game *fishGame) {
    for (int i = 0; i < fishGame->numActivePlayerBullets; ++i) {
        fish_bullet *bullet = fishGame->playerBullets + i;
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
            despawnPlayerBullet(fishGame, bullet, i);
            --i;
        }
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
    vector2 playerPos = fishGame->player.pos;

    bullet_cell *overlappedCells[4];
    int numCellsCovered = 0;
    checkPointCellOverlap(bulletCellGrid, &numCellsCovered, overlappedCells, playerPos + Vector2(-PLAYER_RADIUS, -PLAYER_RADIUS));
    checkPointCellOverlap(bulletCellGrid, &numCellsCovered, overlappedCells, playerPos + Vector2(PLAYER_RADIUS, -PLAYER_RADIUS));
    checkPointCellOverlap(bulletCellGrid, &numCellsCovered, overlappedCells, playerPos + Vector2(-PLAYER_RADIUS, PLAYER_RADIUS));
    checkPointCellOverlap(bulletCellGrid, &numCellsCovered, overlappedCells, playerPos + Vector2(PLAYER_RADIUS, PLAYER_RADIUS));

    for (int cellIndex = 0; cellIndex < numCellsCovered; ++cellIndex) {
        bullet_cell *bulletCell = overlappedCells[cellIndex];
        for (int i = 0; i < bulletCell->numBullets; ++i) {
            fish_bullet *bullet = bulletCell->bullets[i];
            float dist = sqrtf(square(bullet->pos.x - playerPos.x) + 
                               square(bullet->pos.y - playerPos.y));
            if (dist < BULLET_RADIUS + PLAYER_RADIUS) {
                return true;
            }
        }
    }
    return false;
}

bool checkBulletBossCollisions (fish_game *fishGame, int *bulletDespawnIndex) {
    fish_boss *boss = &fishGame->boss;
    int numHitboxes = boss->numHitboxes;
    rectangle *hitBoxes = boss->hitBoxes;

    for (int i = 0; i < fishGame->numActivePlayerBullets; ++i) {
        fish_bullet *bullet = fishGame->playerBullets + i;

        for (int j = 0; j < numHitboxes; ++j) {
            rectangle bulletRect;
            bulletRect.min = bullet->pos + Vector2(-BULLET_RADIUS, -BULLET_RADIUS);
            bulletRect.min = bulletRect.min - boss->pos;
            bulletRect.max = bullet->pos + Vector2(BULLET_RADIUS, BULLET_RADIUS);
            bulletRect.max = bulletRect.max - boss->pos;

            if (rectangleIntersection(bulletRect, hitBoxes[j])) {
                *bulletDespawnIndex = i;
                return true;
            }
        }
    }
    return false;
}

bool checkPlayerBossCollision (fish_game *fishGame) {
    fish_boss *boss = &fishGame->boss;
    int numHitboxes = boss->numHitboxes;
    rectangle *hitBoxes = boss->hitBoxes;

    toy_player *player = &fishGame->player;

    for (int j = 0; j < numHitboxes; ++j) {
        rectangle playerRect;
        playerRect.min = player->pos + Vector2(-PLAYER_RADIUS, -PLAYER_RADIUS);
        playerRect.min = playerRect.min - boss->pos;
        playerRect.max = player->pos + Vector2(PLAYER_RADIUS, PLAYER_RADIUS);
        playerRect.max = playerRect.max - boss->pos;

        if (rectangleIntersection(playerRect, hitBoxes[j])) {
            return true;
        }
    }

    return false;
}

// game half dims: 192x108
void initFishGame (memory_arena *memory, fish_game* fishGame) {
    //setRNGSeed(0); // TODO(ebuchholz): seed with time?

    *fishGame = {};

    //fishGame->gameState = FISH_GAME_STATE_BULLET_HELL;
    fishGame->gameState = FISH_GAME_STATE_TITLE_SCREEN;

    toy_player *player = &fishGame->player;
    player->pos = Vector2(60.0f, 105.0f);
    player->hitPoints = 8;
    //player->hitPoints = 1;
    for (int i = 0 ; i < MAX_NUM_PLAYER_BULLETS; ++i) {
        fishGame->playerBullets[i] = {};
    }
    fishGame->numActivePlayerBullets = 0;

    fishGame->boss.pos = Vector2(362.0f, 108.0f);
    fishGame->bossType = BOSS_TYPE_BASS;
    initBoss(fishGame);

    for (int i = 0 ; i < MAX_NUM_ENEMY_BULLETS; ++i) {
        fishGame->bullets[i] = {};
    }
    fishGame->numActiveBullets = 0;
    fishGame->boss.shootingTimer = 0.0f;
    fishGame->boss.sineVal = 0.0f;

    fishGame->meterChunkY = 0.0f;
    fishGame->meterChunkDY = 0.0f;
    fishGame->meterChunkHeight = 70.0f;
    fishGame->meterFishY = 100.0f;
    fishGame->meterProgress = 75.0f;

    fishGame->currentStrugglePosIndex = 0;
    fishGame->struggleTimer = 0.0f;

    fishGame->shakeTimer = 0.0f;
    fishGame->shakeDirection = FISH_SHAKE_DIRECTION_LEFT;

    fishGame->firstStruggle = true;
}

void updateBulletHell(memory_arena *memory, memory_arena *tempMemory, game_assets *assets, game_input *input, 
                      fish_game *fishGame, sprite_list *spriteList)
{
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

    updatePlayer(fishGame, input);
    updatePlayerBullets(fishGame);

    updateBoss(fishGame);
    updateBullets(fishGame, &bulletCellGrid);

    toy_player *player = &fishGame->player;
    if (!player->hurt) {
        bool playerHitByBullet = checkPlayerBulletCollisions(fishGame, &bulletCellGrid);
        if (checkPlayerBulletCollisions(fishGame, &bulletCellGrid) ||
            checkPlayerBossCollision(fishGame)) 
        {
            player->hurt = true;
            player->hurtFlash = true;
            --player->hitPoints;
        }
    }

    if (player->hurt) {
        player->invincibilityTimer += DELTA_TIME;
        if (player->invincibilityTimer >= PLAYER_INVINCIBILITY_TIMER) {
            player->hurt = false;
            player->invincibilityTimer = 0.0f;
            player->invincibilityFlashTimer = 0.0f;
            player->hurtFlash = false;
        }
        else {
            player->invincibilityFlashTimer += DELTA_TIME;
            if (player->invincibilityFlashTimer >= PLAYER_HURT_FLASH_TIMER) {
                player->invincibilityFlashTimer -=  PLAYER_HURT_FLASH_TIMER;
                player->hurtFlash = !player->hurtFlash;
            }
        }
    }

    int bulletDespawnIndex;
    if (checkBulletBossCollisions(fishGame, &bulletDespawnIndex)) {
        fishGame->boss.hurt = true;
        fishGame->boss.hurtTimer = 0.0f;
        --fishGame->boss.hitPoints;
        despawnPlayerBullet(fishGame, bulletDespawnIndex);
    }

    if (fishGame->boss.hurt) {
        fishGame->boss.hurtTimer += DELTA_TIME;
        if (fishGame->boss.hurtTimer >= BOSS_HURT_TIME) {
            fishGame->boss.hurt = false;
        }
    }

    if (fishGame->player.hitPoints <= 0 || fishGame->boss.hitPoints <= 0) {
        if (fishGame->boss.hitPoints <= 0) {
            fishGame->meterChunkHeight += 50.0f;
        }
        fishGame->playerTransitionStartPos = fishGame->player.pos;
        fishGame->bossTransitionStartPos = fishGame->boss.pos;
        fishGame->gameState = FISH_GAME_STATE_TRANSITION;
        fishGame->transitionT = 0.0f;
        fishGame->transitionState = FISH_GAME_TRANSITION_STATE_MOVING;
    }

    // debug drawing of how many bullets in each cell
    //for (int i = 0; i < bulletCellGrid.numRows; ++i) {
    //    for (int j = 0; j < bulletCellGrid.numCols; ++j) {
    //        bullet_cell *bulletCell = bulletCellGrid.cells + (i * bulletCellGrid.numCols + j);
    //        addText(j * 16.0f + 4.0f, i * 16.0f + 4.0f, numToString(bulletCell->numBullets, &stringMemory), assets, TEXTURE_KEY_FONT, spriteList);
    //    }
    //}

    //pushSpriteTransform(spriteList, Vector2(GAME_WIDTH/2.0f, GAME_HEIGHT/2.0f));

    addSprite(0.0f, 0.0f, assets, TEXTURE_KEY_BG, spriteList, 0.0f, 0.0f);

    addSprite(fishGame->player.pos.x, fishGame->player.pos.y, assets, TEXTURE_KEY_LINE, spriteList, 1.0f, 1.0f);
    if (player->hurtFlash) {
        addSprite(fishGame->player.pos.x, fishGame->player.pos.y, assets, ATLAS_KEY_GAME, "player", spriteList, 0.5f, 0.5f, 1.0f, 0.0f, 0.3f);
    }
    else {
        addSprite(fishGame->player.pos.x, fishGame->player.pos.y, assets, ATLAS_KEY_GAME, "player", spriteList, 0.5f, 0.5f);
    }
    if (fishGame->boss.hurt) {
        addSprite(fishGame->boss.pos.x, fishGame->boss.pos.y, assets, TEXTURE_KEY_BASS, spriteList, 0.5f, 0.5f, 1.0f, 0.0f, 0.3f);
    }
    else {
        addSprite(fishGame->boss.pos.x, fishGame->boss.pos.y, assets, TEXTURE_KEY_BASS, spriteList, 0.5f, 0.5f);
    }

    for (int i = 0; i < fishGame->numActiveBullets; ++i) {
        fish_bullet *bullet = fishGame->bullets + i;
        addSprite(bullet->pos.x, bullet->pos.y, assets, TEXTURE_KEY_BULLET, spriteList, 0.5f, 0.5f);
    }
    for (int i = 0; i < fishGame->numActivePlayerBullets; ++i) {
        fish_bullet *bullet = fishGame->playerBullets + i;
        addSprite(bullet->pos.x, bullet->pos.y, assets, TEXTURE_KEY_PLAYER_BULLET, spriteList, 0.5f, 0.5f);
    }

    for (int i = 0; i < player->hitPoints; ++i) {
        addSprite(8.0f + 8.0f * i, 200.0f, assets, TEXTURE_KEY_YELLOW_BOX, spriteList);
    }
    for (int i = 0; i < fishGame->boss.hitPoints; ++i) {
        addSprite(368.0f - 0.5f * i, 200.0f, assets, TEXTURE_KEY_GREEN_BOX, spriteList);
    }

    // debugging for boss hitboxes
    //int numHitboxes = fishGame->boss.numHitboxes;
    //rectangle *hitBoxes = fishGame->boss.hitBoxes;
    //pushSpriteTransform(spriteList, fishGame->boss.pos);
    //for (int i = 0; i < numHitboxes; ++i) {
    //    rectangle hitBox = hitBoxes[i];
    //    vector2 rectDims = hitBox.max - hitBox.min;
    //    // white box texture is 4x4 px
    //    vector2 whiteBoxScale = rectDims * (1.0f / 4.0f);
    //    matrix3x3 boxTransform = scaleMatrix3x3(whiteBoxScale.x, whiteBoxScale.y);
    //    boxTransform = translationMatrix(hitBox.min.x, hitBox.min.y) * boxTransform;
    //    pushSpriteMatrix(boxTransform, spriteList);
    //    addSprite(0.0f, 0.0f, assets, TEXTURE_KEY_WHITE_BOX, spriteList, 0.0f, 0.0f, 1.0f, 0.0f, 0.5f);
    //    popSpriteMatrix(spriteList);
    //}
    //popSpriteMatrix(spriteList);
}

void drawMeter (game_assets *assets, fish_game *fishGame, sprite_list *spriteList, float offset = 0.0f) {
    pushSpriteTransform(spriteList, Vector2(offset, 0.0f));

    float meterScale = fishGame->meterChunkHeight * (1.0f / 8.0f);
    matrix3x3 meterChunkTransform = scaleMatrix3x3(1.0f, meterScale);
    meterChunkTransform = translationMatrix(8.0f, 208.0f - fishGame->meterChunkY) * meterChunkTransform;
    pushSpriteMatrix(meterChunkTransform, spriteList);
    addSprite(0.0f, 0.0f, assets, TEXTURE_KEY_GREEN_BOX, spriteList, 0.0f, 1.0f);
    popSpriteMatrix(spriteList);

    addSprite(8.0f, 208.0f - fishGame->meterFishY, assets, TEXTURE_KEY_METER_FISH, spriteList, 0.0f, 0.5f);

    float progressScale = fishGame->meterProgress * (1.0f / 8.0f);
    matrix3x3 meterProgressTransform = scaleMatrix3x3(1.0f, progressScale);
    meterProgressTransform = translationMatrix(18.0f, 208.0f) * meterProgressTransform;
    pushSpriteMatrix(meterProgressTransform, spriteList);
    addSprite(0.0f, 0.0f, assets, TEXTURE_KEY_YELLOW_BOX, spriteList, 0.0f, 1.0f);
    popSpriteMatrix(spriteList);

    addSprite(7.0f, 7.0f, assets, TEXTURE_KEY_FISH_METER, spriteList);

    popSpriteMatrix(spriteList);
}

void updateFishingGame(memory_arena *memory, memory_arena *tempMemory, game_assets *assets, game_input *input, 
                       fish_game *fishGame, sprite_list *spriteList)
{
    if (input->zKey.down) {
        fishGame->meterChunkDY += 700.0f * DELTA_TIME;
    }
    fishGame->meterChunkDY -= 300.0f * DELTA_TIME;
    if (fishGame->meterChunkDY > 3000.0f) {
        fishGame->meterChunkDY = 300.0f;
    }
    if (fishGame->meterChunkDY < -3000.0f) {
        fishGame->meterChunkDY = -3000.0f;
    }
    
    fishGame->meterChunkY += fishGame->meterChunkDY * DELTA_TIME;
    if (fishGame->meterChunkY < 0.0f) {
        fishGame->meterChunkY = 0.0f;
        if (fishGame->meterChunkDY < 0.0f) {
            fishGame->meterChunkDY = 0.0f;
        }
    }
    if (fishGame->meterChunkY > 200.0f - fishGame->meterChunkHeight) {
        fishGame->meterChunkY = 200.0f - fishGame->meterChunkHeight;
        if (fishGame->meterChunkDY > 0.0f) {
            fishGame->meterChunkDY = 0.0f;
        }
    }

    fishGame->meterSineVal += DELTA_TIME;
    fishGame->meterFishY = 100.0f + sinf(fishGame->meterSineVal) * 50.0f;
    if (fishGame->meterFishY < 0.0f) {
        fishGame->meterFishY = 0.0f;
    }
    if (fishGame->meterFishY > 200.0f) {
        fishGame->meterFishY = 200.0f;
    }

    if (fishGame->meterFishY >= fishGame->meterChunkY &&
        fishGame->meterFishY <= fishGame->meterChunkY + fishGame->meterChunkHeight) 
    {
        fishGame->meterProgress += 15 * DELTA_TIME;
    }
    else {
        fishGame->meterProgress -= 25 * DELTA_TIME;
    }
    if (fishGame->meterProgress < 0.0f) {
        fishGame->meterProgress = 0.0f;
    }
    if (fishGame->meterProgress > 200.0f) {
        fishGame->meterProgress = 200.0f;
    }

    vector2 strugglePositions[4];
    strugglePositions[0] = Vector2(190.0f, 60.0f);
    strugglePositions[1] = Vector2(280.0f, 60.0f);
    strugglePositions[2] = Vector2(280.0f, 140.0f);
    strugglePositions[3] = Vector2(190.0f, 140.0f);

    fishGame->struggleTimer += DELTA_TIME;
    if (fishGame->struggleTimer < 1.0f) {
        fishGame->struggleT += DELTA_TIME;
        int nextStruggleIndex = (fishGame->currentStrugglePosIndex + 1) % 4;

        vector2 lastPos;
        if (fishGame->firstStruggle) {
            lastPos = fishGame->bossTransitionStartPos;
        }
        else {
            lastPos = strugglePositions[fishGame->currentStrugglePosIndex];
        }
        vector2 nextPos = strugglePositions[nextStruggleIndex];

        fishGame->boss.pos = lastPos + fishGame->struggleT * (nextPos - lastPos);
    }
    else if (fishGame->struggleTimer < 3.0f) {
        fishGame->shakeTimer += DELTA_TIME;
        if (fishGame->shakeTimer >= 0.08f) {
            fishGame->shakeTimer -= 0.08f;
            if (fishGame->shakeDirection == FISH_SHAKE_DIRECTION_LEFT) {
                fishGame->shakeDirection = FISH_SHAKE_DIRECTION_RIGHT;
            }
            else {
                fishGame->shakeDirection = FISH_SHAKE_DIRECTION_LEFT;
            }
        }
        vector2 offset;
        if (fishGame->shakeDirection == FISH_SHAKE_DIRECTION_LEFT) {
            offset = Vector2(-10.0f, 0.0f);
        }
        else {
            offset = Vector2(10.0f, 0.0f);
        }
        vector2 currentPos = strugglePositions[(fishGame->currentStrugglePosIndex + 1) % 4];
        fishGame->boss.pos = currentPos + (fishGame->shakeTimer / 0.05f) * offset;
    }
    else {
        fishGame->currentStrugglePosIndex = 
            (fishGame->currentStrugglePosIndex + 1) % 4;
        fishGame->boss.pos = strugglePositions[fishGame->currentStrugglePosIndex];
        fishGame->struggleT = 0.0f;
        fishGame->shakeTimer = 0.0f;
        fishGame->struggleTimer -= 3.0f;
        fishGame->firstStruggle = false;
    }

    addSprite(0.0f, 0.0f, assets, TEXTURE_KEY_BG, spriteList, 0.0f, 0.0f);

    vector2 linePos = fishGame->boss.pos - Vector2(60.0f, 0.0f);
    addSprite(linePos.x, linePos.y, assets, TEXTURE_KEY_LINE, spriteList, 1.0f, 1.0f);
    addSprite(fishGame->boss.pos.x, fishGame->boss.pos.y, assets, TEXTURE_KEY_BASS, spriteList, 0.5f, 0.5f);

    drawMeter(assets, fishGame, spriteList);
}

void updateTransition(memory_arena *memory, memory_arena *tempMemory, game_assets *assets, game_input *input, 
                      fish_game *fishGame, sprite_list *spriteList)
{
    // keep bullets on screen but stop spawning them
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

    updatePlayerBullets(fishGame);
    updateBullets(fishGame, &bulletCellGrid);

    float meterOffset = 0.0f;
    switch (fishGame->transitionState) {
        case FISH_GAME_TRANSITION_STATE_MOVING: {
            vector2 center = Vector2(192.0f, 108.0f);
            vector2 rightCenter = Vector2(384.0f, 108.0f);

            fishGame->transitionT += DELTA_TIME;
            if (fishGame->transitionT >= 1.0f) {
                fishGame->transitionT = 1.0f;
            }
            fishGame->player.pos = fishGame->playerTransitionStartPos + 
                                   fishGame->transitionT * (center - fishGame->playerTransitionStartPos);
            fishGame->boss.pos = fishGame->bossTransitionStartPos + 
                                 fishGame->transitionT * (rightCenter - fishGame->bossTransitionStartPos);
            if (fishGame->transitionT >= 1.0f) {
                fishGame->transitionState = FISH_GAME_TRANSITION_STATE_EATING;
                fishGame->transitionT = 0.0f;
            }

            meterOffset = (1.0f - fishGame->transitionT) * -30.0f;
        } break;
        case FISH_GAME_TRANSITION_STATE_EATING: {
            vector2 fishTarget = Vector2(192.0f + 80.0f, 108.0f);
            vector2 rightCenter = Vector2(384.0f, 108.0f);

            fishGame->transitionT += DELTA_TIME * 4.0f;
            if (fishGame->transitionT >= 1.0f) {
                fishGame->transitionT = 1.0f;
            }

            fishGame->boss.pos = rightCenter + 
                                 fishGame->transitionT * (fishTarget - rightCenter);
            if (fishGame->transitionT >= 1.0f) {
                fishGame->transitionState = FISH_GAME_TRANSITION_STATE_SIZING_METERS;
                fishGame->transitionT = 0.0f;
            }

        } break;
        case FISH_GAME_TRANSITION_STATE_SIZING_METERS: {
           int hpToDrain = (int)(10.0f * DELTA_TIME);
           if (hpToDrain < 1) {
               hpToDrain = 1;
           }
           fishGame->boss.hitPoints -= hpToDrain;
           if (fishGame->boss.hitPoints <= 0) {
               fishGame->boss.hitPoints -= hpToDrain;
           }

           fishGame->meterChunkHeight -= 35.0f * DELTA_TIME;
           if (fishGame->meterChunkHeight < 10.0f) {
               fishGame->meterChunkHeight = 10.0f;
           }

           if (fishGame->boss.hitPoints <= 0) {
               fishGame->gameState = FISH_GAME_STATE_FISHING;
               fishGame->bossTransitionStartPos = fishGame->boss.pos;
           }

        } break;
    }

    addSprite(0.0f, 0.0f, assets, TEXTURE_KEY_BG, spriteList, 0.0f, 0.0f);

    addSprite(fishGame->player.pos.x, fishGame->player.pos.y, assets, TEXTURE_KEY_LINE, spriteList, 1.0f, 1.0f);
    addSprite(fishGame->player.pos.x, fishGame->player.pos.y, assets, ATLAS_KEY_GAME, "player", spriteList, 0.5f, 0.5f);
    addSprite(fishGame->boss.pos.x, fishGame->boss.pos.y, assets, TEXTURE_KEY_BASS, spriteList, 0.5f, 0.5f);

    for (int i = 0; i < fishGame->numActiveBullets; ++i) {
        fish_bullet *bullet = fishGame->bullets + i;
        addSprite(bullet->pos.x, bullet->pos.y, assets, TEXTURE_KEY_BULLET, spriteList, 0.5f, 0.5f);
    }
    for (int i = 0; i < fishGame->numActivePlayerBullets; ++i) {
        fish_bullet *bullet = fishGame->playerBullets + i;
        addSprite(bullet->pos.x, bullet->pos.y, assets, TEXTURE_KEY_PLAYER_BULLET, spriteList, 0.5f, 0.5f);
    }

    for (int i = 0; i < fishGame->player.hitPoints; ++i) {
        addSprite(8.0f + 8.0f * i, 200.0f, assets, TEXTURE_KEY_YELLOW_BOX, spriteList);
    }
    for (int i = 0; i < fishGame->boss.hitPoints; ++i) {
        addSprite(368.0f - 0.5f * i, 200.0f, assets, TEXTURE_KEY_GREEN_BOX, spriteList);
    }

    drawMeter(assets, fishGame, spriteList, meterOffset);
}

void updateTitleScreen(memory_arena *memory, memory_arena *tempMemory, game_assets *assets, game_input *input, 
                       fish_game *fishGame, sprite_list *spriteList)
{
    if (input->zKey.down) {
        fishGame->gameState = FISH_GAME_STATE_BULLET_HELL;
    }
    addText(8.0f, 8.0f, "FISH GAME", assets, TEXTURE_KEY_FONT, spriteList);
    addText(8.0f, 24.0f, "Controls: Z, Arrow Keys", assets, TEXTURE_KEY_FONT, spriteList);
    addText(8.0f, 40.0f, "Press Z to Start", assets, TEXTURE_KEY_FONT, spriteList);
}

void updateFishGame (memory_arena *memory, memory_arena *tempMemory, game_assets *assets, game_input *input, 
                     fish_game *fishGame, sprite_list *spriteList)
{
    // memory for dynamically created strings
    memory_arena stringMemory = {};
    stringMemory.capacity = 512 * 1024;
    stringMemory.base = allocateMemorySize(tempMemory, stringMemory.capacity);

    switch (fishGame->gameState) {
        case FISH_GAME_STATE_TITLE_SCREEN: {
            updateTitleScreen(memory, tempMemory, assets, input, fishGame, spriteList);
        } break;
        case FISH_GAME_STATE_BULLET_HELL: {
            updateBulletHell(memory, tempMemory, assets, input, fishGame, spriteList);
        } break;
        case FISH_GAME_STATE_TRANSITION: {
            updateTransition(memory, tempMemory, assets, input, fishGame, spriteList);
        } break;
        case FISH_GAME_STATE_FISHING: {
            updateFishingGame(memory, tempMemory, assets, input, fishGame, spriteList);
        } break;
    }

}
