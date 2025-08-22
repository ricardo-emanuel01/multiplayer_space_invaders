# include "game.h"
# include "gameLogic.h"
# include "render.h"


void mainLoop() {
    Game game;
    SetConfigFlags(FLAG_MSAA_4X_HINT);
    InitWindow(game.screenWidth, game.screenHeight, "Space Invaders Clone");
    InitAudioDevice();
    SetExitKey(KEY_NULL);
    SetTargetFPS(60);
    DisableCursor();

    initGame(&game);

    while (game.hotData->gameState != CLOSE) {
        float delta = GetFrameTime();
        processInput(&game.hotData->input);
        updateGame(&game, delta);
        BeginDrawing();
            drawGame(&game);
        EndDrawing();
    }

    cleanupGame(&game);
    CloseAudioDevice();
    CloseWindow();
}
