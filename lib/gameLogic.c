#include "gameLogic.h"

#include <raylib.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "entity.h"
#include "gameData.h"


// The index will be incremented at each frame
void playSoundFX(Game *game, SoundSelect sound) {
    switch (sound) {
        case ALIEN_EXPLOSION_FX:
        {
            PlaySound(game->sounds->alienExplosion);
        } break;
        case ALIEN_FIRE_FX:
        {
            PlaySound(game->sounds->alienFire);
        } break;
        case LOSE_FX:
        {
            PlaySound(game->sounds->lose);
        } break;
        case MENU_FX:
        {
            PlaySound(game->sounds->menu);
        } break;
        case POWERUP_FX:
        {
            PlaySound(game->sounds->powerup);
        } break;
        case SHIP_EXPLOSION_FX:
        {
            PlaySound(game->sounds->shipExplosion);
        } break;
        case SHIP_FIRE_FX:
        {
            PlaySound(game->sounds->shipFire);
        } break;
        case VICTORY_FX:
        {
            PlaySound(game->sounds->victory);
        } break;
        default: return;
    }

    addSound(game->soundEventsBuf, sound);
}

void manageMusic(Game *game, MusicSelect music) {
    switch (music) {
        case PLAY_BACKGROUND_MUSIC:
        {
            PlayMusicStream(game->sounds->background);
            game->musicEvents |= 1;
        } break;
        case STOP_BACKGROUND_MUSIC:
        {
            StopMusicStream(game->sounds->background);
            game->musicEvents &= ~1;
        } break;
        case PLAY_ENEMY_SHIP_MUSIC:
        {
            PlayMusicStream(game->sounds->enemyShip);
            game->musicEvents |= 1 << 1;
        } break;
        case STOP_ENEMY_SHIP_MUSIC:
        {
            StopMusicStream(game->sounds->enemyShip);
            game->musicEvents &= ~(1 << 1);
        } break;
    }
}

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
    game->ships[0].state = DEAD;
    game->hotData->gameState = LOSE;
    manageMusic(game, STOP_BACKGROUND_MUSIC);
    manageMusic(game, STOP_ENEMY_SHIP_MUSIC);
    playSoundFX(game, LOSE_FX);
}

void activatePowerup(Game *game, Entity *powerup, int shipNumber) {
    powerup->state = INACTIVE;
    ShipsTimers *shipsTimers = &game->hotData->shipsTimers;

    switch (powerup->type) {
        case FAST_MOVE:
        {
            shipsTimers->remainingTimeFastMove[shipNumber] = game->coldData->powerupDuration;
        } break;
        case FAST_SHOT:
        {
            shipsTimers->remainingTimeFastShot[shipNumber] = game->coldData->powerupDuration;
            shipsTimers->remainingTimeToFire[shipNumber]   = 0.0f;
        } break;
        default: break;
    }
}

void checkShipPowerupCollision(Game *game) {
    EntitiesIterator it = createIterator(game->powerups, POWERUPS, game->nPowerups);

    while (!iteratorReachedEnd(&it)) {
        Entity *powerup = getCurrentEntity(&it);

        if (CheckCollisionRecs(powerup->bounds, game->ships[0].bounds)) {
            activatePowerup(game, powerup, 0);
        }

        iteratorNext(&it);
    }

    resetIterator(&it);
    while (!iteratorReachedEnd(&it)) {
        Entity *powerup = getCurrentEntity(&it);

        if (CheckCollisionRecs(powerup->bounds, game->ships[1].bounds)) {
            activatePowerup(game, powerup, 1);
        }

        iteratorNext(&it);
    }
}

void checkAlienBulletCollision(Game *game) {
    CollisionIterator it = createCollisionIterator(game->bullets, game->horde, game->nBullets, nRowsAliens*nColsAliens);
    int dropCheck = 15;
    int bulletIdx, alienIdx;

    while (!collisionIteratorReachedEnd(&it)) {
        if (checkPairCollision(&it, &bulletIdx, &alienIdx)) {
            game->bullets[bulletIdx].state = INACTIVE;
            game->horde[alienIdx].state    = DEAD;
            game->enemiesAlive--;
            playSoundFX(game, ALIEN_EXPLOSION_FX);
            if (rand() % 100 < dropCheck) {
                generatePowerup(&game->horde[alienIdx].bounds, game->powerups, game->nPowerups);
            }

            if (alienIdx == game->hordeLastAlive) {
                for (; game->hordeLastAlive > 0 && game->horde[game->hordeLastAlive].state == DEAD; --game->hordeLastAlive);
            }
        }

        collisionIteratorNext(&it);
    }
}

