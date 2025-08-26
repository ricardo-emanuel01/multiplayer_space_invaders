# include <stdlib.h>
# include <raylib.h>
# include <string.h>

# include "entity.h"
# include "gameData.h"


ColdGameData *initColdGameData() {
    const float shipSpeeds[]       = {300.0f, 450.0f};
    const float shipDelaysToFire[] = {0.5f, 0.1f};
    const float screenLimits[]     = {250.0f, 1670.0f};

    ColdGameData *
    gameData = (ColdGameData *)malloc(sizeof(ColdGameData));
    *gameData = (ColdGameData) {
        .enemyShipDelayToFire = 0.25f,
        .projectileSpeed      = 600.0f,
        .powerupDuration      = 2.0f,
        .hordeSpeedIncrease   = 25.0f,
        .hordeStepY           = 100.0f,
        .enemyShipSleepTime   = 4.0f,
        .alienTimePerFrame    = 0.1f,
    };

    memcpy(
        &gameData->shipSpeeds,
        shipSpeeds,
        2*sizeof(float)
    );

    memcpy(
        &gameData->shipDelaysToFire,
        shipDelaysToFire,
        2*sizeof(float)
    );

    memcpy(&gameData->screenLimits, screenLimits, 2*sizeof(float));

    return gameData;
}

HotGameData *initHotGameData() {
    HotGameData *
    gameData = (HotGameData *)malloc(sizeof(HotGameData));
    *gameData = (HotGameData){
        .hordeSpeed                   = 100.0f,
        .enemyShipSpeed               = -450.0f,
        .gameState                    = MENU,
        .menuButton                   = START,
        .input                        = (Input *)calloc(2, sizeof(Input)),
        .shipsTimers = {
            .remainingTimeFastMove = {0.0f, 0.0f},
            .remainingTimeFastShot = {0.0f, 0.0f},
            .remainingTimeToFire   = {0.0f, 0.0f}
        },
        .enemyShipTimers = {
            .remainingTimeAlarm  = 4.0f,
            .remainingTimeToFire = 0.0f
        }
    };

    return gameData;
}

Sounds *initSounds() {
    Sounds *sounds = (Sounds *)malloc(sizeof(Sounds));
    *sounds = (Sounds){
        .background     = LoadMusicStream("assets/sounds/background.ogg"),
        .enemyShip      = LoadMusicStream("assets/sounds/enemyShip.ogg"),
        .shipFire       = LoadSound("assets/sounds/shipFire.ogg"),
        .alienFire      = LoadSound("assets/sounds/alienFire.ogg"),
        .shipExplosion  = LoadSound("assets/sounds/shipExplosion.ogg"),
        .alienExplosion = LoadSound("assets/sounds/alienExplosion.ogg"),
        .powerup        = LoadSound("assets/sounds/powerup.ogg"),
        .lose           = LoadSound("assets/sounds/lose.ogg"),
        .victory        = LoadSound("assets/sounds/victory.ogg"),
        .menu           = LoadSound("assets/sounds/menu.ogg"),
    };

    return sounds;
}

void cleanupSounds(Sounds **sounds) {
    UnloadMusicStream((*sounds)->background);
    UnloadMusicStream((*sounds)->enemyShip);
    UnloadSound((*sounds)->shipFire);
    UnloadSound((*sounds)->alienFire);
    UnloadSound((*sounds)->shipExplosion);
    UnloadSound((*sounds)->alienExplosion);
    UnloadSound((*sounds)->powerup);
    UnloadSound((*sounds)->lose);
    UnloadSound((*sounds)->victory);
    UnloadSound((*sounds)->menu);
    free(*sounds);
    *sounds = NULL;
}

Textures *initTextures() {
    Textures *tex = (Textures *)malloc(sizeof(Textures));
    *tex = (Textures) {
        .ship        = LoadTexture("assets/textures/ship.png"),
        .enemyShip   = LoadTexture("assets/textures/enemyShip.png"),
        .alien1      = LoadTexture("assets/textures/alien1.png"),
        .alien2      = LoadTexture("assets/textures/alien2.png"),
        .alien3      = LoadTexture("assets/textures/alien3.png"),
        .bullet      = LoadTexture("assets/textures/bullet.png"),
        .shotPowerup = LoadTexture("assets/textures/shotPowerup.png"),
        .movePowerup = LoadTexture("assets/textures/movePowerup.png"),
    };

    return tex;
}

void cleanupTextures(Textures **textures) {
    UnloadTexture((*textures)->ship);
    UnloadTexture((*textures)->enemyShip);
    UnloadTexture((*textures)->alien1);
    UnloadTexture((*textures)->alien2);
    UnloadTexture((*textures)->alien3);
    UnloadTexture((*textures)->bullet);
    UnloadTexture((*textures)->shotPowerup);
    UnloadTexture((*textures)->movePowerup);

    free(*textures);
    *textures = NULL;
}

