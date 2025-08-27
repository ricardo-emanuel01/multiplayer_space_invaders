#ifndef _RENDER_H_
#define _RENDER_H_

#include <raylib.h>


typedef struct SnapshotGameState SnapshotGameState;

typedef enum VerticalAlignment {
    TOP,
    MIDDLE,
    BOTTOM,
} VerticalAlignment;

typedef struct Game Game;

void drawGame(Game *game);
void drawSnapshot(Game *game, SnapshotGameState *);

#endif