void checkShipBulletCollision(Game *game, int shipNumber) {
    EntitiesIterator it = createIterator(game->bullets, BULLETS_AT_SHIP, game->nBullets);

    while (game->ships[shipNumber].state == ACTIVE && !iteratorReachedEnd(&it)) {
        Entity *currBullet = getCurrentEntity(&it);
        if (CheckCollisionRecs(currBullet->bounds, game->ships[shipNumber].bounds)) {
            currBullet->state = INACTIVE;
            game->ships[shipNumber].state = DEAD;
            playSoundFX(game, SHIP_EXPLOSION_FX);
            break;
        }

        iteratorNext(&it);
    }

    if (game->ships[0].state == DEAD && game->ships[1].state == DEAD) {
        loseGame(game);
    }
}

void checkEnemyShipBulletCollision(Game *game) {
    if (game->enemyShip.state == ACTIVE) {
        EntitiesIterator it = createIterator(game->bullets, BULLETS_AT_ENEMIES, game->nBullets);
    
        while (!iteratorReachedEnd(&it)) {
            int bulletIdx = getCurrentIndex(&it);
            if (CheckCollisionRecs(game->bullets[bulletIdx].bounds, game->enemyShip.bounds)) {
                game->enemyShip.state = DEAD;
                game->bullets->state  = INACTIVE;
                game->enemiesAlive--;
                playSoundFX(game, SHIP_EXPLOSION_FX);
            }
    
            iteratorNext(&it);
        }
    }
}

void checkCollisions(Game *game) {
    checkAlienBulletCollision(game);
    checkEnemyShipBulletCollision(game);
    checkShipBulletCollision(game, 0);
    checkShipBulletCollision(game, 1);
    checkShipPowerupCollision(game);
}

void fire(Game* game, Entity *entity, int shipNumber) {
    switch (entity->type) {
        case SHIP:
        {
            ShipsTimers *shipsTimers = &game->hotData->shipsTimers;
            if (shipsTimers->remainingTimeToFire[shipNumber] <= 0.0) {
                generateBullet(&entity->bounds, game->bullets, true, game->nBullets);
                playSoundFX(game, SHIP_FIRE_FX);
                if (shipsTimers->remainingTimeFastShot[shipNumber] > 0.0) {
                    shipsTimers->remainingTimeToFire[shipNumber] = game->coldData->shipDelaysToFire[BUFFED];
                } else {
                    shipsTimers->remainingTimeToFire[shipNumber] = game->coldData->shipDelaysToFire[REGULAR];
                }
            }
        } break;
        case ENEMY_SHIP:
        {
            EnemyShipTimers *enemyShipTimers = &game->hotData->enemyShipTimers;
            generateBullet(&entity->bounds, game->bullets, false, game->nBullets);
            playSoundFX(game, SHIP_FIRE_FX);
            enemyShipTimers->remainingTimeToFire = game->coldData->enemyShipDelayToFire;
        } break;
        case ALIEN1:
        case ALIEN2:
        case ALIEN3:
        {
            generateBullet(&entity->bounds, game->bullets, false, game->nBullets);
            playSoundFX(game, ALIEN_FIRE_FX);
        } break;
        default: break;
    }
}

