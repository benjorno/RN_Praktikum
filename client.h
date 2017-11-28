#ifndef Client_h
#define Client_h

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

int getpeername(int s, struct sockaddr *addr, socklen_t *len);
#define SUPPORTED_VERSION 1
#define USERNAME_MAX_SIZE 64
#define USERNAME_REAL_SIZE 63
#define MAX_MESSAGE_SIZE 65531
#define PROTOCOL_VERSION 1
enum message_types {
	DISCOVERY_REQUEST = 128,
	DISCOVERY_REPLY = 64,
	SEND_MSG = 80
};

typedef struct {
	uint8_t version;
	uint8_t type;
	uint16_t length;
	struct peerList* firstPeer; //zeiger auf den nächsten peer_info
} discovery_header;

typedef struct send_msg_header{
	uint8_t version;
	uint8_t type;
	uint16_t length;
	char user_message[MAX_MESSAGE_SIZE]; //65536 Bytes - Headersize(4 Byte)
} send_msg_header;

typedef struct peerList {
	uint32_t peerAddress; //4Byte
	uint8_t peerPort; 	//1Byte
	char userName[USERNAME_MAX_SIZE]; //64Byte
	struct peerList* nextPeer;
	struct peerList* prevPeer;
} peerList;

typedef struct {
	uint8_t version;
	uint8_t type;
	uint16_t length;
	char user_message[MAX_MESSAGE_SIZE];
	struct peerList* firstPeer;
} receive_header ;

#endif /* Client_h */

