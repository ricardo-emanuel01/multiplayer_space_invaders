# include <stdint.h>
# include <raylib.h>

# include "entity.h"
# include "gameData.h"


void processInput(Input *input) {
    *input = 0;

    float stickX = 0.0f;
    const float stickDeadzone = 0.1f;
    bool gamepadAvailable = false;
    if (IsGamepadAvailable(0)) {
        stickX = GetGamepadAxisMovement(0, GAMEPAD_AXIS_LEFT_X);
        if (stickX < stickDeadzone && stickX > -stickDeadzone) stickX = 0.0f;
        gamepadAvailable = true;
    }

    if (
        IsKeyPressed(KEY_UP) ||
        (gamepadAvailable && IsGamepadButtonPressed(0, GAMEPAD_BUTTON_LEFT_FACE_UP))
    ) {
        *input |= 1;
    }

    if (
        IsKeyPressed(KEY_DOWN) ||
        (gamepadAvailable && IsGamepadButtonPressed(0, GAMEPAD_BUTTON_LEFT_FACE_DOWN))
    ) {
        *input |= 1 << 1;
    }

    if (
        IsKeyDown(KEY_LEFT) ||
        stickX < 0.0f
    ) {
        *input |= 1 << 2;
    }

    if (
        IsKeyDown(KEY_RIGHT) ||
        stickX > 0.0f
    ) {
        *input |= 1 << 3;
    }

    if (
        IsKeyPressed(KEY_SPACE) ||
        (gamepadAvailable && IsGamepadButtonPressed(0, GAMEPAD_BUTTON_RIGHT_FACE_DOWN))
    ) {
        *input |= 1 << 4;
    }

    if (
        IsKeyPressed(KEY_ENTER) ||
        (gamepadAvailable && IsGamepadButtonPressed(0, GAMEPAD_BUTTON_RIGHT_FACE_RIGHT))
    ) {
        *input |= 1 << 5;
    }

    if (
        IsKeyPressed(KEY_ESCAPE) ||
        (gamepadAvailable && IsGamepadButtonPressed(0, GAMEPAD_BUTTON_MIDDLE_RIGHT))
    ) {
        *input |= 1 << 6;
    }
}

void loseGame(Game *game) {
    game->ship.state = DEAD;
    game->hotData->gameState = LOSE;
    StopMusicStream(game->sounds->background);
    PlaySound(game->sounds->shipExplosion);
    PlaySound(game->sounds->lose);
}

void activatePowerup(Game *game, int powerupIdx) {
    game->powerups[powerupIdx].state = INACTIVE;

    switch (game->powerups[powerupIdx].type) {
        case FAST_MOVE:
        {
            game->hotData->fastMoveRemainingTime = game->coldData->powerupDuration;
        } break;
        case FAST_SHOT:
        {
            game->hotData->fastShotRemainingTime = game->coldData->powerupDuration;
        } break;
        default: break;
    }
}

void checkShipPowerupCollision(Game *game) {
    EntitiesIterator it = createIterator(game->powerups, POWERUPS, game->nPowerups);

    while (!iteratorReachedEnd(&it)) {
        int powerupIdx = getCurrentIndex(&it);

        if (checkCollisionRecs(game->powerups[powerupIdx].bounds, game->ship.bounds)) {
            activatePowerup(game, powerupIdx);
        }

        iteratorNext(&it);
    }
}

void checkAlienBulletCollision(Game *game) {
    CollisionIterator it = createCollisionIterator(game->bullets, game->horde, game->nBullets, nRowsAliens*nColsAliens);
    int bulletIdx, alienIdx;

    while (!collisionIteratorReachedEnd(&it)) {
        if (checkPairCollision(&it, &bulletIdx, &alienIdx)) {
            // NOTE: That block should be a separate funcion? 
            game->bullets[bulletIdx].state = INACTIVE;
            game->horde[alienIdx].state    = DEAD;
            game->enemiesAlive--;
            PlaySound(game->sounds->alienExplosion);

            if (alienIdx == game->hordeLastAlive) {
                for (; game->hordeLastAlive > 0 && game->horde[game->hordeLastAlive].state == DEAD; --game->hordeLastAlive);
            }
            // ---------------------------------

            // NOTE: should the iterator implement that?
            resetIterator(&it.aliens);
            iteratorNext(&it.bullets);
        }

        collisionIteratorNext(&it);
    }
}

void checkShipBulletCollision(Game *game) {
    EntitiesIterator it = createIterator(game->bullets, BULLETS_AT_SHIP, game->nBullets);

    while (!iteratorReachedEnd(&it)) {
        int bulletIdx = getCurrentIndex(&it);
        if (checkCollisionRecs(game->bullets[bulletIdx].bounds, game->ship.bounds)) {
            game->bullets[bulletIdx].state = INACTIVE;
            loseGame(game);
            return;
        }

        iteratorNext(&it);
    }
}

void checkEnemyShipBulletCollision(Game *game) {
    EntitiesIterator it = createIterator(game->bullets, BULLETS_AT_ENEMIES, game->nBullets);

    while (!iteratorReachedEnd(&it)) {
        int bulletIdx = getCurrentIndex(&it);
        if (checkCollisionRecs(game->bullets[bulletIdx], game->enemyShip.bounds)) {
            game->enemyShip.state = DEAD;
            game->bullets->state  = INACTIVE;
            game->enemiesAlive--;
            PlaySound(game->sounds->shipExplosion);
        }

        iteratorNext(&it);
    }
}

void checkCollisions(Game *game) {
    checkAlienBulletCollision(game);
    checkEnemyShipBulletCollision(game);
    checkShipBulletCollision(game);
    checkShipPowerupCollision(game);
}
