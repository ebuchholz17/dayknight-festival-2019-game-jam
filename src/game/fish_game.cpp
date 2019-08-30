#include "fish_game.h"

void updateTitleScreen(memory_arena *memory, memory_arena *tempMemory, game_assets *assets, game_input *input, 
                       fish_game *fishGame, sprite_list *spriteList)
{
    if (input->zKey.justPressed || input->pointerJustDown) {
        fishGame->gameState = FISH_GAME_STATE_CAPSULE_MACHINE;
        fishGame->capsuleMachineState = FISH_GAME_CAPSULE_MACHINE_STATE_WAITING;
        fishGame->transitionT = 0.0f;
    }
    addText(92.0f, 56.0f, "Championship Fishing 2019", assets, TEXTURE_KEY_FONT, spriteList);
    addText(100.0f, 72.0f, "Controls: Z, Arrow Keys", assets, TEXTURE_KEY_FONT, spriteList);
    addText(128.0f, 88.0f, "Press Z to Start", assets, TEXTURE_KEY_FONT, spriteList);
}

char *getCapsuleFrame (fish_game *fishGame) {
    char *capsuleColor = "";
    switch (fishGame->player.type) {
        case TOY_TYPE_NINJA: {
            capsuleColor = "capsule_bottom_green";
        } break;
        case TOY_TYPE_STICKY_HAND: {
            capsuleColor = "capsule_bottom_blue";
        } break;
        case TOY_TYPE_BOUNCY_BALL: {
            capsuleColor = "capsule_bottom_red";
        } break;
    }
    return capsuleColor;
}

void updateCapsuleMachine(memory_arena *memory, memory_arena *tempMemory, game_assets *assets, game_input *input, 
                          fish_game *fishGame, sprite_list *spriteList)
{
    fishGame->transitionT += DELTA_TIME;
    float handleRotation = 0.0f;
    float capsuleY = 108.0f;
    switch (fishGame->capsuleMachineState) {
        case FISH_GAME_CAPSULE_MACHINE_STATE_WAITING: {
            if (fishGame->transitionT > 0.5f) {
                fishGame->transitionT = 0.0f;
                fishGame->capsuleMachineState = FISH_GAME_CAPSULE_MACHINE_STATE_TURNING;
            }
        } break;
        case FISH_GAME_CAPSULE_MACHINE_STATE_TURNING: {
            float t = fishGame->transitionT;
            handleRotation = 2.0f * PI * t;
            if (fishGame->transitionT > 1.0f) {
                fishGame->transitionT = 0.0f;
                fishGame->capsuleMachineState = FISH_GAME_CAPSULE_MACHINE_STATE_FALLING;
            }
        } break;
        case FISH_GAME_CAPSULE_MACHINE_STATE_FALLING: {
            float t = fishGame->transitionT / 0.5f;
            float capsuleStartY = 88.0f;
            float capsuleEndY = 166.0f;
            capsuleY = capsuleStartY + t * (capsuleEndY - capsuleStartY);
            if (fishGame->transitionT > 0.5f) {
                fishGame->transitionT = 0.0f;
                fishGame->capsuleMachineState = FISH_GAME_CAPSULE_MACHINE_STATE_WAITING_2;
            }
        } break;
        case FISH_GAME_CAPSULE_MACHINE_STATE_WAITING_2: {
            capsuleY = 166.0f;
            if (fishGame->transitionT > 0.5f) {
                fishGame->transitionT = 0.0f;
                fishGame->gameState = FISH_GAME_STATE_SCENE;
            }
        } break;
    }
    float t = fishGame->transitionT;

    addSprite(192.0f, 161.0f, assets, ATLAS_KEY_GAME, "capsule_machine_bottom", spriteList, 0.5f, 0.5f);
    addSprite(192.0f, capsuleY, assets, ATLAS_KEY_GAME, "capsule_top", spriteList, 0.5f, 1.0f);
    addSprite(192.0f, capsuleY, assets, ATLAS_KEY_GAME, getCapsuleFrame(fishGame), spriteList, 0.5f, 0.0f);
    addSprite(192.0f, 108.0f, assets, ATLAS_KEY_GAME, "capsule_machine_front", spriteList, 0.5f, 0.5f);
    addSprite(192.0f, 118.0f, assets, ATLAS_KEY_GAME, "capsule_machine_handle", spriteList, 0.5f, 0.5f, 1.0f, handleRotation);
}

void updateScene(memory_arena *memory, memory_arena *tempMemory, game_assets *assets, game_input *input, 
                 fish_game *fishGame, sprite_list *spriteList)
{
    fishGame->transitionT += DELTA_TIME;
    float t = fishGame->transitionT;
    float t2 = fishGame->transitionT * fishGame->transitionT;
    vector2 lurePos = Vector2(150.0f, 100.0f);
    vector2 offset = Vector2(t * 100.0f, 0.5f * 200.0f * t2 + -100.0f * t);
    lurePos += offset;

    if (fishGame->transitionT >= 1.5f) {
        fishGame->transitionT = 0.0f;
        fishGame->gameState = FISH_GAME_STATE_OPEN_UP;
    }

    addSprite(0.0f, 0.0f, assets, ATLAS_KEY_GAME, "scene", spriteList, 0.0f, 0.0f);
    addSprite(lurePos.x, lurePos.y, assets, ATLAS_KEY_GAME, "yellow_box", spriteList, 0.5f, 0.5f, 0.5f);
}

void drawPlayer (fish_game *fishGame, game_assets *assets, sprite_list *spriteList, float alpha = 1.0f) {
    switch (fishGame->player.type) {
        case TOY_TYPE_NINJA: {
            addSprite(fishGame->player.pos.x, fishGame->player.pos.y, assets, ATLAS_KEY_GAME, "ninja", spriteList, 0.5f, 0.5f, 1.0f, 0.0f, alpha);
        } break;
        case TOY_TYPE_STICKY_HAND: {
            addSprite(fishGame->player.pos.x, fishGame->player.pos.y, assets, ATLAS_KEY_GAME, "sticky_handle", spriteList, 0.5f, 0.5f, 1.0f, 0.0f, alpha);
            if (!fishGame->player.slapping) {
                addSprite(fishGame->player.pos.x, fishGame->player.pos.y, assets, ATLAS_KEY_GAME, "sticky_hand", spriteList, 0.5f, 0.5f, 1.0f, 0.0f, alpha);
            }
            else {
                char *handFrame = "";
                if (fishGame->player.stickyHandState == STICKY_HAND_STATE_REACHING) {
                    handFrame = "sticky_hand_slap";
                }
                else {
                    handFrame = "sticky_hand";
                }
                vector2 diff = fishGame->player.handPos - fishGame->player.pos;
                float ratio = fabsf(diff.y) / fabsf(diff.x);
                if (fabsf(diff.y) > fabsf(diff.x)) {
                    int steps = (int)(fabsf(diff.y) / 2.0f);
                    for (int i= 0; i < steps; ++i) {
                        addSprite(fishGame->player.pos.x + (float)i * (1.0f / (float)steps), fishGame->player.pos.y + i * 2.0f, assets, ATLAS_KEY_GAME, "sticky_hand_arm", spriteList, 0.5f, 0.5f, 1.0f, 0.0f, alpha);
                    }
                }
                else {
                    int steps = (int)(fabsf(diff.x) / 2.0f);
                    for (int i= 0; i < steps; ++i) {
                        addSprite(fishGame->player.pos.x + i * 2.0f, fishGame->player.pos.y + (float)i * (1.0f / (float)steps) * diff.y, assets, ATLAS_KEY_GAME, "sticky_hand_arm", spriteList, 0.5f, 0.5f, 1.0f, 0.0f, alpha);
                    }
                }
                addSprite(fishGame->player.handPos.x, fishGame->player.handPos.y, assets, ATLAS_KEY_GAME, handFrame, spriteList, 0.5f, 0.5f, 1.0f, 0.0f, alpha);
            }
        } break;
        case TOY_TYPE_BOUNCY_BALL: {
            addSprite(fishGame->player.pos.x, fishGame->player.pos.y, assets, ATLAS_KEY_GAME, "bouncy_ball", spriteList, 0.5f, 0.5f, 1.0f, fishGame->player.rotation, alpha);
        } break;
    }
}

