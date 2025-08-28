#include "peer.h"

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "gameData.h"


int initPeerUDP(
    Peer *peer,
    const char *selfAddr,
    const char *remoteAddr,
    uint16_t selfPort,
    uint16_t remotePort
) {
    *peer = (Peer) {
        .sockFD = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP),
        .selfAddr = {
            .sin_family      = AF_INET,
            .sin_port        = htons(selfPort),
            .sin_addr.s_addr = inet_addr(selfAddr)
        },
        .remoteAddr = {
            .sin_family      = AF_INET,
            .sin_port        = htons(remotePort),
            .sin_addr.s_addr = inet_addr(remoteAddr)
        }
    };

    if (peer->sockFD < 0) {
        perror("failed to create the socket.\n");
        return -1;
    }

    peer->remoteLen = sizeof(peer->remoteAddr);
    memset(&peer->selfAddr.sin_zero, 0, sizeof(peer->selfAddr.sin_zero));
    memset(&peer->remoteAddr.sin_zero, 0, sizeof(peer->remoteAddr.sin_zero));

    int flags = fcntl(peer->sockFD, F_GETFL, 0);
    int result = fcntl(peer->sockFD, F_SETFL, flags | O_NONBLOCK);
    if (result < 0) {
        perror("failed to set socket operations non-blocking.\n");
        close(peer->sockFD);
        return -2;
    }

    if (bind(peer->sockFD, (struct sockaddr *)&peer->selfAddr, sizeof(peer->selfAddr)) < 0) {
        perror("failed to bind the socket.\n");
        close(peer->sockFD);
        return -3;
    }

    return 0;
}

int sendData(Peer *peer, char *src, size_t size) {
    int n = sendto(
        peer->sockFD,
        src,
        size,
        0,
        (struct sockaddr *)&peer->remoteAddr,
        peer->remoteLen
    );

    if (n < 0) {
        if (errno != EAGAIN && errno != EWOULDBLOCK) {
            perror("error sending data.\n");
            return -2;
        }

        return -1;
    }

    return 0;
}

int recvData(Peer *peer, char *dst, size_t size) {
    int n = recvfrom(
        peer->sockFD,
        dst,
        size,
        0,
        (struct sockaddr *)&peer->remoteAddr,
        &peer->remoteLen
    );

    if (n < 0) {
        if (errno != EAGAIN && errno != EWOULDBLOCK) {
            perror("error receiving data.\n");
            return -2;
        }

        return -1;
    }

    return 0;
}
