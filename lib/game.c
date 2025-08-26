# include <stdio.h>
# include <string.h>
# include <sys/time.h>
# include <unistd.h>
# include "game.h"
# include "gameData.h"
# include "gameLogic.h"
# include "host.h"
# include "remote.h"
# include "render.h"


# define PROC_TICK_DURATION 0.016f
# define COMM_TICK_DURATION 0.0083f
# define PORT 2112


double getTimeSecs() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (double)tv.tv_sec + (double)tv.tv_usec / 1e6;
}

void hostLoop(
    Game *game,
    SnapshotGameState *snap,
    Host *host,
    double *lastProcTick,
    double *lastCommTick
) {
    double now = getTimeSecs();

    // TODO: Apply rollback
    if (now - *lastProcTick >= PROC_TICK_DURATION) {
        processInput(game->hotData->input);
        updateGame(game, PROC_TICK_DURATION);
        BeginDrawing();
            drawGame(game);
        EndDrawing();

        *lastProcTick += PROC_TICK_DURATION;
    }

    if (now - *lastCommTick >= COMM_TICK_DURATION) {
        buildSnapshot(game, snap);

        int bytesRecvd = 0;
        while (bytesRecvd < sizeof(Input)) {
            int n = recv(host->remote_fd, ((char *)&game->hotData->input[1] + bytesRecvd), sizeof(Input) - bytesRecvd, 0);
            if (n < 0) {
                perror("error receiving remote host input.\n");
                return;
            }
            bytesRecvd += n;
        }

        int bytesSent = 0;
        while (bytesSent < sizeof(SnapshotGameState)) {
            int n = send(host->remote_fd, ((char *)snap) + bytesSent, sizeof(SnapshotGameState) - bytesSent, 0);
            if (n < 0) {
                perror("error sending game snapshot.\n");
                return;
            }

            bytesSent += n;
        }

        *lastCommTick += COMM_TICK_DURATION;
    }
}

void remoteLoop(
    Game *game,
    SnapshotGameState *snap,
    Remote *remote,
    double *lastProcTick,
    double *lastCommTick,
    Player2CommandsBuf *commandsBuf
) {
    double now = getTimeSecs();

    if (now - *lastProcTick >= PROC_TICK_DURATION) {
        int currentCommand = commandsBuf->size;
        processInput(&commandsBuf->input[currentCommand]);
        BeginDrawing();
            drawSnapshot(game, snap);
        EndDrawing();

        // Will apply only with the rollback
        // commandsBuf->size++;
        *lastProcTick += PROC_TICK_DURATION;
    }

    if (now - *lastCommTick >= COMM_TICK_DURATION) {
        buildSnapshot(game, snap);

        int bytesSent = 0;
        while (bytesSent < sizeof(Input)) {
            int n = send(remote->remote_fd, ((char *)&commandsBuf->input[0]) + bytesSent, sizeof(Input) - bytesSent, 0);
            if (n < 0) {
                perror("error sending remote input.\n");
                game->hotData->gameState = CLOSE;
                return;
            }
            bytesSent += n;
        }

        int bytesRecvd = 0;
        while (bytesRecvd < sizeof(SnapshotGameState)) {
            int n = recv(remote->remote_fd, ((char *)snap) + bytesRecvd, sizeof(SnapshotGameState) - bytesRecvd, 0);
            if (n < 0) {
                perror("error receiving snapshot.\n");
                return;
            } else if (n == 0) {
                // TODO: manage disconnection
                game->hotData->gameState = CLOSE;
                return;
            }
            bytesRecvd += n;
        }

        commandsBuf->size = 0;
        game->hotData->menuButton = snap->menuButton;
        game->hotData->gameState = snap->gameState;
        *lastCommTick += COMM_TICK_DURATION;
    }
}

int mainLoop(const char *player) {
    Game game;
    Host host;
    Remote remote;
    SnapshotGameState snap;
    double lastCommTick;
    double lastProcTick;

    // Initialize network
    if (strcmp(player, "host") == 0) {
        initHostTCP(&host, PORT);
        acceptConnectionTCP(&host);
    } else if (strcmp(player, "remote") == 0) {
        initRemoteTCP(&remote, "127.0.0.1", PORT);
        connectTCP(&remote);
    }

    SetConfigFlags(FLAG_MSAA_4X_HINT);
    InitWindow(1920.0f, 1080.0f, "Space Invaders Clone");
    InitAudioDevice();
    initGame(&game);
    SetExitKey(KEY_NULL);

    // Initialize game loop
    if (strcmp(player, "host") == 0) {
        lastCommTick = lastProcTick = getTimeSecs();
        while (game.hotData->gameState != CLOSE) {
            hostLoop(
                &game,
                &snap,
                &host,
                &lastProcTick,
                &lastCommTick
            );
        }

        close(host.host_fd);
        close(host.remote_fd);
    } else if (strcmp(player, "remote") == 0) {
        Player2CommandsBuf *commands = initCommandsBuf((int)(COMM_TICK_DURATION/PROC_TICK_DURATION));
        lastCommTick = lastProcTick = getTimeSecs();
        while (game.hotData->gameState != CLOSE) {
            remoteLoop(
                &game,
                &snap,
                &remote,
                &lastProcTick,
                &lastCommTick,
                commands
            );
        }

        close(remote.remote_fd);
        cleanupCommandsBuf(&commands);
    }

    cleanupGame(&game);
    CloseAudioDevice();
    CloseWindow();
    return 0;
}