void updateShip(Game *game, Input *input, float deltaTime, int shipNumber) {
    if (game->ships[shipNumber].state != ACTIVE) return;

    ShipsTimers *shipsTimers = &game->hotData->shipsTimers;

    // NOTE: that can work if I simulate the projectiles properly
    shipsTimers->remainingTimeFastMove[shipNumber]   -= deltaTime;
    shipsTimers->remainingTimeFastShot[shipNumber]   -= deltaTime;
    shipsTimers->remainingTimeToFire[shipNumber]     -= deltaTime;

    if ((*input) & (1 << 2)) {
        if (shipsTimers->remainingTimeFastMove[shipNumber] > 0.0) {
            game->ships[shipNumber].bounds.x -= game->coldData->shipSpeeds[BUFFED] * deltaTime;
        } else {
            game->ships[shipNumber].bounds.x -= game->coldData->shipSpeeds[REGULAR] * deltaTime;
        }
    }

    if ((*input) & (1 << 3)) {
        if (shipsTimers->remainingTimeFastMove[shipNumber] > 0.0) {
            game->ships[shipNumber].bounds.x += game->coldData->shipSpeeds[BUFFED] * deltaTime;
        } else {
            game->ships[shipNumber].bounds.x += game->coldData->shipSpeeds[REGULAR] * deltaTime;
        }
    }

    if (game->ships[shipNumber].bounds.x <= game->coldData->screenLimits[LEFT]) {
        game->ships[shipNumber].bounds.x = game->coldData->screenLimits[LEFT];
    }
    if (game->ships[shipNumber].bounds.x + game->ships[0].bounds.width >= game->coldData->screenLimits[RIGHT]) {
        game->ships[shipNumber].bounds.x = game->coldData->screenLimits[RIGHT] - game->ships[shipNumber].bounds.width;
    }

    if ((*input) & (1 << 4)) {
        fire(game, &game->ships[shipNumber], shipNumber);
    }

    *input = 0;
}

void updateEnemyShip(Game *game, float deltaTime) {
    EnemyShipTimers *enemyShipTimers = &game->hotData->enemyShipTimers;
    if (game->enemyShip.state == INACTIVE) {
        enemyShipTimers->remainingTimeAlarm -= deltaTime;
        if (enemyShipTimers->remainingTimeAlarm <= 0.0f) {
            game->enemyShip.state = ACTIVE;
            manageMusic(game, PLAY_ENEMY_SHIP_MUSIC);
        }
    } else if (game->enemyShip.state == ACTIVE) {
        enemyShipTimers->remainingTimeToFire -= deltaTime;
        UpdateMusicStream(game->sounds->enemyShip);
        game->enemyShip.bounds.x += game->hotData->enemyShipSpeed * deltaTime;

        if (enemyShipTimers->remainingTimeToFire <= 0.0) {
            fire(game, &game->enemyShip, -1);
        }

        if (game->hotData->enemyShipSpeed > 0.0f) {
            if (game->enemyShip.bounds.x > game->screenWidth) {
                game->enemyShip.state = INACTIVE;
                game->hotData->enemyShipSpeed *= -1;
                manageMusic(game, STOP_ENEMY_SHIP_MUSIC);
                enemyShipTimers->remainingTimeAlarm = game->coldData->enemyShipSleepTime;
            }
        } else if (game->hotData->enemyShipSpeed < 0.0f) {
            if (game->enemyShip.bounds.x < game->coldData->screenLimits[LEFT]) {
                game->enemyShip.bounds.x = game->coldData->screenLimits[LEFT];
                game->hotData->enemyShipSpeed *= -1;
            }
        }
    }

}

float getAliensMaxHorizontalPos(EntitiesIterator *it) {
    if (!iteratorReachedEnd(it)) {
        float maxHorizontalPos = getCurrentEntity(it)->bounds.x;
        while (!iteratorReachedEnd(it)) {
            float currentX = getCurrentEntity(it)->bounds.x;
            maxHorizontalPos = currentX > maxHorizontalPos ? currentX : maxHorizontalPos; 
    
            iteratorNext(it);
        }
    
        return maxHorizontalPos;
    }
}

float getAliensMinHorizontalPos(EntitiesIterator *it) {
    if (!iteratorReachedEnd(it)) {
        float minHorizontalPos = getCurrentEntity(it)->bounds.x;
        while (!iteratorReachedEnd(it)) {
            float currentX = getCurrentEntity(it)->bounds.x;
            minHorizontalPos = currentX < minHorizontalPos ? currentX : minHorizontalPos;
    
            iteratorNext(it);
        }
    
        return minHorizontalPos;
    }
}

