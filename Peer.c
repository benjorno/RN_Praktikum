#include "Peer.h"
#define LOCAL_PORT 15000
#define LOCAL_PORT2 "15000"
#define DISCOVERY_REQUEST_SLEEP 4

char localIp[16];
char username[MAX_SIZE_USERNAME];
char command[255];
char ip[16];
int port;

struct peerInfo peers[MAX_SIZE_PEERS];

pthread_t receiverThread, discoveryThread;

/**
 * Öffnet einen Socket und verbindet sich
 */
int openSocketAndConnect(char* destinationIp, int destinationPort) {
	int socketRequest;
	struct sockaddr_in address;
	int res;
	socketRequest = socket(AF_INET, SOCK_STREAM, 0);
	if (socketRequest > 0) {
		//printf("Socket wurde angelegt\n");
	} else if (socketRequest == -1) {
		perror("socketRequest");
		fprintf(stderr, "ERROR: Es kann kein Socket erstellt werden.\n");
		exit(EXIT_FAILURE);
	}
	memset(&address, 0, sizeof(address)); // Struktur Initialisieren.
	address.sin_family = AF_INET;
	address.sin_port = htons(destinationPort);
	res = inet_pton(AF_INET, destinationIp, &address.sin_addr);
	if (res < 0) {
		perror("wrong ip");
		fprintf(stderr,
				"ERROR: Es kann keine Konvertierung durchgeführt werden.\n");
		exit(EXIT_FAILURE);
	}
	//mit server verbinden
	if (connect(socketRequest, (struct sockaddr *) &address, sizeof(address))
			== -1) {
		perror("connect failed");
		close(socketRequest);
	}
	return socketRequest;
}

/**
 * Sendet einen DiscoveryRequest an einen ausgewählten Peer.
 */
void sendToPeer(enum message_types type, char* destinationIp, int destinationPort, char* message){
	int socketRequest = openSocketAndConnect(destinationIp, destinationPort);

	if(type == DISCOVERY_REQUEST || type == DISCOVERY_REPLY) {
		discoveryHeader disco;
		memset((void *) &disco, 0, sizeof(disco));
		disco.peers[0] = peers[0];

		commonHeader header;
		memset((void *) &header, 0, sizeof(header));
		header.version = PROTOCOL_VERSION;
		header.type = type;
		header.length = htons(sizeof(disco));
		disco.header = header;
		ssize_t bytes_send = send(socketRequest, (void*) &disco, sizeof(disco),0);
		if (bytes_send < 0 ) {
			fprintf(stderr, "ERROR; Send \n");
		} else if (bytes_send == 0) {
			printf("keine daten");
		}
	} else {
		messageHeader messageHead;
		memset((void *) &messageHead, 0, sizeof(messageHead));
		strcpy(messageHead.userMessage, message);
		commonHeader header;
		memset((void *) &header, 0, sizeof(header));
		header.version = PROTOCOL_VERSION;
		header.type = type;
		header.length = htons(sizeof(header));
		messageHead.header = header;
		ssize_t bytes_send = send(socketRequest, (void*) &messageHead, sizeof(messageHead),0);
		printf("Gesendete Bytes: %li \n",bytes_send);
		if (bytes_send < 0 ) {
			fprintf(stderr, "ERROR; Send \n");
		} else if (bytes_send == 0) {
			printf("keine daten");
		}
	}
	close(socketRequest);
}

void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
    	//peer->peerAddress = &(((struct sockaddr_in*)sa)->sin_addr);
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

/**
 * Sendet alle 4 Sekunden einen Discovery-Request.
 */
void *sendDiscoveryRequest() {
	while(1) {
		sendToPeer(DISCOVERY_REQUEST, ip, port, NULL);
		sleep(DISCOVERY_REQUEST_SLEEP);
	}
}

/**
 * Startet Empfangsserver für eingehende Nachrichten.
 */
