#include "client.h"

#define LOCAL_PORT 15000
#define LOCAL_IP "141.22.27.106"
char username[USERNAME_REAL_SIZE];
char command[255];
char username[USERNAME_REAL_SIZE];
#define MAXBUFLEN 1024
//initialisiere einen Discovery Request
void sendDiscoveryRequestTo(char* destinationIp, int destinationPort){
	int socketRequest;
	struct sockaddr_in address;
	int res;
	//int size;

	//anlegen eines Sockets
	socketRequest = socket(AF_INET, SOCK_STREAM, 0);
	if (socketRequest > 0) {
			printf ("Socket wurde angelegt\n");
	}else if (socketRequest == -1) {
		perror("socketRequest");
		fprintf(stderr, "ERROR: Es kann kein Socket erstellt werden.\n");
		exit(EXIT_FAILURE);
	}
	memset(&address, 0, sizeof(address)); // Struktur Initialisieren.
	address.sin_family = AF_INET;
	address.sin_port = htons(destinationPort);
	//inet_aton(destinationIp, &address.sin_addr);

	// Destination = Speicher ergebnis.
	//res = inet_pton(AF_INET, SERVER_IP, &address.sin_addr);
	res = inet_pton(AF_INET, destinationIp, &address.sin_addr);
	if (res < 0) {
		perror("wrong ip");
		fprintf(stderr,
				"ERROR: Es kann keine Konvertierung durchgeführt werden.\n");
		exit(EXIT_FAILURE);
	}
	//mit server verbinden
	if (connect(socketRequest, (struct sockaddr *) &address, sizeof(address)) == -1) {
		perror("connect failed");
		close(socketRequest);
	}

	//----------------------------------------------------------------------
	//generieren des Headers
	discovery_header header;
	memset((void *) &header, 0, sizeof(discovery_header));
	//peer_info peers;
	//memset((void *) &peers, 0, sizeof(peer_info));

	header.version = SUPPORTED_VERSION;
	header.type = DISCOVERY_REQUEST;
	header.length = MAX_MESSAGE_SIZE;

	/*
	 * Informationen über die bisherigen Peers werden hier eingetragen.
	 * Informationen werden aus der Klasse PeerList.c entnommen.
	 *
	for(;;){
		peers.peer_address = ;
		peers.peer_port = ;
		peers.user_name = ;
		peers.next_peer = ;
	}
	*/
	ssize_t bytes_send = send(socketRequest, (void*) &header, sizeof(discovery_header), 0);
	if (bytes_send < 0) {
		fprintf(stderr, "ERROR; Send \n");
	}
	close(socketRequest);
}

void updatePeerList();

void sendMessageTo(char* message, int destination_port, char *destination_ip, int length_message) {
	send_msg_header header;
	memset((void *) &header, 0, sizeof(send_msg_header));

	header.version = SUPPORTED_VERSION;
	header.type = SEND_MSG;
	header.length = MAX_MESSAGE_SIZE;

	int create_socket;
	struct sockaddr_in address;
	//int size;

	//anlegen eines Sockets
	create_socket=socket(AF_INET, SOCK_STREAM, 0);
	if (create_socket > 0)
		printf ("Socket wurde angelegt\n");

	address.sin_family = AF_INET;
	address.sin_port = htons(destination_port);
	inet_aton(destination_ip, &address.sin_addr);

	//mit server verbinden
	if (connect ( create_socket, (struct sockaddr *) &address, sizeof(address)) == 0)
		printf ("Verbindung mit dem Server (%s) hergestellt\n", inet_ntoa(address.sin_addr));

	//Nachricht senden
	//ssize_t bytes_send = send(socket, (void*) &header, sizeof(discovery_header), 0);
	char payload[MAX_MESSAGE_SIZE];
	int max_message_size = length_message > MAX_MESSAGE_SIZE ? MAX_MESSAGE_SIZE :length_message;
	snprintf(payload, max_message_size, "%s", message);

	FILE *outstream = fdopen(create_socket, "w");

	if (fwrite(&header, sizeof(send_msg_header), 1, outstream) != 1) {
		perror("fwrite");
		fprintf(stderr, "ERROR: Kann nicht zum Message stream schreiben. \n");
	}
	if (fwrite(&payload, sizeof(char) * max_message_size, 1, outstream) != 1) {
		perror("fwrite");
		fprintf(stderr, "ERROR: Kann nicht zum Message stream schreiben. \n");
	}
	fflush(outstream);
	close (create_socket);
}

void startLocalServer() {
	struct sockaddr_in sa;
	int res;
	int SocketFD;

	SocketFD = socket(AF_INET, SOCK_STREAM, 0); // Erstellen des Sockets.
	if (SocketFD == -1) {
		perror("socket");
		fprintf(stderr, "ERROR: Es kann kein Socket erstellt werden.\n");
		exit(EXIT_FAILURE);
	}

	memset(&sa, 0, sizeof(sa)); // Struktur Initialisieren.

	sa.sin_family = AF_INET;
	sa.sin_port = htons(LOCAL_PORT);

	// Destination = Speicher ergebnis.
	res = inet_pton(AF_INET, LOCAL_IP, &sa.sin_addr);
	if (res < 0) {
		perror("wrong ip");
		fprintf(stderr,
				"ERROR: Es kann keine Konvertierung durchgeführt werden.\n");
		exit(EXIT_FAILURE);
	}

	//Connection zum Server aufbauen
	if (connect(SocketFD, (struct sockaddr *) &sa, sizeof sa) == -1) {
		perror("connect failed");
		close(SocketFD);
	}
}

