#include "remote.h"

#include <fcntl.h>
#include <netinet/tcp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "gameData.h"


int initRemoteTCP(Remote *remote, uint16_t hostPort, uint16_t remotePort) {
    *remote = (Remote) {
        .remote_fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP),
        .host_addr = {
            .sin_family      = AF_INET,
            .sin_port        = htons(hostPort),
            .sin_addr.s_addr = inet_addr("127.0.0.1")
        },
        .remote_addr = {
            .sin_family      = AF_INET,
            .sin_port        = htons(remotePort),
            .sin_addr.s_addr = INADDR_ANY
        }
    };
    remote->host_len = sizeof(remote->host_addr);
    memset(&remote->host_addr.sin_zero, 0, sizeof(remote->host_addr.sin_zero));
    memset(&remote->remote_addr.sin_zero, 0, sizeof(remote->remote_addr.sin_zero));

    if (remote->remote_fd < 0) {
        perror("failed to create remote socket.\n");
        return -1;
    }

    if (bind(remote->remote_fd, (struct sockaddr *)&remote->remote_addr, sizeof(remote->remote_addr)) < 0) {
        perror("failed to bind the host socket.\n");
        close(remote->remote_fd);
        return -3;
    }

    int flags = fcntl(remote->remote_fd, F_GETFL, 0);
    int result = fcntl(remote->remote_fd, F_SETFL, flags | O_NONBLOCK);
    if (result < 0) {
        perror("failed to set socket operations non-blocking.\n");
        close(remote->remote_fd);
        return -2;
    }

    printf("remote initialized.\n");

    return 0;
}