void *startSocketForReceive() {
    fd_set master;    // master file descriptor list
    fd_set read_fds;  // temp file descriptor list for select()
    int fdmax;        // maximum file descriptor number

    int listener;     // listening socket descriptor
    int newfd;        // newly accept()ed socket descriptor
    struct sockaddr_storage remoteaddr; // client address
    socklen_t addrlen;

    char buf[256];    // buffer for client data
    int nbytes;

    char remoteIP[INET6_ADDRSTRLEN];

    int yes=1;        // for setsockopt() SO_REUSEADDR, below
    int i, j, rv;

    struct addrinfo hints, *ai, *p;

    FD_ZERO(&master);    // clear the master and temp sets
    FD_ZERO(&read_fds);

    // get us a socket and bind it
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    if ((rv = getaddrinfo(NULL, LOCAL_PORT2, &hints, &ai)) != 0) {
        fprintf(stderr, "selectserver: %s\n", gai_strerror(rv));
        exit(1);
    }

    for(p = ai; p != NULL; p = p->ai_next) {
        listener = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (listener < 0) {
            continue;
        }

        // lose the pesky "address already in use" error message
        setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

        if (bind(listener, p->ai_addr, p->ai_addrlen) < 0) {
            close(listener);
            continue;
        }

        break;
    }

    // if we got here, it means we didn't get bound
    if (p == NULL) {
        fprintf(stderr, "selectserver: failed to bind\n");
        exit(2);
    }

    freeaddrinfo(ai); // all done with this

    // listen
    if (listen(listener, 10) == -1) {
        perror("listen");
        exit(3);
    }

    // add the listener to the master set
    FD_SET(listener, &master);

    // keep track of the biggest file descriptor
    fdmax = listener; // so far, it's this one

    // main loop
    for(;;) {
        read_fds = master; // copy it
        if (select(fdmax+1, &read_fds, NULL, NULL, NULL) == -1) {
            perror("select");
            exit(4);
        }

        // run through the existing connections looking for data to read
        for(i = 0; i <= fdmax; i++) {
            if (FD_ISSET(i, &read_fds)) { // we got one!!
                if (i == listener) {
                    // handle new connections
                    addrlen = sizeof remoteaddr;
                    newfd = accept(listener,
                        (struct sockaddr *)&remoteaddr,
                        &addrlen);

                    if (newfd == -1) {
                        perror("accept");
                    } else {
                        FD_SET(newfd, &master); // add to master set
                        if (newfd > fdmax) {    // keep track of the max
                            fdmax = newfd;
                        }
                        printf("selectserver: new connection from %s on "
                            "socket %d\n",
                            inet_ntop(remoteaddr.ss_family,
                                get_in_addr((struct sockaddr*)&remoteaddr),
                                remoteIP, INET6_ADDRSTRLEN),
                            newfd);
                    }
                } else {
                    // handle data from a client
                	commonHeader header;
                	nbytes = recv(i, (void *) &header, sizeof(header), 0);
                	if(header.version == PROTOCOL_VERSION) {
						if(header.type == DISCOVERY_REQUEST || header.type == DISCOVERY_REPLY) {
							discoveryHeader discovery;
							int nBytesDiscovery = recv(i, (void *) &discovery.peers, sizeof(discovery.peers),0);
							if(nBytesDiscovery > 0) {
								printf("\nDiscovery von %s erhalten!\n", discovery.peers[0].username);
	//							printf("Peer Name %s:",discovery.peers->username);
							}
						} else {
							messageHeader messageHead;
							int nBytesMessage = recv(i, (void *) &messageHead.userMessage, sizeof(messageHead.userMessage),0);
							if(nBytesMessage > 0) {
								printf("messageHeader received %i bytes:\n", nBytesMessage);
								printf("\nNachricht von xy erhalten: %s\n",messageHead.userMessage);
							}
						}
					} else {
						printf("Received message with wrong version.\n");
					}
                    if (nbytes <= 0) {
                        // got error or connection closed by client
                        if (nbytes == 0) {
                            // connection closed
                            printf("selectserver: socket %d hung up\n", i);
                        } else {
                            perror("recv");
                        }
                        close(i); // bye!
                        FD_CLR(i, &master); // remove from master set
                    } else {
                        // we got some data from a client
                        for(j = 0; j <= fdmax; j++) {
                            // send to everyone!
                            if (FD_ISSET(j, &master)) {
                                // except the listener and ourselves
                                if (j != listener && j != i) {
                                    if (send(j, buf, nbytes, 0) == -1) {
                                        perror("send");
                                    }
                                }
                            }
                        }
                    }
                } // END handle data from client
            } // END got new incoming connection
        } // END looping through file descriptors
    } // END for(;;)--and you thought it would never end!

    return 0;
}

/**
 * Fragt den Username beim Start des Programms ab.
 */
char* getUsername() {
    printf("Bitte geben Sie einen Benutzernamen ein: \n");
	fgets(username, MAX_SIZE_USERNAME, stdin);
	//printf("Ihr Username ist: %s\n",username);
	//username in die liste schreiben
	//strcpy(peer->userName,username);
	return username;
}

/**
 * Einlesen der Ziel-IP und -Port.
 */
void getConnectionInfo() {
	printf("Bitte geben Sie eine IP zum Verbinden ein: ");
	memset((void *) &ip, 0, 16 * sizeof(char));
	fgets(ip, 17, stdin);
	//printf("Die eingegebene IP lautet %s", ip);
	printf("\nBitte geben Sie einen Port zum Verbinden ein: ");
	scanf("%i",&port);
	//printf("\nDer eingegebene Port lautet %i", port);
}

/**
 * Initialisiert die Peerliste mit dem eigenen Usernamen,
 * Ip und Port als erstes Element.
 */
void initializePeerList() {
	strcpy(peers[0].username, username);
	peers[0].port = LOCAL_PORT;
	inet_pton(AF_INET, localIp, &peers[0].address);
}

void getUserCommand() {
	while (1) {
		printf("\nBitte geben sie einen Befehl ein: ");
		memset((void  *) &command, 0, 255 * sizeof(char));
		fgets(command, 255, stdin);
		command[strcspn(command, "\n")] = 0;
		if (strcasecmp(command, "/disco") == 0) {
			//schicke discovery request
			sendToPeer(DISCOVERY_REQUEST, ip, port, NULL);
		} else if (strcasecmp(command, "/msg") == 0) {
			char message[MAX_MESSAGE_SIZE];
			char username_dest[MAX_SIZE_USERNAME];
			printf("An wen wollen Sie eine Nachricht schicken? \n");
			fgets(username_dest, MAX_SIZE_USERNAME, stdin);
			printf("\nGeben Sie bitte Ihre Nachricht ein: \n");
			fgets(message, MAX_MESSAGE_SIZE, stdin);
			username_dest[strcspn(username_dest, "\n")] = 0;
			sendToPeer(SEND_MSG, ip, port, message);
		} else {
			printf("Unbekannter Befehl.\n");
		}
	}
}

int main(int argc, char **argv) {
	getUsername();
	getConnectionInfo();
	initializePeerList();

	int disco = pthread_create(&discoveryThread, NULL, sendDiscoveryRequest, 0);
	if (disco == 1) {
		printf("ERROR");
		exit(-1);
	}

    int multiChatter = pthread_create(&receiverThread, NULL, startSocketForReceive, 0);
    if (multiChatter == 1) {
    	printf("ERROR");
    	exit(-1);
    };

    getUserCommand();
}
