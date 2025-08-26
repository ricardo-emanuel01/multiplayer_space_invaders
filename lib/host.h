# ifndef _HOST_H_
# define _HOST_H_

# include <arpa/inet.h>


typedef struct Host Host;


int initHostTCP(Host *host, uint16_t port);
int acceptConnectionTCP(Host *host);
void cleanupHostTCP(Host **host);

# endif