void drawBoss (fish_game *fishGame, game_assets *assets, sprite_list *spriteList, float alpha = 1.0f, float rotation = 0.0f) {
    switch (fishGame->bossType) {
        case BOSS_TYPE_BASS: {
            addSprite(fishGame->boss.pos.x, fishGame->boss.pos.y, assets, ATLAS_KEY_GAME, "bass", spriteList, 0.3f, 0.5f, 1.0f, rotation, alpha);
        } break;
        case BOSS_TYPE_CAR: {
            addSprite(fishGame->boss.pos.x, fishGame->boss.pos.y, assets, ATLAS_KEY_GAME, "honda", spriteList, 0.2f, 0.6f, 1.0f, rotation, alpha);
        } break;
    }
}

void updateOpenUp(memory_arena *memory, memory_arena *tempMemory, game_assets *assets, game_input *input, 
                 fish_game *fishGame, sprite_list *spriteList)
{
    fishGame->transitionT += DELTA_TIME;
    vector2 capsuleTopPos = Vector2();
    vector2 capsuleBottomPos = Vector2();
    bool capsuleOpened = false;
    switch (fishGame->openUpState) {
        case FISH_GAME_OPEN_UP_STATE_DROP_LURE: {
            bool done = false;
            if (fishGame->transitionT >= 1.0f) {
                fishGame->transitionT = 1.0f;
                fishGame->openUpState = FISH_GAME_OPEN_UP_STATE_REVEAL_LURE;
                done = true;
            }
            float t = fishGame->transitionT;
            vector2 playerStartPos = Vector2(60.0f, -50.0f);
            vector2 playerEndPos = Vector2(60.0f, 105.0f);
            fishGame->player.pos = playerStartPos + 
                                   t * (playerEndPos - playerStartPos);
            capsuleTopPos = fishGame->player.pos;
            capsuleBottomPos = fishGame->player.pos;
            fishGame->boss.pos = Vector2(562.0f, 108.0f);

            if (done) {
                fishGame->transitionT = 0.0f;
            }
        } break;
        case FISH_GAME_OPEN_UP_STATE_REVEAL_LURE: {
            float t = fishGame->transitionT;
            float t2 = fishGame->transitionT * fishGame->transitionT;
            capsuleTopPos = Vector2(60.0f + -100 * t, 105.0f + 0.5f * 200.0f * t2 + -100 * t);
            capsuleBottomPos = Vector2(60.0f, 105.0f + 0.5f * 200.0f * t2);
            if (fishGame->transitionT >= 1.0f) {
                fishGame->transitionT = 0.0f;
                fishGame->openUpState = FISH_GAME_OPEN_UP_STATE_BOSS_ENTER;
            }
        } break;
        case FISH_GAME_OPEN_UP_STATE_BOSS_ENTER: {
            bool done = false;
            capsuleOpened = true;
            if (fishGame->transitionT >= 0.5f) {
                fishGame->transitionT = 0.5f;
                fishGame->gameState = FISH_GAME_STATE_BULLET_HELL;
                done = true;
            }
            float t = fishGame->transitionT / 0.5f;
            vector2 bossStartPos = Vector2(562.0f, 105.0f);
            vector2 bossEndPos = Vector2(362.0f, 105.0f);
            fishGame->boss.pos = bossStartPos + 
                                   t * (bossEndPos - bossStartPos);
            if (done) {
                fishGame->transitionT = 0.0f;
            }
        } break;
    }

    addSprite(0.0f, 0.0f, assets, ATLAS_KEY_GAME, "bg", spriteList, 0.0f, 0.0f);

    drawBoss(fishGame, assets, spriteList);

    addSprite(fishGame->player.pos.x, fishGame->player.pos.y, assets, ATLAS_KEY_GAME, "line", spriteList, 1.0f, 1.0f);
    drawPlayer(fishGame, assets, spriteList);
    if (!capsuleOpened) {
        addSprite(capsuleTopPos.x, capsuleTopPos.y + 2.0f, assets, ATLAS_KEY_GAME, "capsule_top", spriteList, 0.5f, 1.0f);
        addSprite(capsuleBottomPos.x, capsuleBottomPos.y + 2.0f, assets, ATLAS_KEY_GAME, getCapsuleFrame(fishGame), spriteList, 0.5f, 0.0f);
    }
}

void initPlayer (fish_game *fishGame) {
    toy_player *player = &fishGame->player;
    switch (player->type) {
        case TOY_TYPE_NINJA: {
            player->hitPoints = 8;
        } break;
        case TOY_TYPE_STICKY_HAND: {
            player->hitPoints = 8;
            player->handPos = player->pos;
        } break;
        case TOY_TYPE_BOUNCY_BALL: {
            player->hitPoints = 10;
        } break;
    }
}

