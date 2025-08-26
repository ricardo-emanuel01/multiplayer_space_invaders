# include <netinet/tcp.h>
# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <unistd.h>
# include "gameData.h"
# include "remote.h"


int initRemoteTCP(Remote *remote, const char *addr, uint16_t port) {
    *remote = (Remote) {
        .remote_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP),
        .host_addr = {
            .sin_family = AF_INET,
            .sin_port   = htons(port) 
        }
    };
    inet_pton(AF_INET, addr, &remote->host_addr.sin_addr);
    memset(&remote->host_addr.sin_zero, 0, sizeof(remote->host_addr.sin_zero));
    int yes = 1;
    setsockopt(remote->remote_fd, IPPROTO_TCP, TCP_NODELAY, (const void *)&yes, sizeof(yes));

    if (remote->remote_fd < 0) {
        perror("failed to create remote socket.\n");
        return -1;
    }

    printf("remote initialized.\n");

    return 0;
}

int connectTCP(Remote *remote) {
    if (connect(remote->remote_fd, (struct sockaddr *)&remote->host_addr, sizeof(remote->host_addr)) < 0) {
        perror("failed to handshake.\n");
        close(remote->remote_fd);
        return -1;
    }
    // int flags = fcntl(remote->remote_fd, F_GETFL, 0);
    // int result = fcntl(remote->remote_fd, F_SETFL, flags | O_NONBLOCK);
    // if (result < 0) {
    //     perror("failed to set socket operations non-blocking.\n");
    //     close(remote->remote_fd);
    //     return -2;
    // }
    printf("connected...\n");
}
