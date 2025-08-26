# include <netinet/tcp.h>
# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <stdbool.h>
# include <unistd.h>
# include "gameData.h"
# include "host.h"


int initHostTCP(Host *host, uint16_t port) {
    *host = (Host) {
        .host_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP),
        .host_addr = {
            .sin_family      = AF_INET,
            .sin_port        = htons(port),
            .sin_addr.s_addr = INADDR_ANY
        },
    };
    memset(&host->host_addr.sin_zero, 0, sizeof(host->host_addr.sin_zero));

    if (host->host_fd < 0) {
        perror("failed to create the host socket.\n");
        return -1;
    }
    int yes = 1;
    setsockopt(host->host_fd, IPPROTO_TCP, TCP_NODELAY, (const void *)&yes, sizeof(yes));

    if (bind(host->host_fd, (struct sockaddr *)&host->host_addr, sizeof(host->host_addr)) < 0) {
        perror("failed to bind the host socket.\n");
        close(host->host_fd);
        return -3;
    }

    if (listen(host->host_fd, 5) < 0) {
        perror("failed to setup the listening for a handshake.\n");
        close(host->host_fd);
        return -4;
    }

    printf("Host initialized.\n");

    return 0;
}

int acceptConnectionTCP(Host *host) {
    host->remote_fd = accept(host->host_fd, (struct sockaddr *)&host->remote_addr, &host->remote_addr_len);
    if (host->remote_fd < 0) {
        perror("handshake failed.\n");
        close(host->host_fd);
        return -1;
    }

    // int flags = fcntl(host->host_fd, F_GETFL, 0);
    // int result = fcntl(host->host_fd, F_SETFL, flags | O_NONBLOCK);
    // if (result < 0) {
    //     perror("failed to set socket operations non-blocking.\n");
    //     close(host->host_fd);
    //     return -2;
    // }

    printf("connected...\n");
    return 0;
}
