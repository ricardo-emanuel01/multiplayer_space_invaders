#include "render.h"

#include <arpa/inet.h>
#include <raylib.h>
#include <stdint.h>
#include <string.h>

#include "entity.h"
#include "gameData.h"


void drawEntity(Game *game, Entity *entity) {
    if (entity->state != ACTIVE) return;

    Vector2 origin = {0.0f, 0.0f};
    float rotation = 0.0f;
    Texture2D tex;
    Rectangle sourceRect;

    switch (entity->type) {
        case SHIP:
        {
            tex = game->textures->ship;
            sourceRect = game->animation->shipFrame;
        } break;
        case ENEMY_SHIP:
        {
            tex = game->textures->enemyShip;
            sourceRect = game->animation->enemyShipFrame;
        } break;
        case ALIEN1:
        {
            tex = game->textures->alien1;
            sourceRect = game->animation->aliensFrame;
        } break;
        case ALIEN2:
        {
            tex = game->textures->alien2;
            sourceRect = game->animation->aliensFrame;
        } break;
        case ALIEN3:
        {
            tex = game->textures->alien3;
            sourceRect = game->animation->aliensFrame;
        } break;
        case BULLET:
        {
            tex = game->textures->bullet;
            sourceRect = game->animation->bulletFrame;
        } break;
        case FAST_SHOT:
        {
            tex = game->textures->shotPowerup;
            sourceRect = game->animation->powerupFrame;
        } break;
        case FAST_MOVE:
        {
            tex = game->textures->movePowerup;
            sourceRect = game->animation->powerupFrame;
        } break;
        default: break;
    }

    // NOTE: What is the tint parameter?
    DrawTexturePro(
        tex, sourceRect, entity->bounds, origin, rotation, WHITE
    );
}

void drawEntities(Game *game, EntitiesIterator *it) {
    while (!iteratorReachedEnd(it)) {
        int index = getCurrentIndex(it);
        if (index != -1) {
            drawEntity(game, &it->entities[index]);
        }

        iteratorNext(it);
    }
}

void drawMenuBackground(Rectangle *rec) {
    Vector2 origin = {0.0f, 0.0f};

    DrawRectanglePro(*rec, origin, 0.0f, WHITE);
}

void drawTextCentered(
    const char *text,
    float fontSize,
    VerticalAlignment vAlignment,
    Rectangle anchor,
    Font font
) {
    float spacing = 5.0f;
    Vector2 textSize = MeasureTextEx(font, text, fontSize, spacing);
    Vector2 textPosition = {
        .x = anchor.x + (anchor.width - textSize.x) / 2.0f,
        .y = anchor.y
    };

    switch (vAlignment) {
        case TOP:
        {
            textPosition.y += 10.0f;
        } break;
        case MIDDLE:
        {
            textPosition.y += (anchor.height - textSize.y) / 2;
        } break;
        case BOTTOM:
        {
            textPosition.y += anchor.height - textSize.y - 10.0f;
        } break;
        default: break;
    }

    DrawTextEx(font, text, textPosition, fontSize, spacing, BLACK);
}

void drawEndStatus(Game *game) {
    char message[9];
    if (game->hotData->gameState == WIN) {
        strcpy(message, "VICTORY");
    } else {
        strcpy(message, "DEFEATED");
    }

    float posX = (game->screenWidth - MeasureText(message, 200))/2.0f;
    DrawText(message, posX, 150.0f, 200, RAYWHITE);
}

void drawConnectingScreen() {

}

void drawMenu(Game *game) {
    const float height = 400.0f;
    const float width = 600.0f;
    const float x = (game->screenWidth - width)/2.0f;
    const float y = (game->screenHeight - height)/2.0f;
    Rectangle background = {
        .height = height,
        .width = width,
        .x = x,
        .y = y
    };

    drawMenuBackground(&background);

    char textTop[8];
    char textBottom[] = "QUIT";
    float fontSizeTop, fontSizeBottom;
    switch (game->hotData->gameState) {
        case WIN:
        case LOSE:
        {
            TextCopy(textTop, "RESTART");
        } break;
        case MENU:
        {
            TextCopy(textTop, "START");
        } break;
        case PAUSED:
        {
            TextCopy(textTop, "RESUME");
        }
        default: break;
    }

    switch (game->hotData->menuButton) {
        case START:
        {
            fontSizeTop    = 100.0f;
            fontSizeBottom = 80.0f;
        } break;
        case QUIT:
        {
            fontSizeTop    = 80.0f;
            fontSizeBottom = 100.0f;
        } break;
        default: break;
    }

    drawTextCentered(textTop, fontSizeTop, TOP, background, GetFontDefault());
    drawTextCentered(textBottom, fontSizeBottom, BOTTOM, background, GetFontDefault());
}

