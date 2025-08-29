#include "entity.h"

#include <raylib.h>
#include <stdint.h>
#include <stdlib.h>


EntitiesIterator createIterator(
    Entity *entities, IteratorType type, uint16_t size
) {
    EntitiesIterator it = {
        .entities     = entities,
        .type         = type,
        .size         = size,
    };
    resetIterator(&it);

    return it;
}

void resetIterator(EntitiesIterator *it) {
    switch (it->type) {
        case BULLETS_AT_SHIP:
        {
            it->currentIndex = 0;
            while (!iteratorReachedEnd(it) && (getCurrentEntity(it)->state != ACTIVE || getCurrentEntity(it)->up)) {
                it->currentIndex++;
            }
        } break;
        case BULLETS_AT_ENEMIES:
        {
            it->currentIndex = 0;
            while (!iteratorReachedEnd(it) && (getCurrentEntity(it)->state != ACTIVE || !getCurrentEntity(it)->up)) {
                it->currentIndex++;
            }
        } break;
        case ALIENS:
        case POWERUPS:
        case BULLETS:
        {
            it->currentIndex = 0;
            while (!iteratorReachedEnd(it) && getCurrentEntity(it)->state != ACTIVE) {
                it->currentIndex++;
            }
        } break;
        default: break;
    }
}

bool iteratorReachedEnd(EntitiesIterator *it) {
    return it->currentIndex >= it->size;
}

int getCurrentIndex(EntitiesIterator *it) {
    if (!iteratorReachedEnd(it)) {
        return it->currentIndex;
    }

    return -1;
}

void iteratorNext(EntitiesIterator *it) {
    if (!iteratorReachedEnd(it)) {
        it->currentIndex++;
        switch (it->type) {
            case BULLETS_AT_SHIP:
            {
                while (!iteratorReachedEnd(it) && (getCurrentEntity(it)->state != ACTIVE || getCurrentEntity(it)->up)) {
                    it->currentIndex++;
                }
            } break;
            case BULLETS_AT_ENEMIES:
            {
                while (!iteratorReachedEnd(it) && (getCurrentEntity(it)->state != ACTIVE || !getCurrentEntity(it)->up)) {
                    it->currentIndex++;
                }
            } break;
            case BULLETS:
            case ALIENS:
            case POWERUPS:
            {
                while (!iteratorReachedEnd(it) && getCurrentEntity(it)->state != ACTIVE) {
                    it->currentIndex++;
                }
            } break;
            default: break;
        }
    }
}

Entity *getCurrentEntity(EntitiesIterator *it) {
    if (iteratorReachedEnd(it)) return NULL;
    int index = getCurrentIndex(it);
    return &it->entities[index];
}

CollisionIterator createCollisionIterator(
    Entity *bullets, Entity *horde, uint16_t sizeBullets, uint16_t sizeHorde
) {
    return (CollisionIterator) {
        .bullets = createIterator(bullets, BULLETS_AT_ENEMIES, sizeBullets),
        .aliens  = createIterator(horde, ALIENS, sizeHorde),
    };
}

bool collisionIteratorReachedEnd(CollisionIterator *it) {
    return (iteratorReachedEnd(&it->bullets) || iteratorReachedEnd(&it->aliens));
}

bool checkPairCollision(
    CollisionIterator *it, int *bulletIdx, int *alienIdx
) {
    *bulletIdx = getCurrentIndex(&it->bullets);
    *alienIdx  = getCurrentIndex(&it->aliens);

    if (bulletIdx != NULL && alienIdx != NULL) {
        return CheckCollisionRecs(it->bullets.entities[*bulletIdx].bounds, it->aliens.entities[*alienIdx].bounds);
    }
    return false;
}

void collisionIteratorNext(CollisionIterator *it) {
    Entity *currentBullet = getCurrentEntity(&it->bullets);
    Entity *currentAlien  = getCurrentEntity(&it->aliens);
    if (currentBullet != NULL && currentAlien != NULL) {
        if (getCurrentEntity(&it->bullets)->state == INACTIVE && getCurrentEntity(&it->aliens)->state == DEAD)  {
            iteratorNext(&it->bullets);
            resetIterator(&it->aliens);
        } else {
            iteratorNext(&it->aliens);
            if (!iteratorReachedEnd(&it->bullets) && iteratorReachedEnd(&it->aliens)) {
                resetIterator(&it->aliens);
                iteratorNext(&it->bullets);
            }
        }
    }
}

Entity *createPlayerShips() {
    const float height = 72.0f;
    const float width = 96.0f;
    const float x = 912.0f;
    const float y = 900.0f;

    Entity *ships = (Entity *)malloc(2 * sizeof(Entity));
    for (int i = 0; i < 2; ++i) {
        ships[i] = (Entity) {
            .bounds = {
                .height = height,
                .width  = width,
                .y      = y
            },
            .state = ACTIVE,
            .type  = SHIP
        };
        if (i == 0) ships[i].bounds.x = x - width;
        else ships[i].bounds.x = x + width;
    }

    return ships;
}

