#include "game.h"

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>

#include "gameData.h"
#include "gameLogic.h"
#include "host.h"
#include "remote.h"
#include "render.h"


#define PROC_TICK_DURATION 0.016f
#define COMM_TICK_DURATION 0.016f
#define PORT 2112


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
    double *lastCommTick,
    CommandsBufPlayer2 *commandsPlayer2
) {
    double now = getTimeSecs();

    // TODO: Apply rollback
    if (now - *lastProcTick >= PROC_TICK_DURATION) {
        processInput(&game->hotData->input);
        updateGame(game, commandsPlayer2, now - *lastProcTick);
        BeginDrawing();
            drawGame(game);
        EndDrawing();

        *lastProcTick = now;
    }

    if (now - *lastCommTick >= COMM_TICK_DURATION) {
        buildSnapshot(game, snap);

        int bytesRecvd = 0;
        while (bytesRecvd < commandsPlayer2->capacity * sizeof(Input)) {
            int n = recv(
                host->remote_fd,
                ((char *)commandsPlayer2->input + bytesRecvd),
                commandsPlayer2->capacity * sizeof(Input) - bytesRecvd,
                0
            );
            if (n > 0) {
                bytesRecvd += n;
            } else if (n == 0) {
                // Disconnection
                game->hotData->gameState = CLOSE;
                break;
            } else if (n < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
                // NO DATA AVAILABLE
                break;
            } else {
                perror("failed to recv data.\n");
                break;
            }
        }

        int bytesSent = 0;
        while (bytesSent < sizeof(SnapshotGameState)) {
            int n = send(host->remote_fd, ((char *)snap) + bytesSent, sizeof(SnapshotGameState) - bytesSent, 0);
            if (n > 0) {
                bytesSent += n;
            } else if (n < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
                break;
            } else {
                perror("error sending game snapshot.\n");
                break;
            }
        }

        *lastCommTick = now;
    }
}

void remoteLoop(
    Game *game,
    SnapshotGameState *snap,
    Remote *remote,
    double *lastProcTick,
    double *lastCommTick,
    CommandsBufPlayer2 *commandsBuf
) {
    double now = getTimeSecs();

    if (now - *lastProcTick >= PROC_TICK_DURATION) {
        int currentCommand = commandsBuf->size;
        processInput(&commandsBuf->input[currentCommand]);
        processMusic(game, snap);
        processSoundFX(game, snap);
        BeginDrawing();
            drawSnapshot(game, snap);
        EndDrawing();

        *lastProcTick = now;
    }

    if (now - *lastCommTick >= COMM_TICK_DURATION) {
        int bytesSent = 0;
        while (bytesSent < sizeof(Input) * commandsBuf->capacity) {
            int n = send(
                remote->remote_fd,
                ((char *)commandsBuf->input) + bytesSent,
                sizeof(Input) * commandsBuf->capacity - bytesSent,
                0
            );
            if (n > 0) {
                bytesSent += n;
            } else if (n < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
                break;
            } else {
                perror("failed to send.\n");
                break;
            }
        }

        int bytesRecvd = 0;
        while (bytesRecvd < sizeof(SnapshotGameState)) {
            int n = recv(remote->remote_fd, ((char *)snap) + bytesRecvd, sizeof(SnapshotGameState) - bytesRecvd, 0);
            if (n > 0) {
                bytesRecvd += n;
            } else if (n == 0) {
                // Disconnection
                game->hotData->gameState = CLOSE;
                break;
            } else if (n < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
                // NO DATA AVAILABLE
                break;
            } else {
                perror("failed to recv data.\n");
                break;
            }
        }

        commandsBuf->size = 0;
        game->hotData->menuButton = ntohl(snap->menuButton);
        game->hotData->gameState = ntohl(snap->gameState);
        *lastCommTick = now;
    }
}

int mainLoop(const char *player) {
    Game game;
    Host host;
    Remote remote;
    SnapshotGameState snap = {0};
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
        CommandsBufPlayer2 *commandsPlayer2 = initCommandsBuf((int)(COMM_TICK_DURATION/PROC_TICK_DURATION));
        lastCommTick = lastProcTick = getTimeSecs();
        while (game.hotData->gameState != CLOSE) {
            hostLoop(
                &game,
                &snap,
                &host,
                &lastProcTick,
                &lastCommTick,
                commandsPlayer2
            );
        }

        cleanupCommandsBuf(&commandsPlayer2);
        close(host.host_fd);
        close(host.remote_fd);
    } else if (strcmp(player, "remote") == 0) {
        CommandsBufPlayer2 *commands = initCommandsBuf((int)(COMM_TICK_DURATION/PROC_TICK_DURATION));
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

            // Will apply only with the rollback
            commands->size++;
            commands->size %= commands->capacity;
        }

        close(remote.remote_fd);
        cleanupCommandsBuf(&commands);
    }

    cleanupGame(&game);
    CloseAudioDevice();
    CloseWindow();
    return 0;
}
