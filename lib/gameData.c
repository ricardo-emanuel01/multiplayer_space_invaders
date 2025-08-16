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
    *gameData = (ColdGameData){
        .enemyShipDelayToFire = 0.25f,
        .enemyShipSpeed       = 450.0f,
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
        .lastFrameTime               = GetTime(),
        .hordeSpeed                  = 100.0f,
        .gameState                   = PAUSED,
        .menuButton                  = START,
        .enemyShipGoingLeft          = true,
        .shipLastShotTime            = 0.0,
        .remainingTimeEnemyShipAlarm = 4.0f,
        .enemyShipLastShotTime       = 0.0f,
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
        .enemyCurrentFrame          = 0,
    };

    return animation;
}

void cleanupAnimation(Animation **animation) {
    free(*animation);
    *animation = NULL;
}

void initGame(Game *game) {
    uint16_t nProjectiles = 1000;
    *game = (Game) {
        .ship           = createPlayerShip(),
        .enemyShip      = createEnemyShip(),
        .horde          = createHorde(),
        .bullets        = createBulletsArray(nProjectiles),
        .powerups       = createPowerupsArray(nProjectiles),
        .coldData       = initColdGameData(),
        .hotData        = initHotGameData(),
        .sounds         = initSounds(),
        .textures       = initTextures(),
        .animation      = initAnimation(),
        .hordeLastAlive = nRowsAliens * nColsAliens - 1,
        .nBullets       = nProjectiles,
        .nPowerups      = nProjectiles,
        .enemiesAlive   = 1 + nRowsAliens*nColsAliens,
        .hordeLastAlive = nRowsAliens*nColsAliens - 1,
    };

    game->sounds->background.looping = true;
    game->sounds->enemyShip.looping = true;
    PlayMusicStream(game->sounds->background);
}

void cleanupGame(Game *game) {
    cleanupSounds(&game->sounds);
    cleanupTextures(&game->textures);
    cleanupAnimation(&game->animation);
    destroyHorde(&game->horde);
    destroyBullets(&game->bullets);
    destroyPowerups(&game->powerups);

    free(game->hotData);
    free(game->coldData);
    game->hotData = NULL;
    game->coldData = NULL;
}

void rebootGame(Game *game) {
    cleanupGame(game);
    initGame(game);
    game->hotData->gameState = CONNECTING;
}
