#include "client.h"

#define LOCAL_PORT 15000
#define LOCAL_IP "141.22.27.106"
int port = 15000;
char username[USERNAME_REAL_SIZE];


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
			}
		} else {
			printf("Der Header enthaelt die unbekannte Versionnummer %d \n", header.version);
		}
	}
	return NULL;
}

char command[255];
char username[USERNAME_REAL_SIZE];
#define BUF 1024
void getUsername() {
    printf("Bitte geben Sie einen Benutzernamen ein: \n");
	fgets(username, USERNAME_REAL_SIZE, stdin);
	printf("Ihr Username ist: %s\n",username);
}

void getIPFromUsername(char username[]) {
	//hier wird der username in der Tabellegesucht und die entsprechende IP ermittelt, ggf. auch port
}

int main(int argc, char **argv) {
	//startLocalServer();
	getUsername();

	/**
	 * Starte lokalen "Server" um Nachrichten zu empfangen
	 */
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
	}

	/**
	**Thread starten um Nachrichten vom Server zu empfangen
	*/
	pthread_t messageHandlerThread;
	int* socketLocalServer = &socketLocal;
	int messageHandler = pthread_create(&messageHandlerThread, NULL, messageHandlerMain, (void*) socketLocalServer);
	if (messageHandler) {
		printf("ERROR; return code from pthread_create() is %d\n", messageHandler);
		close(socketLocal);
		exit(-1);
	}


	while (1) {
		printf("Bitte geben sie einen Befehl ein: \n");
		memset((void  *) &command, 0, 255 * sizeof(char));
		fgets(command, 255, stdin);
		command[strcspn(command, "\n")] = 0;
		if (strcasecmp(command, "/info") == 0) {
			//infoLoad(SocketFD);
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
			sendMessageTo(message, port, argv[1], MAX_MESSAGE_SIZE);
		} else {
			printf("Unbekannter Befehl \n");
		}
	}
}
