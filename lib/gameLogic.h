# ifndef _GAME_LOGIC_H_
# define _GAME_LOGIC_H_


typedef struct Game Game;

void processInput(Input *input);
void updateGame(Game *game, float deltaTime);
void processMusic(Game *, SnapshotGameState *);
void processSoundFX(Game *, SnapshotGameState *);

# endif