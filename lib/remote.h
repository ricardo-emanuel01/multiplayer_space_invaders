#ifndef _REMOTE_H_
#define _REMOTE_H_

#include <arpa/inet.h>


typedef struct Remote Remote;

int initRemoteTCP(Remote *remote, const char *addr, uint16_t port);
int connectTCP(Remote *remote);
void cleanupRemoteTCP(Remote **remote);

#endif
