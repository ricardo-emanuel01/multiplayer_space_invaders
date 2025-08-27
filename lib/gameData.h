#ifndef _GAME_DATA_H_
#define _GAME_DATA_H_

#include <arpa/inet.h>
#include <raylib.h>
#include <stdint.h>

#include "entity.h"
#include "render.h"

#define Input uint8_t
#define SoundEvents uint8_t
#define MusicEvents uint8_t
#define N_ENTITIES 118
#define CAP_SOUND_EVENT_BUF 3

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

typedef enum ScreenLimit {
    LEFT,
    RIGHT,
} ScreenLimit;

typedef enum SoundSelect {
    ALIEN_EXPLOSION_FX = 0,
    ALIEN_FIRE_FX,
    LOSE_FX,
    MENU_FX,
    POWERUP_FX,
    SHIP_EXPLOSION_FX,
    SHIP_FIRE_FX,
    VICTORY_FX,
} SoundSelect;

typedef enum MusicSelect {
    PLAY_BACKGROUND_MUSIC,
    STOP_BACKGROUND_MUSIC,
    PLAY_ENEMY_SHIP_MUSIC,
    STOP_ENEMY_SHIP_MUSIC,
} MusicSelect;

typedef struct CommandsBufPlayer2 {
    Input *input;
    int capacity;
    int size;
} CommandsBufPlayer2;

typedef struct SoundEventsBuf {
    SoundEvents *soundEvents;
    int currentIdx;
} SoundEventsBuf;

typedef struct Remote {
    int remote_fd;
    struct sockaddr_in host_addr;
} Remote;

typedef struct Host {
    int host_fd, remote_fd;
    struct sockaddr_in host_addr, remote_addr;
    socklen_t remote_addr_len;
} Host;

// Used to send the remote host the entities to draw
typedef struct EntityBounds {
    // Sends height and width to get around the ID
    uint16_t x, y;
} EntityBounds;

// TODO: send or predict the frame of the aliens
typedef struct SnapshotGameState {
    /**
     * 22 alien1s, 11 alien2s, 22 alien 3s, 1 enemy ship, 2 players,
     * 10 fast shots, 10 fast moves and 40 bullets
     */
    EntityBounds entities[N_ENTITIES];
    GameState gameState;
    MenuButton menuButton;
    SoundEvents soundEvents[CAP_SOUND_EVENT_BUF];
    MusicEvents musicEvents;
} SnapshotGameState;

typedef struct ColdGameData {
    float shipSpeeds[2];
    float shipDelaysToFire[2];
    float screenLimits[2];
    float enemyShipDelayToFire;
    float projectileSpeed;
    float powerupDuration;
    float hordeSpeedIncrease;
    float hordeStepY;
    float enemyShipSleepTime;
    float alienTimePerFrame;
} ColdGameData;

typedef struct ShipsTimers {
    float remainingTimeFastShot[2];
    float remainingTimeFastMove[2];
    float remainingTimeToFire[2];
} ShipsTimers;

typedef struct EnemyShipTimers {
    float remainingTimeAlarm;
    float remainingTimeToFire;
} EnemyShipTimers;

typedef struct HotGameData {
    ShipsTimers     shipsTimers;
    EnemyShipTimers enemyShipTimers;
    float           enemyShipSpeed;
    float           hordeSpeed;
    GameState       gameState;
    MenuButton      menuButton;
    // TODO: Revisit that name and logic
    bool            hordeDown;
    Input           input;
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
    int alienCurrentFrame;
} Animation;

typedef struct Game {
    Entity          enemyShip;
    int             screenHeight;
    Entity*         ships;
    Entity*         horde;
    Entity*         bullets;
    Entity*         powerups;
    ColdGameData*   coldData;
    HotGameData*    hotData;
    Sounds*         sounds;
    Textures*       textures;
    Animation*      animation;
    SoundEventsBuf* soundEventsBuf;
    int             screenWidth;
    uint16_t        nBullets;
    uint16_t        nPowerups;
    uint16_t        enemiesAlive;
    uint8_t         hordeLastAlive;
    MusicEvents     musicEvents;
} Game;

void initGame(Game *game);
void rebootGame(Game* game);
void cleanupGame(Game *game);
void buildSnapshot(Game *game, SnapshotGameState *);
CommandsBufPlayer2 *initCommandsBuf(int capacity);
void cleanupCommandsBuf(CommandsBufPlayer2 **buf);
SoundEventsBuf *initSoundEventsBuf(int capacity);
void cleanupSoundEventsBuf(SoundEventsBuf **buf);
void addSound(SoundEventsBuf *, SoundSelect);

#endif