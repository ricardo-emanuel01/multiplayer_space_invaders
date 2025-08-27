#ifndef _GAME_LOGIC_H_
#define _GAME_LOGIC_H_

#include "gameData.h" 

typedef struct Game Game;
typedef struct SnapshotGameState SnapshotGameState;

void processInput(Input *input);
void updateGame(Game *game, CommandsBufPlayer2 *commandsPlayer2, float deltaTime);
void processMusic(Game *, SnapshotGameState *);
void processSoundFX(Game *, SnapshotGameState *);

#endif