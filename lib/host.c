#include "host.h"

#include <fcntl.h>
#include <netinet/tcp.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "gameData.h"


int initHostTCP(Host *host, uint16_t hostPort, uint16_t remotePort) {
    *host = (Host) {
        .host_fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP),
        .host_addr = {
            .sin_family      = AF_INET,
            .sin_port        = htons(hostPort),
            .sin_addr.s_addr = INADDR_ANY
        },
        .remote_addr = {
            .sin_family      = AF_INET,
            .sin_port        = htons(remotePort),
            .sin_addr.s_addr = inet_addr("127.0.0.1")
        }
    };
    host->remote_len = sizeof(host->remote_addr);
    memset(&host->host_addr.sin_zero, 0, sizeof(host->host_addr.sin_zero));
    memset(&host->remote_addr.sin_zero, 0, sizeof(host->remote_addr.sin_zero));

    if (host->host_fd < 0) {
        perror("failed to create the host socket.\n");
        return -1;
    }

    if (bind(host->host_fd, (struct sockaddr *)&host->host_addr, sizeof(host->host_addr)) < 0) {
        perror("failed to bind the host socket.\n");
        close(host->host_fd);
        return -3;
    }

    int flags = fcntl(host->host_fd, F_GETFL, 0);
    int result = fcntl(host->host_fd, F_SETFL, flags | O_NONBLOCK);
    if (result < 0) {
        perror("failed to set socket operations non-blocking.\n");
        close(host->host_fd);
        return -2;
    }

    printf("Host initialized.\n");

    return 0;
}
