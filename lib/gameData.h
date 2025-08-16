# ifndef _GAME_DATA_H_
# define _GAME_DATA_H_

# include <raylib.h>
# include <stdint.h>

# include "entity.h"
# include "render.h"

# define Input uint8_t

typedef enum GameState {
    CONNECTING,
    MENU,
    PLAYING,
    PAUSED,
    LOSE,
    WIN,
    CLOSE,
} GameState;

typedef enum Ship {
    REGULAR,
    BUFFED,
} Ship;

typedef enum MenuButton {
    START,
    QUIT,
} MenuButton;

// NOTE: That should be here??
// typedef struct TextButton {
//     Font font;
//     Rectangle bounds;
//     const char text[8];
//     float fontSize;
//     VerticalAlignment vAlignment;
// } TextButton;

typedef struct ColdGameData {
    float shipSpeeds[2];
    float shipDelaysToFire[2];
    float screenLimits[2];
    float enemyShipDelayToFire;
    float enemyShipSpeed;
    float projectileSpeed;
    float powerupDuration;
    float hordeSpeedIncrease;
    float hordeStepY;
    float enemyShipSleepTime;
    float alienTimePerFrame;
} ColdGameData;

typedef struct HotGameData {
    double fastShotRemainingTime;
    double fastMoveRemainingTime;
    double remainingTimeEnemyShipAlarm;
    double lastFrameTime;
    double shipLastShotTime;
    double enemyShipLastShotTime;
    float hordeSpeed;
    GameState gameState;
    MenuButton menuButton;
    Input input;
    bool enemyShipGoingLeft;
} HotGameData;

typedef struct Sounds {
    Music background;
    Music enemyShip;
    Sound shipFire;
    Sound alienFire;
    Sound shipExplosion;
    Sound alienExplosion;
    Sound powerup;
    Sound lose;
    Sound victory;
    Sound menu;
} Sounds;

typedef struct Textures {
    Texture2D ship;
    Texture2D enemyShip;
    Texture2D alien1;
    Texture2D alien2;
    Texture2D alien3;
    Texture2D bullet;
    Texture2D shotPowerup;
    Texture2D movePowerup;
} Textures;

typedef struct Animation {
    Rectangle aliensFrame;
    Rectangle shipFrame;
    Rectangle bulletFrame;
    Rectangle enemyShipFrame;
    Rectangle powerupFrame;
    float timeRemainingToChangeFrame;
    int enemyCurrentFrame;
} Animation;

typedef struct Game {
    Entity ship;
    Entity enemyShip;
    Entity *horde;
    Entity *bullets;
    Entity *powerups;
    ColdGameData *coldData;
    HotGameData *hotData;
    Sounds *sounds;
    Textures *textures;
    Animation *animation;
    float screenHeight;
    float screenWidth;
    uint16_t nBullets;
    uint16_t nPowerups;
    uint16_t enemiesAlive;
    uint8_t hordeLastAlive;
} Game;

# endif