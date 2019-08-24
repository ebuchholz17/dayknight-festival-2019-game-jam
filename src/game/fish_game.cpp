#include "fish_game.h"

void initFishGame (memory_arena *memory, fish_game* fishGame) {
    //setRNGSeed(0); // TODO(ebuchholz): seed with time?

    *fishGame = {};

    fishGame->playerX = 0.0f;
    fishGame->playerY = 0.0f;
}

void updateFishGame (memory_arena *memory, memory_arena *tempMemory, game_assets *assets, game_input *input, 
                     fish_game *fishGame, sprite_list *spriteList)
{
    // memory for dynamically created strings
    memory_arena stringMemory = {};
    stringMemory.capacity = 512 * 1024;
    stringMemory.base = allocateMemorySize(tempMemory, stringMemory.capacity);

    if (input->upKey.down) {
        fishGame->playerY -= 100.0f * DELTA_TIME;
    }
    if (input->downKey.down) {
        fishGame->playerY += 100.0f * DELTA_TIME;
    }
    if (input->leftKey.down) {
        fishGame->playerX -= 100.0f * DELTA_TIME;
    }
    if (input->rightKey.down) {
        fishGame->playerX += 100.0f * DELTA_TIME;
    }

    pushSpriteTransform(spriteList, Vector2(GAME_WIDTH/2.0f, GAME_HEIGHT/2.0f));
    addSprite(fishGame->playerX, fishGame->playerY, assets, ATLAS_KEY_GAME, "player", spriteList, 0.5f, 0.5f);
    popSpriteMatrix(spriteList);

    addText(8, 8, "hello world", assets, TEXTURE_KEY_FONT, spriteList);
}
