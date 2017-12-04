#ifndef PEER_H_
#define PEER_H_

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>

#include <netdb.h>
#include <pthread.h>

#define PROTOCOL_VERSION 1
#define MAX_SIZE_USERNAME 64
#define MAX_MESSAGE_SIZE 65532
#define MAX_SIZE_PEERS 10

int getpeername(int s, struct sockaddr *addr, socklen_t *len);

enum message_types {
	DISCOVERY_REQUEST = 1,
	DISCOVERY_REPLY = 2,
	SEND_MSG = 10
};

typedef struct {
	uint8_t version;
	uint8_t type;
	uint16_t length;
} commonHeader;

typedef struct {
	commonHeader header;
	char userMessage[MAX_MESSAGE_SIZE];
} messageHeader;

typedef struct peerInfo {
	uint32_t address[16];
	uint8_t port;
	char username[MAX_SIZE_USERNAME];
} peerInfo;

typedef struct {
	commonHeader header;
	peerInfo peers[MAX_SIZE_PEERS];
} discoveryHeader;

#endif /* PEER_H_ */

