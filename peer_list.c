#include "peer_list.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>

#include <netdb.h>
#include <pthread.h>


struct ListNode* appendNode(struct ListNode* list, uint32_t address, uint8_t port, char* userName) {
    // Speicher bestellen
    struct ListNode* node = malloc(sizeof(struct ListNode));
    // Zum Ende der Liste gehen
    for (; list && list->nextPeer; list = list->nextPeer);
    // Wert eintragen und Zeiger setzen
    node->peerAddress = address;
    node->peerPort = port;
    strcpy(node->userName, userName);
    node->prevPeer = list ? list : NULL;
    node->nextPeer = NULL;
    if (list)
        list->nextPeer = node;
    return node;
}

ListNode* insertNode(ListNode* list, ListNode* dest, ListNode* obj) {
    if (!list || !obj)
       return NULL;
    if (!dest)
        dest = list;
    obj->nextPeer = dest;
    obj->prevPeer = dest == list ? NULL : dest->prevPeer;
    if (dest == list)
        list = obj;
    dest->prevPeer->nextPeer = obj;
    dest->prevPeer = obj;
    return list;
}

void printList(struct ListNode* list) {
    // Schleife zum durchlaufen
    for (; list; list = list->nextPeer)
        printf("%s ", list->userName);
}

ListNode* removeNode(ListNode* list, ListNode* obj) {
    if (!list || !obj)
       return NULL;
    if (obj->nextPeer)
        obj->nextPeer->prevPeer = obj->prevPeer;
    else
        obj->prevPeer->nextPeer = NULL;
    if (obj->prevPeer)
        obj->prevPeer->nextPeer = obj->nextPeer;
    else {
        obj->nextPeer->prevPeer = NULL;
        list = obj->nextPeer;
    }
    return list;
}
