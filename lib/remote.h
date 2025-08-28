#ifndef _REMOTE_H_
#define _REMOTE_H_

#include <arpa/inet.h>


typedef struct Remote Remote;

int initRemoteTCP(Remote *remote, uint16_t hostPort, uint16_t remotePort);

#endif