void drawGame(Game *game) {
    ClearBackground(BLACK);
    DrawFPS(10, 10);

    drawEntity(game, &game->ships[0]);
    drawEntity(game, &game->ships[1]);
    drawEntity(game, &game->enemyShip);
    
    EntitiesIterator hordeIt = createIterator(
        game->horde, ALIENS, nRowsAliens*nColsAliens
    );
    drawEntities(game, &hordeIt);

    EntitiesIterator bulletsIt = createIterator(
        game->bullets, BULLETS, game->nBullets
    );
    drawEntities(game, &bulletsIt);

    EntitiesIterator powerupsIt = createIterator(
        game->powerups, POWERUPS, game->nPowerups
    );
    drawEntities(game, &powerupsIt);

    if (
        game->hotData->gameState != PLAYING    &&
        game->hotData->gameState != CONNECTING &&
        game->hotData->gameState != CLOSE
    ) {
        drawMenu(game);
    }

    if (game->hotData->gameState == WIN || game->hotData->gameState == LOSE) {
        drawEndStatus(game);
    }

    if (game->hotData->gameState == CONNECTING) {
        drawConnectingScreen();
    }
}

void drawEntityNetwork(Game *game, EntityBounds bounds, EntityType type) {
    Vector2 origin = {0.0f, 0.0f};
    Texture2D currentTex;
    Rectangle srcRectangle, dstRectangle;
    switch (type) {
        case SHIP:
        {
            currentTex = game->textures->ship;
            srcRectangle = game->animation->shipFrame;
            dstRectangle = (Rectangle) {
                .height = game->ships[0].bounds.height,
                .width  = game->ships[0].bounds.width,
                .x      = (float)ntohs(bounds.x),
                .y      = (float)ntohs(bounds.y)
            };
        } break;
        case ENEMY_SHIP:
        {
            currentTex = game->textures->enemyShip;
            srcRectangle = game->animation->enemyShipFrame;
            dstRectangle = (Rectangle) {
                .height = game->enemyShip.bounds.height,
                .width  = game->enemyShip.bounds.width,
                .x      = (float)ntohs(bounds.x),
                .y      = (float)ntohs(bounds.y)
            };
        } break;
        case ALIEN1:
        {
            currentTex = game->textures->alien1;
            srcRectangle = game->animation->aliensFrame;
            dstRectangle = (Rectangle) {
                .height = game->horde[0].bounds.height,
                .width  = game->horde[0].bounds.width,
                .x      = (float)ntohs(bounds.x),
                .y      = (float)ntohs(bounds.y)
            };
        } break;
        case ALIEN2:
        {
            currentTex = game->textures->alien2;
            srcRectangle = game->animation->aliensFrame;
            dstRectangle = (Rectangle) {
                .height = game->horde[0].bounds.height,
                .width  = game->horde[0].bounds.width,
                .x      = (float)ntohs(bounds.x),
                .y      = (float)ntohs(bounds.y)
            };
        } break;
        case ALIEN3:
        {
            currentTex = game->textures->alien3;
            srcRectangle = game->animation->aliensFrame;
            dstRectangle = (Rectangle) {
                .height = game->horde[0].bounds.height,
                .width  = game->horde[0].bounds.width,
                .x      = (float)ntohs(bounds.x),
                .y      = (float)ntohs(bounds.y)
            };
        } break;
        case BULLET:
        {
            currentTex = game->textures->bullet;
            srcRectangle = game->animation->bulletFrame;
            dstRectangle = (Rectangle) {
                .height = game->bullets[0].bounds.height,
                .width  = game->bullets[0].bounds.width,
                .x      = (float)ntohs(bounds.x),
                .y      = (float)ntohs(bounds.y)
            };
        } break;
        case FAST_MOVE:
        {
            currentTex = game->textures->movePowerup;
            srcRectangle = game->animation->powerupFrame;
            dstRectangle = (Rectangle) {
                .height = game->powerups[0].bounds.height,
                .width  = game->powerups[0].bounds.width,
                .x      = (float)ntohs(bounds.x),
                .y      = (float)ntohs(bounds.y)
            };
        } break;
        case FAST_SHOT:
        {
            currentTex = game->textures->shotPowerup;
            srcRectangle = game->animation->powerupFrame;
            dstRectangle = (Rectangle) {
                .height = game->powerups[0].bounds.height,
                .width  = game->powerups[0].bounds.width,
                .x      = (float)ntohs(bounds.x),
                .y      = (float)ntohs(bounds.y)
            };
        } break;
        default: break;
    }

    DrawTexturePro(
        currentTex, srcRectangle, dstRectangle, origin, 0.0f, WHITE
    );
}

EntityType getEntityType(int index) {
    if (index < 22) return ALIEN1;
    if (index < 33) return ALIEN2;
    if (index < 55) return ALIEN3;
    if (index < 56) return ENEMY_SHIP;
    if (index < 58) return SHIP;
    if (index < 68) return FAST_MOVE;
    if (index < 78) return FAST_SHOT;
    if (index < 118) return BULLET;
}

void drawSnapshot(Game *game, SnapshotGameState *snap) {
    EntityType currentType;
    ClearBackground(BLACK);
    for (int i = 0; i < N_ENTITIES; ++i) {
        if (snap->entities[i].x != 0 || snap->entities[i].y != 0) {
            drawEntityNetwork(game, snap->entities[i], getEntityType(i));
        }
    }

    if (game->hotData->gameState != PLAYING) {
        drawMenu(game);
    }

    if (game->hotData->gameState == WIN || game->hotData->gameState == LOSE) {
        drawEndStatus(game);
    }
}
