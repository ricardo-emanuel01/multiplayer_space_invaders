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
#define COMM_TICK_DURATION 0.05f
#define MAX_TIME_WITHOUT_COMM 10.0f


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
    if (now - host->last_comm > MAX_TIME_WITHOUT_COMM) {
        game->hotData->gameState = CLOSE;
        return;
    }

    // TODO: Apply rollback
    if (now - *lastProcTick >= PROC_TICK_DURATION) {
        processInput(&game->hotData->input);
        updateGame(game, commandsPlayer2, PROC_TICK_DURATION);
        BeginDrawing();
            drawGame(game);
        EndDrawing();

        *lastProcTick = now;
    }

    if (now - *lastCommTick >= COMM_TICK_DURATION) {
        buildSnapshot(game, snap);

        if (game->hotData->gameState == PLAYING) {
            int n = recvfrom(
                host->host_fd,
                ((char *)commandsPlayer2->input ),
                commandsPlayer2->capacity * sizeof(Input),
                0,
                (struct sockaddr *)&host->remote_addr,
                &host->remote_len
            );

            if (n < 0) {
                if (errno != EAGAIN && errno != EWOULDBLOCK) {
                    perror("error receiving commands.\n");
                    game->hotData->gameState = CLOSE;
                    return;
                }
            } else if (n > 0) host->last_comm = now;
        }

        int n = sendto(
            host->host_fd,
            (char *)snap,
            sizeof(SnapshotGameState),
            0,
            (struct sockaddr *)&host->remote_addr,
            host->remote_len
        );
        
        if (n < 0) {
            if (errno != EAGAIN && errno != EWOULDBLOCK) {
                perror("error sending snapshot.\n");
                game->hotData->gameState = CLOSE;
                return;
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
    if (now - remote->last_comm > MAX_TIME_WITHOUT_COMM) {
        game->hotData->gameState = CLOSE;
        return;
    }

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
        if (game->hotData->gameState == PLAYING) {
            int n = sendto(
                remote->remote_fd,
                ((char *)commandsBuf->input),
                sizeof(Input) * commandsBuf->capacity,
                0,
                (struct sockaddr *)&remote->host_addr,
                remote->host_len
            );

            if (n < 0) {
                if (errno != EAGAIN && errno != EWOULDBLOCK) {
                    perror("error sending commands.\n");
                    game->hotData->gameState = CLOSE;
                    return;
                }
            }
        }

        int n = recvfrom(
            remote->remote_fd,
            ((char *)snap),
            sizeof(SnapshotGameState),
            0,
            (struct sockaddr *)&remote->host_addr,
            &remote->host_len
        );

        if (n < 0) {
            if (errno != EAGAIN && errno != EWOULDBLOCK) {
                perror("error receiving snapshot.\n");
                game->hotData->gameState = CLOSE;
                return;
            }
        } else if (n > 0) remote->last_comm = now;

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
        initHostTCP(&host, HOST_PORT, REMOTE_PORT);
    } else if (strcmp(player, "remote") == 0) {
        initRemoteTCP(&remote, HOST_PORT, REMOTE_PORT);
    }

    SetConfigFlags(FLAG_MSAA_4X_HINT);
    InitWindow(1920.0f, 1080.0f, "Space Invaders Clone");
    InitAudioDevice();
    initGame(&game);
    SetExitKey(KEY_NULL);

    // Initialize game loop
    if (strcmp(player, "host") == 0) {
        CommandsBufPlayer2 *commandsPlayer2 = initCommandsBuf((int)(COMM_TICK_DURATION/PROC_TICK_DURATION));
        lastCommTick = lastProcTick = host.last_comm = getTimeSecs();
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
    } else if (strcmp(player, "remote") == 0) {
        CommandsBufPlayer2 *commands = initCommandsBuf((int)(COMM_TICK_DURATION/PROC_TICK_DURATION));
        lastCommTick = lastProcTick = remote.last_comm = getTimeSecs();
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