/**
 * Behandeln der einkommenden Nachrichten vom Server.
 */
void* messageHandlerMain(void * socket_fd_p) {
	int socket_fd = *((int*) socket_fd_p);
	send_msg_header header;
	int flag = 1;
	while (flag) {
		memset((void *) &header, 0, sizeof(send_msg_header));

		//Recieven des Headers
		ssize_t numBytesRcvd = recv(socket_fd, (void*) &header, sizeof(send_msg_header), 0);
		if (numBytesRcvd == 0) {
			perror("No Connection");
			fprintf(stderr, ": Der Server hat die Verbindung geschlossen.\n");
			flag = 0;
		}

		if (header.version == SUPPORTED_VERSION) {
			// Ausgabe der Message
			if (header.type == SEND_MSG) {
				printf("\nNeue Nachricht: %s \n", header.user_message);
			} else if (header.type == DISCOVERY_REQUEST) {
				//aktualisiere meine Liste
				//merken von wem der Request kam und einen Reply erstellen und in die queue einreihen
			}
		} else {
			printf("Der Header enthaelt die unbekannte Versionnummer %d \n", header.version);
		}
	}
	return NULL;
}

void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}
void* messageHandlerMain2() {
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
	    if ((rv = getaddrinfo(NULL, LOCAL_PORT, &hints, &ai)) != 0) {
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
	                    if ((nbytes = recv(i, buf, sizeof buf, 0)) <= 0) {
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
}




void getUsername() {
    printf("Bitte geben Sie einen Benutzernamen ein: \n");
	fgets(username, USERNAME_REAL_SIZE, stdin);
	printf("Ihr Username ist: %s\n",username);
}

void getIPFromUsername(char username[]) {
	//hier wird der username in der Tabellegesucht und die entsprechende IP ermittelt, ggf. auch port
}

int main(int argc, char **argv) {
	getUsername();

	/**
	 * Starte lokalen "Server" um Nachrichten zu empfangen

	struct sockaddr_in sa;
	int res;
	int socketLocal;
	socketLocal = socket(AF_INET, SOCK_STREAM, 0); // Erstellen des Sockets.
	if (socketLocal == -1) {
		perror("socket");
		fprintf(stderr, "ERROR: Es kann kein Socket erstellt werden.\n");
		exit(EXIT_FAILURE);
	}
	memset(&sa, 0, sizeof(sa)); // Struktur Initialisieren.
	sa.sin_family = AF_INET;
	sa.sin_port = htons(LOCAL_PORT);
	// Destination = Speicher ergebnis.
	res = inet_pton(AF_INET, LOCAL_IP, &sa.sin_addr);
	if (res < 0) {
		perror("wrong ip");
		fprintf(stderr, "ERROR: Es kann keine Konvertierung durchgeführt werden.\n");
		exit(EXIT_FAILURE);
	}
	//Connection zum Server aufbauen
	if (connect(socketLocal, (struct sockaddr *) &sa, sizeof sa) == -1) {
		perror("connect failed");
		close(socketLocal);
	}*/

	/**
	**Thread starten um Nachrichten vom Server zu empfangen
	*/
	pthread_t messageHandlerThread2;
	int messageHandler = pthread_create(&messageHandlerThread2, NULL, messageHandlerMain2, 0);
	if (messageHandler) {
		printf("ERROR; return code from pthread_create() is %d\n", messageHandler);
		//close(socketLocal);
		exit(-1);
	}
	/*
	pthread_t messageHandlerThread;
	int* socketLocalServer = &socketLocal;
	int messageHandler = pthread_create(&messageHandlerThread, NULL, messageHandlerMain, (void*) socketLocalServer);
	if (messageHandler) {
		printf("ERROR; return code from pthread_create() is %d\n", messageHandler);
		close(socketLocal);
		exit(-1);
	}
	*/

	/**
	 * Benutzereingaben abfragen
	 */
	while (1) {
		printf("Bitte geben sie einen Befehl ein: \n");
		memset((void  *) &command, 0, 255 * sizeof(char));
		fgets(command, 255, stdin);
		command[strcspn(command, "\n")] = 0;
		if (strcasecmp(command, "/discovery") == 0) {
			//schicke discovery request
		} else if (strcasecmp(command, "/exit") == 0) {
			//userCloseConnection(SocketFD);
			//logoutflag = LOGOUT;
			//return 0;
		} else if (strcasecmp(command, "/msg") == 0) {
			char message[MAX_MESSAGE_SIZE];
			char username_dest[USERNAME_REAL_SIZE];
			printf("An wenn wollen Sie eine Nachricht schicken ? \n");
			fgets(username_dest, USERNAME_REAL_SIZE, stdin);
			printf("\n und nun bitte noch die Nachricht: \n");
			fgets(message, MAX_MESSAGE_SIZE, stdin);
			username_dest[strcspn(username_dest, "\n")] = 0;
			getIPFromUsername(username_dest);
			sendMessageTo(message, LOCAL_PORT, argv[1], MAX_MESSAGE_SIZE);
		} else {
			printf("Unbekannter Befehl \n");
		}
	}
}
