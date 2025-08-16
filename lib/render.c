# include <stdint.h>
# include <raylib.h>

# include "entity.h"
# include "gameData.h"
# include "render.h"


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
    Vector2 textSize = MeasureTextEx(font, text, fontSize, 0.0f);
    Vector2 textPosition = {
        .x = anchor.x + (anchor.width - textSize.x) / 2,
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

    DrawTextEx(font, text, textPosition, fontSize, 5.0f, BLACK);
}

void drawEndStatus(GameState gameState) {
    switch (gameState) {
        case WIN:
        {

        } break;
        case LOSE:
        {

        } break;
        default: break;
    }
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
    drawEntity(game, &game->ship);
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
        drawEndStatus(game->hotData->gameState);
    }

    if (game->hotData->gameState == CONNECTING) {
        drawConnectingScreen();
    }
}
