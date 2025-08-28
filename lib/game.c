#include "game.h"

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>

#include "gameData.h"
#include "gameLogic.h"
#include "peer.h"
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
    Peer *peer,
    double *lastProcTick,
    double *lastCommTick,
    CommandsBufPlayer2 *commandsPlayer2
) {
    double now = getTimeSecs();
    if (now - peer->lastComm > MAX_TIME_WITHOUT_COMM) {
        game->hotData->gameState = CLOSE;
        return;
    }

    if (now - *lastProcTick >= PROC_TICK_DURATION) {
        processInput(&game->hotData->input);
        updateGame(game, commandsPlayer2, PROC_TICK_DURATION);
        BeginDrawing();
            drawGame(game);
        EndDrawing();

        *lastProcTick = now;
    }

    if (now - *lastCommTick >= COMM_TICK_DURATION) {
        if (game->hotData->gameState == PLAYING) {
            int recvResult = recvData(
                peer,
                (char *)commandsPlayer2->input,
                sizeof(Input) * commandsPlayer2->capacity
            );

            if (recvResult == 0) {
                peer->lastComm = now;
            } else if (recvResult == -2) {
                perror("error receiving commands from player 2.\n");
                game->hotData->gameState = CLOSE;
                return;
            }
        }

        buildSnapshot(game, snap);
        int sendResult = sendData(peer, (char *)snap, sizeof(SnapshotGameState));
        if (sendResult == -2) {
            perror("error sending snapshot.\n");
            game->hotData->gameState = CLOSE;
            return;
        }

        // NOTE: I want to update that only if the communication was a success?
        *lastCommTick = now;
    }
}

void remoteLoop(
    Game *game,
    SnapshotGameState *snap,
    Peer *peer,
    double *lastProcTick,
    double *lastCommTick,
    CommandsBufPlayer2 *commandsBuf
) {
    double now = getTimeSecs();
    if (now - peer->lastComm > MAX_TIME_WITHOUT_COMM) {
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
            int sendResult = sendData(
                peer, (char *)commandsBuf->input, sizeof(Input) * commandsBuf->capacity
            );

            if (sendResult == -2) {
                perror("error sending commands.\n");
                game->hotData->gameState = CLOSE;
                return;
            }
        }

        int recvResult = recvData(peer, (char *)snap, sizeof(SnapshotGameState));
        if (recvResult == 0) {
            peer->lastComm = now;
            game->hotData->menuButton = ntohl(snap->menuButton);
            game->hotData->gameState = ntohl(snap->gameState);
        } else if (recvResult == -2) {
            perror("error receiving snapshot.\n");
            game->hotData->gameState = CLOSE;
            return;
        }

        commandsBuf->size = 0;
        *lastCommTick = now;
    }
}

int mainLoop(const char *player) {
    Game game;
    Peer selfPeer;
    SnapshotGameState snap = {0};
    double lastCommTick;
    double lastProcTick;

    int peerInitResult;
    // Initialize network
    if (strcmp(player, "host") == 0) {
        peerInitResult = initPeerUDP(&selfPeer, "127.0.0.1", "127.0.0.1", HOST_PORT, REMOTE_PORT);
    } else if (strcmp(player, "remote") == 0) {
        peerInitResult = initPeerUDP(&selfPeer, "127.0.0.1", "127.0.0.1", REMOTE_PORT, HOST_PORT);
    }

    if (peerInitResult < 0) {
        perror("failed to initialize network.\n");
        return -1;
    }

    SetConfigFlags(FLAG_MSAA_4X_HINT);
    InitWindow(1920.0f, 1080.0f, "Space Invaders Clone");
    InitAudioDevice();
    initGame(&game);
    SetExitKey(KEY_NULL);

    CommandsBufPlayer2 *commandsPlayer2 = initCommandsBuf((int)(COMM_TICK_DURATION/PROC_TICK_DURATION));
    lastCommTick = lastProcTick = selfPeer.lastComm = getTimeSecs();

    // Initialize game loop
    if (strcmp(player, "host") == 0) {
        while (game.hotData->gameState != CLOSE) {
            hostLoop(
                &game,
                &snap,
                &selfPeer,
                &lastProcTick,
                &lastCommTick,
                commandsPlayer2
            );
        }
    } else if (strcmp(player, "remote") == 0) {
        while (game.hotData->gameState != CLOSE) {
            remoteLoop(
                &game,
                &snap,
                &selfPeer,
                &lastProcTick,
                &lastCommTick,
                commandsPlayer2
            );

            // Will apply only with the rollback
            commandsPlayer2->size++;
            commandsPlayer2->size %= commandsPlayer2->capacity;
        }

    }
    
    close(selfPeer.sockFD);
    cleanupCommandsBuf(&commandsPlayer2);
    cleanupGame(&game);
    CloseAudioDevice();
    CloseWindow();
    return 0;
}