void destroyPlayerShips(Entity **ships) {
    free(*ships);
    *ships = NULL;
}

Entity createEnemyShip() {
    const float height = 40.0f;
    const float width = 64.0f;
    const float x = 1920.0f;
    const float y = 50.0f;

    Entity enemyShip = {
        .bounds = {
            .height = height,
            .width  = width,
            .x      = x,
            .y      = y,
        },
        .type  = ENEMY_SHIP,
        .state = INACTIVE,
    };

    return enemyShip;
}


Entity *createHorde() {
    const int sizeHorde = nRowsAliens * nColsAliens;
    const float height = 32.0f;
    const float width = 32.0f;
    const float gapX = 15.0f;
    const float gapY = 20.0f;
    const float offSetX = 1920.0f/2.0f - (width*(float)nColsAliens + gapX*((float)nColsAliens - 1.0f))/2.0f;
    const float offSetY = height*3.0f;
    float x, y;

    Entity *horde = (Entity *)malloc(sizeHorde * sizeof(Entity));

    for (int i = 0; i < sizeHorde; ++i) {
        x = offSetX + ((i % nColsAliens)*(width + gapX));
        y = offSetY + ((i / nColsAliens)*(height + gapY));

        horde[i].bounds = (Rectangle){.height = height, .width = width, .x = x, .y = y};
        horde[i].state  = ACTIVE;
        if (i / nColsAliens < 2) horde[i].type = ALIEN1;
        else if (i / nColsAliens < 3) horde[i].type = ALIEN2;
        else horde[i].type = ALIEN3;
    }

    return horde;
}

void destroyHorde(Entity **horde) {
    free(*horde);
    *horde = NULL;
}

// Create an array of size n of entities which the type field is BULLET
Entity *createBulletsArray(int n) {
    const float height = 32.0f;
    const float width = 4.0f;

    Entity *bullets = (Entity *)malloc(n * sizeof(Entity));

    for (int i = 0; i < n; ++i) {
        bullets[i] = (Entity) {
            .bounds = {.height = height, .width = width},
            .state  = INACTIVE,
            .type   = BULLET,
        };
    }

    return bullets;
}

void destroyBullets(Entity **bullets) {
    free(*bullets);
    *bullets = NULL;
}

Entity *createPowerupsArray(int n) {
    const float height = 25.0f;

    Entity *powerups = (Entity *)malloc(n * sizeof(Entity));

    for (int i = 0; i < n; ++i) {
        // The position and the direction of the bullet will be setted in the activation.
        powerups[i] = (Entity) {
            .bounds = {.height = height, .width = height},
            .state  = INACTIVE,
        };
    }

    return powerups;
}

void destroyPowerups(Entity **powerups) {
    free(*powerups);
    *powerups = NULL;
}

void generateBullet(Rectangle *shooterBounds, Entity *bullets, bool up, int n) {
    int i;
    for (i = 0; i < n && bullets[i].state == ACTIVE; ++i);
    if (i < n) {
        Vector2 position = {
            .x = shooterBounds->x + (shooterBounds->width - bullets[i].bounds.width) / 2.0f,
            .y = shooterBounds->y
        };

        if (up) position.y -= bullets[i].bounds.height;
        else position.y += shooterBounds->height;

        bullets[i].bounds.x = position.x;
        bullets[i].bounds.y = position.y;
        bullets[i].up       = up;
        bullets[i].state    = ACTIVE;
    }
}

void generatePowerup(Rectangle *bounds, Entity *powerups, int n) {
    int newPowerupIdx = 0;
    EntityType powerupType;
    if ((rand() % 100) < 50) powerupType = FAST_MOVE;
    else powerupType = FAST_SHOT;

    switch (powerupType) {
        case FAST_MOVE:
        {
            for (int i = 0; i < n / 2; ++i) {
                if (powerups[i].state != ACTIVE) {
                    newPowerupIdx = i;
                    break;
                }
            }
        } break;
        case FAST_SHOT:
        {
            for (int i = n / 2; i < n; ++i) {
                if (powerups[i].state != ACTIVE) {
                    newPowerupIdx = i;
                    break;
                }
            }
        } break;
        default: break;
    }

    if (newPowerupIdx < n) {
        Vector2 position = {
            .x = bounds->x + (bounds->width - powerups[newPowerupIdx].bounds.width) / 2.0f,
            .y = bounds->y + bounds->height
        };

        powerups[newPowerupIdx].bounds.x = position.x;
        powerups[newPowerupIdx].bounds.y = position.y;
        powerups[newPowerupIdx].up       = false;
        powerups[newPowerupIdx].state    = ACTIVE;
        powerups[newPowerupIdx].type     = powerupType;
    }
}
