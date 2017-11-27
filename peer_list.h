/*
 * peer_list.h
 *
 *  Created on: 26.11.2017
 *      Author: bennet
 */

#ifndef PEER_LIST_H_
#define PEER_LIST_H_

#define USERNAME_MAX_SIZE 64

typedef struct ListNode {
	uint32_t peerAddress; //4Byte
	uint8_t peerPort; 	//1Byte
	char userName[USERNAME_MAX_SIZE]; //64Byte
	struct ListNode* nextPeer;
	struct ListNode* prevPeer;
} ListNode;

#endif /* PEER_LIST_H_ */
