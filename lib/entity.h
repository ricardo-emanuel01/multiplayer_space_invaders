#ifndef _ENTITY_H_
#define _ENTITY_H_

#include <raylib.h>
#include <stdint.h>

#define nRowsAliens 5
#define nColsAliens 11


typedef enum EntityType {
    SHIP,
    ENEMY_SHIP,
    ALIEN1,
    ALIEN2,
    ALIEN3,
    BULLET,
    FAST_SHOT,
    FAST_MOVE,
} EntityType;

typedef enum EntityState {
    ACTIVE,
    // Used to stop the updating of the enemy ship
    INACTIVE,
    DEAD,
} EntityState;

typedef enum IteratorType {
    BULLETS,
    BULLETS_AT_SHIP,
    BULLETS_AT_ENEMIES,
    ALIENS,
    POWERUPS,
} IteratorType;

typedef struct Entity {
    // Each type of entity will have the same width and height, make sense to use a Rectangle?
    // What would be the tradeoffs of using a Vector2?
    Rectangle bounds;
    EntityType type;
    /*
        Used for the player to know how to update bullets (and powerups),
        if it's true the entity is going up and down otherwise.
    */
    EntityState state;
    bool up;
} Entity;

typedef struct EntitiesIterator {
    Entity *entities;
    IteratorType type;
    uint16_t size;
    uint16_t currentIndex;
} EntitiesIterator;

// Will be used to check the collision of bullets and aliens
typedef struct CollisionIterator {
    EntitiesIterator bullets;
    EntitiesIterator aliens;
} CollisionIterator;

EntitiesIterator createIterator(Entity *, IteratorType, uint16_t size);
void resetIterator(EntitiesIterator *it);
bool iteratorReachedEnd(EntitiesIterator *it);
int getCurrentIndex(EntitiesIterator *it);
void iteratorNext(EntitiesIterator *it);
Entity *getCurrentEntity(EntitiesIterator *it);

CollisionIterator createCollisionIterator(Entity *bullets, Entity *horde, uint16_t sizeBullets, uint16_t sizeHorde);
bool collisionIteratorReachedEnd(CollisionIterator *it);
void collisionIteratorNext(CollisionIterator *it);

Entity *createPlayerShips();
void destroyPlayerShips(Entity **ships);
Entity createEnemyShip();
Entity *createHorde();
void destroyHorde(Entity **horde);
Entity *createBulletsArray(int n);
void destroyBullets(Entity **bullets);
Entity *createPowerupsArray(int n);
void destroyPowerups(Entity **powerups);
void generateBullet(Rectangle *shooterBounds, Entity *bullets, bool up, int n);

// The name of the Rectangle variable can improve
void generatePowerup(Rectangle *bounds, Entity *powerups, int n);

#endif
