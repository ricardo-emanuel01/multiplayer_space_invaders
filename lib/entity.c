# include <raylib.h>
# include <stdlib.h>

# include <raylib.h>
# include <stdint.h>
# include "entity.h"


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
            for (it->currentIndex = 0; it->currentIndex < it->size && it->entities[it->currentIndex].state != ACTIVE && it->entities[it->currentIndex].up; ++it->currentIndex);
        } break;
        case BULLETS_AT_ENEMIES:
        {
            for (it->currentIndex = 0; it->currentIndex < it->size && it->entities[it->currentIndex].state != ACTIVE && !it->entities[it->currentIndex].up; ++it->currentIndex);
        } break;
        case ALIENS:
        case POWERUPS:
        {
            for (it->currentIndex = 0; it->currentIndex < it->size && it->entities[it->currentIndex].state != ACTIVE; ++it->currentIndex);
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
        switch (it->type) {
            case BULLETS_AT_SHIP:
            {
                for (; it->currentIndex < it->size && it->entities[it->currentIndex].state != ACTIVE && it->entities[it->currentIndex].up; ++it->currentIndex);
            } break;
            case BULLETS_AT_ENEMIES:
            {
                for (; it->currentIndex < it->size && it->entities[it->currentIndex].state != ACTIVE && !it->entities[it->currentIndex].up; ++it->currentIndex);
            } break;
            case BULLETS:
            case ALIENS:
            case POWERUPS:
            {
                for (; it->currentIndex < it->size && it->entities[it->currentIndex].state != ACTIVE; ++it->currentIndex);
            } break;
            default: break;
        }
    }
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
    return (iteratorReachedEnd(&it->aliens) && iteratorReachedEnd(&it->bullets));
}

bool checkPairCollision(
    CollisionIterator *it, int *bulletIdx, int *alienIdx
) {
    *bulletIdx = getCurrentIndex(&it->bullets);
    *alienIdx  = getCurrentIndex(&it->aliens);

    return checkCollisionRecs(it->bullets.entities[*bulletIdx].bounds, it->aliens.entities[*alienIdx].bounds);
}

void collisionIteratorNext(CollisionIterator *it) {
    if (!iteratorReachedEnd(&it->bullets)) {
        if (iteratorReachedEnd(&it->aliens)) {
            resetIterator(&it->aliens);
            iteratorNext(&it->bullets);
        } else {
            iteratorNext(&it->aliens);
        }
    }
}


Entity createPlayerShip() {
    const float height = 72.0f;
    const float width = 96.0f;
    const float x = 912.0f;
    const float y = 900.0f;

    Entity ship = {
        .bounds = {
            .height = height,
            .width  = width,
            .x      = x,
            .y      = y,
        },
        .type  = SHIP,
        .state = ACTIVE,
    };

    return ship;
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
    const float height = 32.0f;
    const float width = 4.0f;

    Entity *powerups = (Entity *)malloc(n * sizeof(Entity));

    for (int i = 0; i < n; ++i) {
        // The position and the direction of the bullet will be setted in the activation.
        powerups[i] = (Entity) {
            .bounds = {.height = height, .width = width},
            .type   = BULLET,
            .state  = INACTIVE,
        };
    }

    return powerups;
}

void destroyPowerups(Entity **powerups) {
    free(*powerups);
    *powerups = NULL;
}

void generateBullet(Entity *bullets, Vector2 position, bool up, int n) {
    int i;
    for (i = 0; i < n || bullets[i].state == ACTIVE; ++i);

    bullets[i].bounds.x = position.x;
    bullets[i].bounds.y = position.y;
    bullets[i].up       = up;
    bullets[i].state    = ACTIVE;
}

void generatePowerup(Entity *powerups, Vector2 position, int n) {
    int i;
    for (i = 0; i < n || powerups[i].state == ACTIVE; ++i);

    powerups[i].bounds.x = position.x;
    powerups[i].bounds.y = position.y;
    powerups[i].up       = false;
    powerups[i].state    = ACTIVE;

    if ((rand() % 100) < 50) powerups[i].type = FAST_MOVE;
    else powerups[i].type = FAST_SHOT;
}