void initBoss (fish_game *fishGame) {
    fish_boss *boss = &fishGame->boss;
    boss->numHitboxes = 0;
    switch (fishGame->bossType) {
        case BOSS_TYPE_BASS: {
            boss->hitPoints = 250;

            // relative to boss center
            rectangle hitBox;
            hitBox.min = Vector2(-95.0f, -25.0f);
            hitBox.max = Vector2(95.0f, 25.0f);
            boss->hitBoxes[boss->numHitboxes++] = hitBox;
            hitBox.min = Vector2(-55.0f, -40.0f);
            hitBox.max = Vector2(55.0f, 40.0f);
            boss->hitBoxes[boss->numHitboxes++] = hitBox;
            hitBox.min = Vector2(-30.0f, -58.0f);
            hitBox.max = Vector2(80.0f, 58.0f);
            boss->hitBoxes[boss->numHitboxes++] = hitBox;
            hitBox.min = Vector2(-75.0f, 25.0f);
            hitBox.max = Vector2(80.0f, 40.0f);
            boss->hitBoxes[boss->numHitboxes++] = hitBox;
            hitBox.min = Vector2(-5.0f, -68.0f);
            hitBox.max = Vector2(80.0f, 50.0f);
            boss->hitBoxes[boss->numHitboxes++] = hitBox;
        } break;
        case BOSS_TYPE_CAR: {
            boss->hitPoints = 150;

            // relative to boss center
            rectangle hitBox;
            hitBox.min = Vector2(-110.0f, -20.0f);
            hitBox.max = Vector2(470.0f, 50.0f);
            boss->hitBoxes[boss->numHitboxes++] = hitBox;
            hitBox.min = Vector2(-50.0f, -35.0f);
            hitBox.max = Vector2(470.0f, 50.0f);
            boss->hitBoxes[boss->numHitboxes++] = hitBox;
            hitBox.min = Vector2(-50.0f, 0.0f);
            hitBox.max = Vector2(470.0f, 70.0f);
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

void nextBossAttack (fish_game *fishGame) {
    float randomNum = randomFloat();
    if (randomNum < 0.3333f) {
        fishGame->boss.currentAttack = BOSS_ATTACK_1;
    }
    else if (randomNum < 0.6667) {
        fishGame->boss.currentAttack = BOSS_ATTACK_2;
    }
    else {
        fishGame->boss.currentAttack = BOSS_ATTACK_3;
    }
    fishGame->boss.attackTimer = 0.0f;

    fishGame->chargeTargetY = randomFloat() * 180.0f + 15.0f;
}

void spawnLaser (fish_game *fishGame, vector2 velocity) {
    vector2 spawnPos = fishGame->boss.pos + Vector2(-80.0f, -15.0f);
    spawnBullet(fishGame, spawnPos, velocity);
    spawnPos = fishGame->boss.pos + Vector2(-80.0f, -5.0f);
    spawnBullet(fishGame, spawnPos, velocity);
    spawnPos = fishGame->boss.pos + Vector2(-80.0f, 5.0f);
    spawnBullet(fishGame, spawnPos, velocity);
    spawnPos = fishGame->boss.pos + Vector2(-80.0f, 15.0f);
    spawnBullet(fishGame, spawnPos, velocity);

    spawnPos = fishGame->boss.pos + Vector2(-100.0f, -15.0f);
    spawnBullet(fishGame, spawnPos, velocity);
    spawnPos = fishGame->boss.pos + Vector2(-100.0f, -5.0f);
    spawnBullet(fishGame, spawnPos, velocity);
    spawnPos = fishGame->boss.pos + Vector2(-100.0f, 5.0f);
    spawnBullet(fishGame, spawnPos, velocity);
    spawnPos = fishGame->boss.pos + Vector2(-100.0f, 15.0f);
    spawnBullet(fishGame, spawnPos, velocity);

    spawnPos = fishGame->boss.pos + Vector2(-60.0f, -15.0f);
    spawnBullet(fishGame, spawnPos, velocity);
    spawnPos = fishGame->boss.pos + Vector2(-60.0f, -5.0f);
    spawnBullet(fishGame, spawnPos, velocity);
    spawnPos = fishGame->boss.pos + Vector2(-60.0f, 5.0f);
    spawnBullet(fishGame, spawnPos, velocity);
    spawnPos = fishGame->boss.pos + Vector2(-60.0f, 15.0f);
    spawnBullet(fishGame, spawnPos, velocity);

    spawnPos = fishGame->boss.pos + Vector2(-40.0f, -15.0f);
    spawnBullet(fishGame, spawnPos, velocity);
    spawnPos = fishGame->boss.pos + Vector2(-40.0f, -5.0f);
    spawnBullet(fishGame, spawnPos, velocity);
    spawnPos = fishGame->boss.pos + Vector2(-40.0f, 5.0f);
    spawnBullet(fishGame, spawnPos, velocity);
    spawnPos = fishGame->boss.pos + Vector2(-40.0f, 15.0f);
    spawnBullet(fishGame, spawnPos, velocity);

    spawnPos = fishGame->boss.pos + Vector2(-20.0f, -15.0f);
    spawnBullet(fishGame, spawnPos, velocity);
    spawnPos = fishGame->boss.pos + Vector2(-20.0f, -5.0f);
    spawnBullet(fishGame, spawnPos, velocity);
    spawnPos = fishGame->boss.pos + Vector2(-20.0f, 5.0f);
    spawnBullet(fishGame, spawnPos, velocity);
    spawnPos = fishGame->boss.pos + Vector2(-20.0f, 15.0f);
    spawnBullet(fishGame, spawnPos, velocity);

    spawnPos = fishGame->boss.pos + Vector2(0.0f, -15.0f);
    spawnBullet(fishGame, spawnPos, velocity);
    spawnPos = fishGame->boss.pos + Vector2(0.0f, -5.0f);
    spawnBullet(fishGame, spawnPos, velocity);
    spawnPos = fishGame->boss.pos + Vector2(0.0f, 5.0f);
    spawnBullet(fishGame, spawnPos, velocity);
    spawnPos = fishGame->boss.pos + Vector2(0.0f, 15.0f);
    spawnBullet(fishGame, spawnPos, velocity);

    spawnPos = fishGame->boss.pos + Vector2(20.0f, -15.0f);
    spawnBullet(fishGame, spawnPos, velocity);
    spawnPos = fishGame->boss.pos + Vector2(20.0f, -5.0f);
    spawnBullet(fishGame, spawnPos, velocity);
    spawnPos = fishGame->boss.pos + Vector2(20.0f, 5.0f);
    spawnBullet(fishGame, spawnPos, velocity);
    spawnPos = fishGame->boss.pos + Vector2(20.0f, 15.0f);
    spawnBullet(fishGame, spawnPos, velocity);

}

void updateBassAttack (fish_game *fishGame){
    switch (fishGame->boss.currentAttack) {
        case BOSS_ATTACK_1: {
            fishGame->boss.attackTimer += DELTA_TIME;

            fishGame->boss.pos.y = 108.0f + sinf(fishGame->boss.attackTimer * 0.8f) * 75.0f;

            fishGame->boss.shootingTimer += DELTA_TIME;
            while (fishGame->boss.shootingTimer > 0.3f) {
                fishGame->boss.shootingTimer -= 0.3f;

                vector2 velocity = BULLET_SPEED * normalize(Vector2(-1.0f, -0.1f));
                vector2 spawnPos = fishGame->boss.pos + Vector2(-60.0f, sinf(fishGame->boss.attackTimer) * 40.0f);
                spawnBullet(fishGame, spawnPos, velocity);
                velocity = BULLET_SPEED * normalize(Vector2(-1.0f, 0.1f));
                spawnPos = fishGame->boss.pos + Vector2(-40.0f, -sinf(fishGame->boss.attackTimer) * 40.0f);
                spawnBullet(fishGame, spawnPos, velocity);
                velocity = BULLET_SPEED * normalize(Vector2(-1.0f, 0.0f));
                spawnPos = fishGame->boss.pos + Vector2(-20.0f, 0.0f);
                spawnBullet(fishGame, spawnPos, velocity);
            }

            if (fishGame->boss.attackTimer > 2.0f * PI * (1.0f / 0.8)) {
                nextBossAttack(fishGame);
            }
        } break;
        case BOSS_ATTACK_2: {
            fishGame->boss.attackTimer += DELTA_TIME;

            fishGame->boss.pos.y = 108.0f;

            fishGame->boss.shootingTimer += DELTA_TIME;
            if (fishGame->boss.shootingTimer > 0.3f && fishGame->boss.shootingTimer < 0.4f) {
                fishGame->boss.shootingTimer += 0.1f;

                for (int i = 0; i < 32; ++i) {
                    vector2 velocity = BULLET_SPEED * normalize(Vector2(cosf(2.0f * PI * (float)i/32.0f), sinf(2.0f * PI * (float)i/32.0f)));
                    vector2 spawnPos = fishGame->boss.pos + Vector2(-60.0f, 0.0f);
                    spawnBullet(fishGame, spawnPos, velocity);
                }
            }
            else if (fishGame->boss.shootingTimer > 0.7f) {
                fishGame->boss.shootingTimer -= 0.7f;
                
                for (int i = 0; i < 32; ++i) {
                    vector2 velocity = BULLET_SPEED * normalize(Vector2(cosf(2.0f * PI * ((float)i/32.0f + 1.0f / 64.0f)), sinf(2.0f * PI * ((float)i/32.0f + 1.0f / 64.0f))));
                    vector2 spawnPos = fishGame->boss.pos + Vector2(-50.0f, 0.0f);
                    spawnBullet(fishGame, spawnPos, velocity);
                }
            }

            if (fishGame->boss.attackTimer > 4.0f) {
                nextBossAttack(fishGame);
            }
        } break;
        case BOSS_ATTACK_3: {
            fishGame->boss.attackTimer += DELTA_TIME;

            if (fishGame->boss.attackTimer < 0.5f) {
                float t = fishGame->boss.attackTimer / 0.5f;
                float startY = 108.0f;
                float targetY = 48.0f;

                float actualY = startY + t * (targetY - startY);
                fishGame->boss.pos.y = actualY;
            }
            else if (fishGame->boss.attackTimer < 1.5f) {
                fishGame->boss.shootingTimer += DELTA_TIME;
                while (fishGame->boss.shootingTimer > 0.3f) {
                    fishGame->boss.shootingTimer -= 0.3f;

                    vector2 velocity = 3.0f * BULLET_SPEED * normalize(Vector2(-1.0f, 0.0f));
                    spawnLaser(fishGame, velocity);
                }
            }
            else if (fishGame->boss.attackTimer < 2.0f) {
                fishGame->boss.shootingTimer = 0.0f;
                float t = (fishGame->boss.attackTimer - 1.5f) / 0.5f;
                float startY = 58.0f;
                float targetY = 108.0f;

                float actualY = startY + t * (targetY - startY);
                fishGame->boss.pos.y = actualY;
            }
            else if (fishGame->boss.attackTimer < 3.0f) {
                fishGame->boss.shootingTimer += DELTA_TIME;
                while (fishGame->boss.shootingTimer > 0.3f) {
                    fishGame->boss.shootingTimer -= 0.3f;

                    vector2 velocity = 3.0f * BULLET_SPEED * normalize(Vector2(-1.0f, 0.0f));
                    spawnLaser(fishGame, velocity);
                }
            }
            else if (fishGame->boss.attackTimer < 3.5f) {
                fishGame->boss.shootingTimer = 0.0f;
                float t = (fishGame->boss.attackTimer - 3.0f) / 0.5f;
                float startY = 108.0f;
                float targetY = 168.0f;

                float actualY = startY + t * (targetY - startY);
                fishGame->boss.pos.y = actualY;
            }
            else if (fishGame->boss.attackTimer < 4.5f) {
                fishGame->boss.shootingTimer += DELTA_TIME;
                while (fishGame->boss.shootingTimer > 0.3f) {
                    fishGame->boss.shootingTimer -= 0.3f;

                    vector2 velocity = 3.0f * BULLET_SPEED * normalize(Vector2(-1.0f, 0.0f));
                    spawnLaser(fishGame, velocity);
                }
            }
            else if (fishGame->boss.attackTimer < 5.0f) {
                fishGame->boss.shootingTimer = 0.0f;
                float t = (fishGame->boss.attackTimer - 4.5f) / 0.5f;
                float startY = 158.0f;
                float targetY = 108.0f;

                float actualY = startY + t * (targetY - startY);
                fishGame->boss.pos.y = actualY;
            }
            else {
                nextBossAttack(fishGame);
            }

        } break;

    }
}

void updateCarAttack (fish_game *fishGame){
    switch (fishGame->boss.currentAttack) {
        case BOSS_ATTACK_1: {
            fishGame->boss.attackTimer += DELTA_TIME;

            fishGame->boss.pos.y = 108.0f;
            fishGame->boss.shootingTimer += DELTA_TIME;
            while (fishGame->boss.shootingTimer > 0.125f) {
                fishGame->boss.shootingTimer -= 0.125f;

                for (int i = 0; i < 10; ++i) {
                    vector2 velocity = BULLET_SPEED * normalize(Vector2(cosf(3.0f * fishGame->boss.attackTimer + 2.0f * PI * (float)i/10.0f), sinf(3.0f * fishGame->boss.attackTimer + 2.0f * PI * (float)i/10.0f)));
                    vector2 spawnPos = fishGame->boss.pos + Vector2(-60.0f, 0.0f);
                    spawnBullet(fishGame, spawnPos, velocity);
                }
            }

            if (fishGame->boss.attackTimer > 4.0f) {
                nextBossAttack(fishGame);
            }
        } break;
        case BOSS_ATTACK_2: {
            fishGame->boss.attackTimer += DELTA_TIME;

            fishGame->boss.pos.y = 108.0f + 80.0f * sinf(0.25f * fishGame->boss.attackTimer * 2.0f * PI);

            fishGame->boss.shootingTimer += DELTA_TIME;
            while (fishGame->boss.shootingTimer > 0.05f) {
                fishGame->boss.shootingTimer -= 0.05f;

                vector2 velocity = BULLET_SPEED * normalize(Vector2(0.0f, 1.0f));
                for (int i = 0; i < 20; ++i) {
                    vector2 spawnPos = Vector2(-100.0f + (float)i * 50.0f + sinf(2.0f * fishGame->boss.attackTimer) * 50.0f, 0.0f);
                    spawnBullet(fishGame, spawnPos, velocity);
                }
            }

            if (fishGame->boss.attackTimer > 4.0f) {
                fishGame->boss.shootingTimer = 0.0f;
                nextBossAttack(fishGame);
            }
        } break;
        case BOSS_ATTACK_3: {
            fishGame->boss.attackTimer += DELTA_TIME;

            if (fishGame->boss.attackTimer < 0.5f) {
                float t = fishGame->boss.attackTimer / 0.5f;
                float startY = 108.0f;
                float targetY = fishGame->chargeTargetY;

                float actualY = startY + t * (targetY - startY);
                fishGame->boss.pos.y = actualY;
            }
            else if (fishGame->boss.attackTimer < 1.5f) {
                float randomXOffset = randomFloat() * 10.0f - 5.0f;
                float randomYOffset = randomFloat() * 10.0f - 5.0f;
                fishGame->boss.pos.x = 362.0f + randomXOffset;
                fishGame->boss.pos.y = fishGame->chargeTargetY + randomYOffset;
            }
            else if (fishGame->boss.attackTimer < 2.5f) {
                float t = (fishGame->boss.attackTimer - 1.5f) / 1.0f;
                float startX = 362.0f;
                float targetX = -500.0f;

                float actualX = startX + t * (targetX - startX);
                fishGame->boss.pos.x = actualX;
            }
            else if (fishGame->boss.attackTimer < 3.0f) {
                float t = (fishGame->boss.attackTimer - 2.5f) / 0.5f;
                float startX = 562.0f;
                float targetX = 362.0f;

                float actualX = startX + t * (targetX - startX);
                fishGame->boss.pos.x = actualX;
                fishGame->boss.pos.y = 108.0f;
            }
            else {
                nextBossAttack(fishGame);
            }
        } break;

    }
}

void updateBoss (fish_game *fishGame) {
    switch (fishGame->bossType){
        case BOSS_TYPE_BASS:{
            updateBassAttack(fishGame);
        } break;
        case BOSS_TYPE_CAR:{
            updateCarAttack(fishGame);
        } break;
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

void updatePlayer (fish_game *fishGame, game_input *input, bool inputLeft, bool inputRight, bool inputUp, bool inputDown) {
    toy_player *player = &fishGame->player;
    switch (player->type) {
        case TOY_TYPE_NINJA:
        case TOY_TYPE_STICKY_HAND: {
            if (inputUp) {
                player->pos.y -= 100.0f * DELTA_TIME;
            }
            if (inputDown) {
                player->pos.y += 100.0f * DELTA_TIME;
            }
            if (inputLeft) {
                player->pos.x -= 100.0f * DELTA_TIME;
            }
            if (inputRight) {
                player->pos.x += 100.0f * DELTA_TIME;
            }
        } break;
        case TOY_TYPE_BOUNCY_BALL: {
            player->velocity.y += 400.0f * DELTA_TIME;
            //if (input->upKey.down) {
            //    player->velocity.y -= 50.0f * DELTA_TIME;
            //}
            //if (input->downKey.down) {
            //    player->velocity.y += 50.0f * DELTA_TIME;
            //}
            if (inputLeft) {
                player->velocity.x -= 200.0f * DELTA_TIME;
                if (player->velocity.x < -400.0f) {
                    player->velocity.x = -400.0f;
                }
            }
            if (inputRight) {
                player->velocity.x += 200.0f * DELTA_TIME;
                if (player->velocity.x > 400.0f) {
                    player->velocity.x = 400.0f;
                }
            }
            player->pos += player->velocity * DELTA_TIME;
        } break;
    }

    bool touchedFloor = false;
    bool touchedSide = false;
    if (player->pos.x - PLAYER_RADIUS < 0.0f) {
        player->pos.x = PLAYER_RADIUS;
        touchedSide = true;
    }
    if (player->pos.x + PLAYER_RADIUS >= 384.0f) {
        player->pos.x = 384.0f - PLAYER_RADIUS;
        touchedSide = true;
    }
    if (player->pos.y - PLAYER_RADIUS < 0.0f) {
        player->pos.y = PLAYER_RADIUS;
    }
    if (player->pos.y + PLAYER_RADIUS >= 216.0f) {
        player->pos.y = 216.0f - PLAYER_RADIUS;
        touchedFloor = true;
    }

    switch (player->type) {
        case TOY_TYPE_NINJA: {
            if (input->zKey.down || input->pointerDown) {
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
        } break; 
        case TOY_TYPE_STICKY_HAND: {
            if (!player->slapping) {
                player->handPos = player->pos;

                if (input->zKey.justPressed || input->pointerDown) {
                    player->slapping = true;
                    player->slapped = false;
                    player->stickyHandState = STICKY_HAND_STATE_REACHING;
                    player->stickyHandStartPos = player->pos;
                    player->stickyHandTargetPos = player->pos + Vector2(200.0f, 0.0f);
                    player->stickyHandTimer = 0.0f;
                }
            }

            if (player->slapping) {
                player->stickyHandTimer += DELTA_TIME;
                switch (player->stickyHandState) {
                    case STICKY_HAND_STATE_REACHING: {
                        bool done = false;
                        if (player->stickyHandTimer >= 0.3f) {
                            player->stickyHandTimer = 0.3f;
                            done = true;
                        }
                        float t = player->stickyHandTimer / 0.3f;
                        player->handPos = player->stickyHandStartPos + t * (player->stickyHandTargetPos - player->stickyHandStartPos);
                        if (done) {
                            player->stickyHandTimer = 0.0f;
                            player->stickyHandState = STICKY_HAND_STATE_SLAPPING;
                        }
                    } break;
                    case STICKY_HAND_STATE_SLAPPING: {
                        if (player->stickyHandTimer >= 0.1f) {
                            player->stickyHandTimer = 0.0f;
                            player->stickyHandState = STICKY_HAND_STATE_RETURNING;
                        }
                    } break;
                    case STICKY_HAND_STATE_RETURNING: {
                        bool done = false;
                        if (player->stickyHandTimer >= 0.5f) {
                            player->stickyHandTimer = 0.5f;
                            done = true;
                        }
                        float t = player->stickyHandTimer / 0.5f;
                        player->handPos = player->stickyHandTargetPos + t * (player->pos - player->stickyHandTargetPos);
                        if (done) {
                            player->stickyHandTimer = 0.0f;
                            player->slapping = false;
                        }
                    } break;
                }
            }
        } break;
        case TOY_TYPE_BOUNCY_BALL: {
            player->bounceTimer -= DELTA_TIME;
            if (player->bounceTimer < 0.0f) {
                player->bounceTimer = 0.0f;
            }
            if (touchedFloor) {
                player->velocity.y = -400;
                player->dRotation = randomFloat() * 8.0f - 4.0f;
                player->bouncedOnBoss = false;
                player->bouncing = false;
            }
            if (player->bounceTimer <= 0.0f && (input->zKey.justPressed || input->pointerJustDown)) {
                player->velocity.y = 400;
                player->bounceTimer = 0.05f;
                player->bouncing = true;
            }
            if(touchedSide) {
                player->velocity.x = -player->velocity.x;
            }
            player->rotation += player->dRotation * DELTA_TIME * PI;
        } break;
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

bool checkHandBossCollision (fish_game *fishGame) {
    fish_boss *boss = &fishGame->boss;
    int numHitboxes = boss->numHitboxes;
    rectangle *hitBoxes = boss->hitBoxes;

    toy_player *player = &fishGame->player;

    for (int j = 0; j < numHitboxes; ++j) {
        rectangle playerRect;
        playerRect.min = player->handPos + Vector2(-PLAYER_RADIUS, -PLAYER_RADIUS);
        playerRect.min = playerRect.min - boss->pos;
        playerRect.max = player->handPos + Vector2(PLAYER_RADIUS, PLAYER_RADIUS);
        playerRect.max = playerRect.max - boss->pos;

        if (rectangleIntersection(playerRect, hitBoxes[j])) {
            return true;
        }
    }
    return false;
}

bool checkPlayerBouncedOnBoss(fish_game *fishGame) {
    toy_player *player = &fishGame->player;
    if (player->type != TOY_TYPE_BOUNCY_BALL) { return false; }
    if (player->bouncedOnBoss || !player->bouncing) { return false; }

    bool overlappedBoss = checkPlayerBossCollision(fishGame);
    if (overlappedBoss) {
        player->bouncedOnBoss = true;
        return true;
    }
    return false;
}

bool checkPlayerSlappedBoss(fish_game *fishGame) {
    toy_player *player = &fishGame->player;
    if (player->type != TOY_TYPE_STICKY_HAND) { return false; }
    if (player->stickyHandState != STICKY_HAND_STATE_SLAPPING || player->slapped || !player->slapping) { return false; }

    bool overlappedBoss = checkHandBossCollision(fishGame);
    if (overlappedBoss) {
        player->slapped = true;
        return true;
    }
    return false;
}

void updateBulletHell(memory_arena *memory, memory_arena *tempMemory, game_assets *assets, game_input *input, 
                      fish_game *fishGame, sprite_list *spriteList, memory_arena *stringMemory)
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

    matrix3x3 gameTransform = peekSpriteMatrix(spriteList);
    vector3 localPointerPos = Vector3((float)input->pointerX, (float)input->pointerY, 1.0f);
    localPointerPos = inverse(gameTransform) * localPointerPos;
    vector2 pointerPos2d= Vector2(localPointerPos.x, localPointerPos.y);

    vector2 playerDiff = pointerPos2d - fishGame->player.pos;

    bool inputLeft = input->leftKey.down || (fabsf(playerDiff.x) > 5.0f && input->pointerDown && playerDiff.x < 0);
    bool inputRight = input->rightKey.down || (fabsf(playerDiff.x) > 5.0f && input->pointerDown && playerDiff.x > 0);
    bool inputUp = input->upKey.down || (fabsf(playerDiff.y) > 5.0f && input->pointerDown && playerDiff.y < 0);
    bool inputDown = input->downKey.down || (fabsf(playerDiff.y) > 5.0f && input->pointerDown && playerDiff.y > 0);

    updatePlayer(fishGame, input, inputLeft, inputRight, inputUp, inputDown);
    updatePlayerBullets(fishGame);

    updateBoss(fishGame);
    updateBullets(fishGame, &bulletCellGrid);

    toy_player *player = &fishGame->player;
    if (!player->hurt) {
        bool playerHitByBullet = checkPlayerBulletCollisions(fishGame, &bulletCellGrid);
        if (checkPlayerBulletCollisions(fishGame, &bulletCellGrid) ||
            checkPlayerBossCollision(fishGame)) 
        {
            if (!player->bouncing) {
                player->hurt = true;
                player->hurtFlash = true;
                --player->hitPoints;
            }
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
    if (checkPlayerBouncedOnBoss(fishGame)) {
        fishGame->boss.hurt = true;
        fishGame->boss.hurtTimer = 0.0f;
        fishGame->boss.hitPoints -= 3;
    }
    if (checkPlayerSlappedBoss(fishGame)) {
        fishGame->boss.hurt = true;
        fishGame->boss.hurtTimer = 0.0f;
        fishGame->boss.hitPoints -= 4;
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
        fishGame->player.slapping = false;
        fishGame->playerTransitionStartPos = fishGame->player.pos;
        fishGame->bossTransitionStartPos = fishGame->boss.pos;
        fishGame->gameState = FISH_GAME_STATE_TRANSITION;
        fishGame->transitionT = 0.0f;
        fishGame->transitionState = FISH_GAME_TRANSITION_STATE_MOVING;
    }

    //pushSpriteTransform(spriteList, Vector2(GAME_WIDTH/2.0f, GAME_HEIGHT/2.0f));

    addSprite(0.0f, 0.0f, assets, ATLAS_KEY_GAME, "bg", spriteList, 0.0f, 0.0f);

    // debug drawing of how many bullets in each cell
    if (fishGame->debug) {
        for (int i = 0; i < bulletCellGrid.numRows; ++i) {
            for (int j = 0; j < bulletCellGrid.numCols; ++j) {
                bullet_cell *bulletCell = bulletCellGrid.cells + (i * bulletCellGrid.numCols + j);
                addText(j * 16.0f + 4.0f, i * 16.0f + 4.0f, numToString(bulletCell->numBullets, stringMemory), assets, TEXTURE_KEY_FONT, spriteList);
            }
        }
    }

    for (int i = 0; i < fishGame->numActiveBullets; ++i) {
        fish_bullet *bullet = fishGame->bullets + i;
        addSprite(bullet->pos.x, bullet->pos.y, assets, ATLAS_KEY_GAME, "bullet", spriteList, 0.5f, 0.5f);
    }

    if (fishGame->boss.hurt) {
        drawBoss(fishGame, assets, spriteList, 0.3f);
    }
    else {
        drawBoss(fishGame, assets, spriteList);
    }

    addSprite(fishGame->player.pos.x, fishGame->player.pos.y, assets, ATLAS_KEY_GAME, "line", spriteList, 1.0f, 1.0f);
    if (player->hurtFlash) {
        drawPlayer(fishGame, assets, spriteList, 0.3f);
    }
    else {
        drawPlayer(fishGame, assets, spriteList);
    }

    for (int i = 0; i < fishGame->numActivePlayerBullets; ++i) {
        fish_bullet *bullet = fishGame->playerBullets + i;
        addSprite(bullet->pos.x, bullet->pos.y, assets, ATLAS_KEY_GAME, "ninja_star", spriteList, 0.5f, 0.5f);
    }

    for (int i = 0; i < player->hitPoints; ++i) {
        addSprite(8.0f + 8.0f * i, 200.0f, assets, ATLAS_KEY_GAME, "yellow_box", spriteList);
    }
    for (int i = 0; i < fishGame->boss.hitPoints; ++i) {
        addSprite(368.0f - 0.5f * i, 200.0f, assets, ATLAS_KEY_GAME, "green_box", spriteList);
    }

    // debugging for boss hitboxes
    if (fishGame->debug) {
        int numHitboxes = fishGame->boss.numHitboxes;
        rectangle *hitBoxes = fishGame->boss.hitBoxes;
        pushSpriteTransform(spriteList, fishGame->boss.pos);
        for (int i = 0; i < numHitboxes; ++i) {
            rectangle hitBox = hitBoxes[i];
            vector2 rectDims = hitBox.max - hitBox.min;
            // white box texture is 4x4 px
            vector2 whiteBoxScale = rectDims * (1.0f / 4.0f);
            matrix3x3 boxTransform = scaleMatrix3x3(whiteBoxScale.x, whiteBoxScale.y);
            boxTransform = translationMatrix(hitBox.min.x, hitBox.min.y) * boxTransform;
            pushSpriteMatrix(boxTransform, spriteList);
            addSprite(0.0f, 0.0f, assets, ATLAS_KEY_GAME, "white_box", spriteList, 0.0f, 0.0f, 1.0f, 0.0f, 0.5f);
            popSpriteMatrix(spriteList);
        }
        popSpriteMatrix(spriteList);
    }
}

// game half dims: 192x108


void drawMeter (game_assets *assets, fish_game *fishGame, sprite_list *spriteList, float offset = 0.0f) {
    pushSpriteTransform(spriteList, Vector2(offset, 0.0f));

    float meterScale = fishGame->meterChunkHeight * (1.0f / 8.0f);
    matrix3x3 meterChunkTransform = scaleMatrix3x3(1.0f, meterScale);
    meterChunkTransform = translationMatrix(8.0f, 208.0f - fishGame->meterChunkY) * meterChunkTransform;
    pushSpriteMatrix(meterChunkTransform, spriteList);
    addSprite(0.0f, 0.0f, assets, ATLAS_KEY_GAME, "green_box", spriteList, 0.0f, 1.0f);
    popSpriteMatrix(spriteList);

    addSprite(8.0f, 208.0f - fishGame->meterFishY, assets, ATLAS_KEY_GAME, "meter_fish", spriteList, 0.0f, 0.5f);

    float progressScale = fishGame->meterProgress * (1.0f / 8.0f);
    matrix3x3 meterProgressTransform = scaleMatrix3x3(1.0f, progressScale);
    meterProgressTransform = translationMatrix(18.0f, 208.0f) * meterProgressTransform;
    pushSpriteMatrix(meterProgressTransform, spriteList);
    addSprite(0.0f, 0.0f, assets, ATLAS_KEY_GAME, "yellow_box", spriteList, 0.0f, 1.0f);
    popSpriteMatrix(spriteList);

    addSprite(7.0f, 7.0f, assets, ATLAS_KEY_GAME, "fish_meter", spriteList);

    popSpriteMatrix(spriteList);
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
            vector2 fishTarget = Vector2(192.0f + 60.0f, 108.0f);
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
               fishGame->fishingState = FISH_GAME_FISHING_STATE_FISHING;
               fishGame->gameState = FISH_GAME_STATE_FISHING;
               fishGame->bossTransitionStartPos = fishGame->boss.pos;
           }

        } break;
    }

    addSprite(0.0f, 0.0f, assets, ATLAS_KEY_GAME, "bg", spriteList, 0.0f, 0.0f);

    addSprite(fishGame->player.pos.x, fishGame->player.pos.y, assets, ATLAS_KEY_GAME, "line", spriteList, 1.0f, 1.0f);

    for (int i = 0; i < fishGame->numActiveBullets; ++i) {
        fish_bullet *bullet = fishGame->bullets + i;
        addSprite(bullet->pos.x, bullet->pos.y, assets, ATLAS_KEY_GAME, "bullet", spriteList, 0.5f, 0.5f);
    }


    drawPlayer(fishGame, assets, spriteList);

    drawBoss(fishGame, assets, spriteList);
    for (int i = 0; i < fishGame->numActivePlayerBullets; ++i) {
        fish_bullet *bullet = fishGame->playerBullets + i;
        addSprite(bullet->pos.x, bullet->pos.y, assets, ATLAS_KEY_GAME, "ninja_star", spriteList, 0.5f, 0.5f);
    }


    for (int i = 0; i < fishGame->player.hitPoints; ++i) {
        addSprite(8.0f + 8.0f * i, 200.0f, assets, ATLAS_KEY_GAME, "yellow_box", spriteList);
    }
    for (int i = 0; i < fishGame->boss.hitPoints; ++i) {
        addSprite(368.0f - 0.5f * i, 200.0f, assets, ATLAS_KEY_GAME, "green_box", spriteList);
    }

    drawMeter(assets, fishGame, spriteList, meterOffset);
}

void updateFishingGame(memory_arena *memory, memory_arena *tempMemory, game_assets *assets, game_input *input, 
                       fish_game *fishGame, sprite_list *spriteList)
{
    float fadeOutAlpha = 0.0f;
    switch (fishGame->fishingState) {
        case FISH_GAME_FISHING_STATE_FISHING: {
            if (input->zKey.down || input->pointerDown) {
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
            switch (fishGame->bossType) {
                case BOSS_TYPE_BASS: {
                    fishGame->meterFishY = 100.0f + sinf(1.1f * fishGame->meterSineVal) * 60.0f;
                } break;
                case BOSS_TYPE_CAR: {
                    fishGame->meterFishY = 100.0f + sinf(0.25f * fishGame->meterSineVal) * 80.0f + cosf(0.1f + 5.0f * fishGame->meterSineVal) * 20.0f;
                } break;
            }
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
                fishGame->fishingState = FISH_GAME_FISHING_STATE_FADE_OUT;
                fishGame->won = false;
            }
            if (fishGame->meterProgress > 200.0f) {
                fishGame->meterProgress = 200.0f;
                fishGame->fishingState = FISH_GAME_FISHING_STATE_FADE_OUT;
                fishGame->won = true;
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

        } break;
        case FISH_GAME_FISHING_STATE_FADE_OUT: {
            fishGame->fadeOutProgress += DELTA_TIME * 3.0f;
            if (fishGame->fadeOutProgress >= 1.0f) {
                fishGame->fadeOutProgress = 1.0f;
                fishGame->gameState = FISH_GAME_STATE_END_SCREEN;
                fishGame->endScreenState = FISH_GAME_END_SCREEN_STATE_FADE_OUT;
            }
            fadeOutAlpha = fishGame->fadeOutProgress;
        } break;
    }
    addSprite(0.0f, 0.0f, assets, ATLAS_KEY_GAME, "bg", spriteList, 0.0f, 0.0f);

    vector2 linePos = fishGame->boss.pos - Vector2(60.0f, 0.0f);
    addSprite(linePos.x, linePos.y, assets, ATLAS_KEY_GAME, "line", spriteList, 1.0f, 1.0f);
    drawBoss(fishGame, assets, spriteList);

    drawMeter(assets, fishGame, spriteList);

    addSprite(0, 0, assets, ATLAS_KEY_GAME, "black_screen", spriteList, 0.0f, 0.0f, 1.0f, 0.0f, fadeOutAlpha);

    if (fishGame->gameState == FISH_GAME_STATE_END_SCREEN) {
        fishGame->fadeOutProgress = 0.0f;
    }
}

void initFishGame (memory_arena *memory, fish_game* fishGame) {
    unsigned int time = (unsigned int)getTime();
    setRNGSeed(time); // TODO(ebuchholz): seed with time?

    *fishGame = {};
    fishGame->debug = false;

    //fishGame->gameState = FISH_GAME_STATE_BULLET_HELL;
    fishGame->gameState = FISH_GAME_STATE_TITLE_SCREEN;

    toy_player *player = &fishGame->player;
    player->pos = Vector2(60.0f, 105.0f);
    float randomNum = randomFloat();
    if (randomNum < 0.3333f) {
        player->type = TOY_TYPE_NINJA;
    }
    else if (randomNum < 0.6667) {
        player->type = TOY_TYPE_STICKY_HAND;
    }
    else {
        player->type = TOY_TYPE_BOUNCY_BALL;
    }
    player->bounceTimer = 0.0f;
    initPlayer(fishGame);

    for (int i = 0 ; i < MAX_NUM_PLAYER_BULLETS; ++i) {
        fishGame->playerBullets[i] = {};
    }
    fishGame->numActivePlayerBullets = 0;

    fishGame->boss.pos = Vector2(362.0f, 108.0f);
    if (randomFloat() < 0.5f) {
        fishGame->bossType = BOSS_TYPE_BASS;
    }
    else {
        fishGame->bossType = BOSS_TYPE_CAR;
    }
    initBoss(fishGame);
    nextBossAttack(fishGame);

    for (int i = 0 ; i < MAX_NUM_ENEMY_BULLETS; ++i) {
        fishGame->bullets[i] = {};
    }
    fishGame->numActiveBullets = 0;
    fishGame->boss.shootingTimer = 0.0f;
    fishGame->boss.attackTimer = 0.0f;

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

void updateEndScreen (memory_arena *memory, memory_arena *tempMemory, game_assets *assets, game_input *input, 
                      fish_game *fishGame, sprite_list *spriteList) 
{
    float fadeOutAlpha = 0.0f;
    switch (fishGame->endScreenState) {
        case FISH_GAME_END_SCREEN_STATE_FADE_OUT: {
            fishGame->fadeOutProgress += DELTA_TIME * 3.0f;
            if (fishGame->fadeOutProgress >= 1.0f) {
                fishGame->fadeOutProgress = 1.0f;
                fishGame->endScreenState = FISH_GAME_END_SCREEN_STATE_SHOWING_RESULTS;
            }
            fadeOutAlpha = 1.0f - fishGame->fadeOutProgress;
        } break;
        case FISH_GAME_END_SCREEN_STATE_SHOWING_RESULTS: {
            if (input->zKey.justPressed || input->pointerJustDown) {
                *fishGame = {};
                initFishGame(memory, fishGame);
            }
        } break;
    }

    addSprite(280.0f, 90.0f, assets, ATLAS_KEY_GAME, "end_line", spriteList, 0.5f, 1.0f);

    if (fishGame->won) {
        char *bossFrame = "";
        if(fishGame->bossType == BOSS_TYPE_BASS) {
            bossFrame = "bass";
            addSprite(280, 150, assets, ATLAS_KEY_GAME, bossFrame, spriteList, 0.3f, 0.5f, 1.0f, (90.0f * (PI / 180.0f)));
            addText(32.0f, 56.0f, "Largemouth Bass", assets, TEXTURE_KEY_FONT, spriteList);
            addText(72.0f, 72.0f, "8 LBS", assets, TEXTURE_KEY_FONT, spriteList);
            addText(28.0f, 88.0f, "Congratulations!", assets, TEXTURE_KEY_FONT, spriteList);
        }
        else {
            bossFrame = "honda";
            addSprite(280, 150, assets, ATLAS_KEY_GAME, bossFrame, spriteList, 0.2f, 0.6f, 1.0f, (90.0f * (PI / 180.0f)));
            addText(48.0f, 56.0f, "Honda tesla", assets, TEXTURE_KEY_FONT, spriteList);
            addText(56.0f, 72.0f, "3,200 LBS", assets, TEXTURE_KEY_FONT, spriteList);
            addText(28.0f, 88.0f, "Congratulations!", assets, TEXTURE_KEY_FONT, spriteList);
        }
    }
    else {
        char *playerFrame = "";
        switch (fishGame->player.type) {
            case TOY_TYPE_NINJA: {
                playerFrame = "ninja";
            } break;
            case TOY_TYPE_STICKY_HAND: {
                playerFrame = "sticky_hand";
            } break;
            case TOY_TYPE_BOUNCY_BALL: {
                playerFrame = "bouncy_ball";
            } break;
        }
        addSprite(280.0f, 90.0f, assets, ATLAS_KEY_GAME, playerFrame, spriteList, 0.5f, 0.5f);

        addText(36.0f, 56.0f, "He got away", assets, TEXTURE_KEY_FONT, spriteList);
        addText(4.0f, 72.0f, "Press Z to continue", assets, TEXTURE_KEY_FONT, spriteList);
    }

    addSprite(0, 0, assets, ATLAS_KEY_GAME, "black_screen", spriteList, 0.0f, 0.0f, 1.0f, 0.0f, fadeOutAlpha);
}

void updateFishGame (memory_arena *memory, memory_arena *tempMemory, game_assets *assets, game_input *input, 
                     fish_game *fishGame, sprite_list *spriteList)
{
    // memory for dynamically created strings
    memory_arena stringMemory = {};
    stringMemory.capacity = 512 * 1024;
    stringMemory.base = allocateMemorySize(tempMemory, stringMemory.capacity);

    if (input->dKey.justPressed) {
        fishGame->debug = !fishGame->debug;
    }

    switch (fishGame->gameState) {
        case FISH_GAME_STATE_TITLE_SCREEN: {
            updateTitleScreen(memory, tempMemory, assets, input, fishGame, spriteList);
        } break;
        case FISH_GAME_STATE_CAPSULE_MACHINE: {
            updateCapsuleMachine(memory, tempMemory, assets, input, fishGame, spriteList);
        } break;
        case FISH_GAME_STATE_SCENE: {
            updateScene(memory, tempMemory, assets, input, fishGame, spriteList);
        } break;
        case FISH_GAME_STATE_OPEN_UP: {
            updateOpenUp(memory, tempMemory, assets, input, fishGame, spriteList);
        } break;
        case FISH_GAME_STATE_BULLET_HELL: {
            updateBulletHell(memory, tempMemory, assets, input, fishGame, spriteList, &stringMemory);
        } break;
        case FISH_GAME_STATE_TRANSITION: {
            updateTransition(memory, tempMemory, assets, input, fishGame, spriteList);
        } break;
        case FISH_GAME_STATE_FISHING: {
            updateFishingGame(memory, tempMemory, assets, input, fishGame, spriteList);
        } break;
        case FISH_GAME_STATE_END_SCREEN: {
            updateEndScreen(memory, tempMemory, assets, input, fishGame, spriteList);
        } break;
    }
}
