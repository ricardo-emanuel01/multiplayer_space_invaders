#ifndef _PEER_H_
#define _PEER_H_

#include <stddef.h>
#include <stdint.h>


typedef struct Peer Peer;

// Initialize a peer which will be binded at selfAddr and send data to remoteAddr
int initPeerUDP(Peer *peer, const char *selfAddr, const char *remoteAddr, uint16_t selfPort, uint16_t remotePort);
int sendData(Peer *peer, char *src, size_t size);
int recvData(Peer *peer, char *dst, size_t size);


#endif