void updateHorde(Game *game, float deltaTime) {
    game->animation->timeRemainingToChangeFrame -= deltaTime;
    if (game->animation->timeRemainingToChangeFrame <= 0.0f) {
        game->animation->timeRemainingToChangeFrame = game->coldData->alienTimePerFrame;
        game->animation->alienCurrentFrame = (game->animation->alienCurrentFrame + 1) % 4;
        game->animation->aliensFrame.x = game->animation->alienCurrentFrame * game->animation->aliensFrame.width;
    }

    EntitiesIterator hordeIt = createIterator(game->horde, ALIENS, nRowsAliens*nColsAliens);
    int dropCheck = 1;

    // It's possible to look for the leftest and the rightest alien inside that while!!
    while (!iteratorReachedEnd(&hordeIt)) {
        Entity *current = getCurrentEntity(&hordeIt);
        if (rand() % 3000 < dropCheck) fire(game, current, -1);

        current->bounds.x += game->hotData->hordeSpeed * deltaTime;
        if (game->hotData->hordeDown) {
            current->bounds.y += game->coldData->hordeStepY;
        }
        iteratorNext(&hordeIt);
    }

    game->hotData->hordeDown = false;
    resetIterator(&hordeIt);

    // Checks collision with the screen bounds
    if (game->hotData->hordeSpeed > 0.0f) {
        float maxHorizontalPos = getAliensMaxHorizontalPos(&hordeIt);
        if (maxHorizontalPos + game->horde[0].bounds.width >= game->coldData->screenLimits[RIGHT]) {
            game->hotData->hordeSpeed += game->coldData->hordeSpeedIncrease;
            game->hotData->hordeSpeed *= -1;
            game->hotData->hordeDown = true;
        }
    } else if (game->hotData->hordeSpeed < 0.0f) {
        float minHorizontalPos = getAliensMinHorizontalPos(&hordeIt);
        if (minHorizontalPos <= game->coldData->screenLimits[LEFT]) {
            game->hotData->hordeSpeed -= game->coldData->hordeSpeedIncrease;
            game->hotData->hordeSpeed *= -1;
            game->hotData->hordeDown = true;
        }
    }

    // Lose when aliens reach player's ship level
    Entity *lastAlive = &game->horde[game->hordeLastAlive];
    if (lastAlive->bounds.y + lastAlive->bounds.height > game->ships[0].bounds.y) {
        loseGame(game);
    }
}

void updateProjectiles(Game *game, float deltaTime) {
    EntitiesIterator bulletsIt = createIterator(game->bullets, BULLETS, game->nBullets);
    EntitiesIterator powerupsIt = createIterator(game->powerups, POWERUPS, game->nPowerups);

    while (!iteratorReachedEnd(&bulletsIt)) {
        Entity *current = getCurrentEntity(&bulletsIt);
        if (current->up) {
            current->bounds.y -= game->coldData->projectileSpeed * deltaTime;
        } else {
            current->bounds.y += game->coldData->projectileSpeed * deltaTime;
        }

        if (current->bounds.y >= game->screenHeight || current->bounds.y <= -current->bounds.height) {
            current->state = INACTIVE;
        }

        iteratorNext(&bulletsIt);
    }

    while (!iteratorReachedEnd(&powerupsIt)) {
        Entity *current = getCurrentEntity(&powerupsIt);
        current->bounds.y += game->coldData->projectileSpeed * deltaTime;
        if (current->bounds.y >= game->screenHeight || current->bounds.y <= -current->bounds.height) {
            current->state = INACTIVE;
        }

        iteratorNext(&powerupsIt);
    }
}

void updateMenu(Game *game) {
    Input input = game->hotData->input;
    if ((input & 1) || (input & (1 << 1))) {
        switch (game->hotData->gameState) {
            case PLAYING: break;
            case MENU:
            case PAUSED:
            case WIN:
            case LOSE:
            {
                if (game->hotData->menuButton == START) {
                    game->hotData->menuButton = QUIT;
                } else if (game->hotData->menuButton == QUIT) {
                    game->hotData->menuButton = START;
                }
                playSoundFX(game, MENU_FX);
            } break;
            case CLOSE: break;
            default: break;
        }
    }
}