Animation *initAnimation() {
    Animation *animation = (Animation *)malloc(sizeof(Animation));
    *animation = (Animation) {
        .aliensFrame    = {.height = 16.0f, .width = 16.0f, .x = 0.0f, .y = 0.0f},
        .shipFrame      = {.height = 12.0f, .width = 16.0f, .x = 0.0f, .y = 0.0f},
        .bulletFrame    = {.height = 8.0f, .width = 4.0f, .x = 0.0f, .y =0.0f},
        .enemyShipFrame = {.height = 10.0f, .width = 16.0f, .x = 0.0f, .y = 0.0f},
        .powerupFrame   = {.height = 18.0f, .width = 18.0f, .x = 0.0f, .y = 0.0f},
        .timeRemainingToChangeFrame = 0.1f,
        .alienCurrentFrame          = 0,
    };

    return animation;
}

void cleanupAnimation(Animation **animation) {
    free(*animation);
    *animation = NULL;
}

void initGame(Game *game) {
    uint16_t nPowerups = 20;
    uint16_t nBullets  = 40;
    *game = (Game) {
        .ships          = createPlayerShips(),
        .enemyShip      = createEnemyShip(),
        .horde          = createHorde(),
        .bullets        = createBulletsArray(nBullets),
        .powerups       = createPowerupsArray(nPowerups),
        .coldData       = initColdGameData(),
        .hotData        = initHotGameData(),
        .sounds         = initSounds(),
        .textures       = initTextures(),
        .animation      = initAnimation(),
        .nBullets       = nBullets,
        .nPowerups      = nPowerups,
        // plus 1 from the enemy ship
        .enemiesAlive   = nRowsAliens*nColsAliens + 1,
        .hordeLastAlive = nRowsAliens*nColsAliens - 1,
        .screenHeight   = 1080.0f,
        .screenWidth    = 1920.0f
    };

    game->sounds->background.looping = true;
    game->sounds->enemyShip.looping = true;
}

void cleanupGame(Game *game) {
    cleanupSounds(&game->sounds);
    cleanupTextures(&game->textures);
    cleanupAnimation(&game->animation);
    destroyHorde(&game->horde);
    destroyBullets(&game->bullets);
    destroyPowerups(&game->powerups);

    free(game->hotData->input);
    free(game->hotData);
    free(game->coldData);
    free(game->ships);
    game->hotData = NULL;
    game->coldData = NULL;
}

void rebootGame(Game *game) {
    cleanupGame(game);
    initGame(game);
    game->hotData->gameState = PLAYING;
}

void buildSnapshot(Game *game, SnapshotGameState *snap) {
    snap->gameState = game->hotData->gameState;
    snap->menuButton = game->hotData->menuButton;

    memset(&snap->entities, 0, N_ENTITIES * sizeof(EntityBounds));
    int j = 0;
    for (int i = 0; i < 55; ++i) {
        if (game->horde[i].state == ACTIVE) {
            snap->entities[j++] = (EntityBounds) {
                .x = htons((uint16_t)game->horde[i].bounds.x),
                .y = htons((uint16_t)game->horde[i].bounds.y)
            };
        } else j++;
    }

    if (game->enemyShip.state == ACTIVE) {
        snap->entities[j++] = (EntityBounds) {
            .x = htons((uint16_t)game->enemyShip.bounds.x),
            .y = htons((uint16_t)game->enemyShip.bounds.y)
        };
    } else j++;

    if (game->ships[0].state == ACTIVE) {
        snap->entities[j++] = (EntityBounds) {
            .x = htons((uint16_t)game->ships[0].bounds.x),
            .y = htons((uint16_t)game->ships[0].bounds.y)
        };
    } else j++;

    if (game->ships[1].state == ACTIVE) {
        snap->entities[j++] = (EntityBounds) {
            .x = htons((uint16_t)game->ships[1].bounds.x),
            .y = htons((uint16_t)game->ships[1].bounds.y)
        };
    } else j++;

    for (int i = 0; i < game->nPowerups; ++i) {
        if (game->powerups[i].state == ACTIVE) {
            snap->entities[j++] = (EntityBounds) {
                .x = htons((uint16_t)game->powerups[i].bounds.x),
                .y = htons((uint16_t)game->powerups[i].bounds.y)
            };
        } else j++;
    }

    for (int i = 0; i < game->nBullets; ++i) {
        if (game->bullets[i].state == ACTIVE) {
            snap->entities[j++] = (EntityBounds) {
                .x = htons((uint16_t)game->bullets[i].bounds.x),
                .y = htons((uint16_t)game->bullets[i].bounds.y)
            };
        } else j++;
    }
}

Player2CommandsBuf *initCommandsBuf(int capacity) {
    Player2CommandsBuf *commands = (Player2CommandsBuf *)malloc(sizeof(Player2CommandsBuf));
    *commands = (Player2CommandsBuf) {
        .capacity = capacity,
        .size = 0,
        .input = (Input *)malloc(capacity * sizeof(Input))
    };

    return commands;
}

void cleanupCommandsBuf(Player2CommandsBuf **buf) {
    free((*buf)->input);
    free(*buf);
    *buf = NULL;
}