void updatePlayer2(Game *game, CommandsBufPlayer2 *commands, float delta) {
    for (int i = 0; i < commands->capacity; ++i) {
        updateShip(game, &commands->input[i], delta, 1);
        checkShipBulletCollision(game, 1);
    }
}

void updateGame(Game *game, CommandsBufPlayer2 *commandsPlayer2, float deltaTime) {
    game->soundEventsBuf->soundEvents[game->soundEventsBuf->currentIdx] = 0;

    switch (game->hotData->gameState) {
        case PLAYING:
        {
            UpdateMusicStream(game->sounds->background);

            if (game->hotData->input & (1 << 6)) {
                game->hotData->gameState = PAUSED;
            }
            checkCollisions(game);
            updateShip(game, &game->hotData->input, deltaTime, 0);
            updatePlayer2(game, commandsPlayer2, deltaTime);
            updateEnemyShip(game, deltaTime);
            updateHorde(game, deltaTime);
            updateProjectiles(game, deltaTime);

            if (game->enemiesAlive <= 0) {
                game->hotData->gameState = WIN;
                manageMusic(game, STOP_BACKGROUND_MUSIC);
                playSoundFX(game, VICTORY_FX);
            }
        } break;
        case MENU:
        case PAUSED:
        {
            UpdateMusicStream(game->sounds->background);
            updateMenu(game);
            if (game->hotData->input & (1 << 5)) {
                if (game->hotData->menuButton == START) {
                    game->hotData->gameState = PLAYING;
                    manageMusic(game, PLAY_BACKGROUND_MUSIC);
                } else {
                    game->hotData->gameState = CLOSE;
                }
            }
        } break;
        case WIN:
        case LOSE:
        {
            updateMenu(game);
            if (game->hotData->input & (1 << 5)) {
                if (game->hotData->menuButton == START) {
                    rebootGame(game);
                } else {
                    game->hotData->gameState = CLOSE;
                }
            }
        } break;
        case CLOSE: break;
        default: break;
    }

    game->soundEventsBuf->currentIdx = (game->soundEventsBuf->currentIdx + 1) % CAP_SOUND_EVENT_BUF;
}

void processMusic(Game *game, SnapshotGameState *snap) {
    UpdateMusicStream(game->sounds->background);
    UpdateMusicStream(game->sounds->enemyShip);

    if ((snap->musicEvents & 1) && !IsMusicStreamPlaying(game->sounds->background)) {
        PlayMusicStream(game->sounds->background);
    }

    if (!(snap->musicEvents & 1) && IsMusicStreamPlaying(game->sounds->background)) {
        StopMusicStream(game->sounds->background);
    }

    if ((snap->musicEvents & (1 << 1)) && !IsMusicStreamPlaying(game->sounds->enemyShip)) {
        PlayMusicStream(game->sounds->enemyShip);
    }

    if (!(snap->musicEvents & (1 << 1)) && IsMusicStreamPlaying(game->sounds->enemyShip)) {
        StopMusicStream(game->sounds->enemyShip);
    }

    snap->musicEvents = 0;
}

void processSoundFX(Game *game, SnapshotGameState *snap) {
    for (int i = 0; i < CAP_SOUND_EVENT_BUF; ++i) {
        if (snap->soundEvents[i] & 1) {
            PlaySound(game->sounds->alienExplosion);
        }

        if (snap->soundEvents[i] & (1 << 1)) {
            PlaySound(game->sounds->alienFire);
        }

        if (snap->soundEvents[i] & (1 << 2)) {
            PlaySound(game->sounds->lose);
        }

        if (snap->soundEvents[i] & (1 << 3)) {
            PlaySound(game->sounds->menu);
        }

        if (snap->soundEvents[i] & (1 << 4)) {
            PlaySound(game->sounds->powerup);
        }

        if (snap->soundEvents[i] & (1 << 5)) {
            PlaySound(game->sounds->shipExplosion);
        }

        if (snap->soundEvents[i] & (1 << 6)) {
            PlaySound(game->sounds->shipFire);
        }

        if (snap->soundEvents[i] & (1 << 7)) {
            PlaySound(game->sounds->victory);
        }

        snap->soundEvents[i] = 0;
    }
}
