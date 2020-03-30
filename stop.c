#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "simulator.h"

#define BUFFER_SIZE 30


void main() {

	int clientSocket;
	struct sockaddr_in serverAddress;
	int status;

	// Create the client socket
	clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (clientSocket < 0) {
		 printf("Client error : could not open socket.\n");
		 exit(-1);
	}

	// Setup address
	memset(&serverAddress, 0, sizeof(serverAddress));
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_addr.s_addr = inet_addr(SERVER_IP);
	serverAddress.sin_port = htons((unsigned short) SERVER_PORT);

	//Connect to Dispatch server
	status = connect(clientSocket, (struct sockaddr *) &serverAddress, sizeof(serverAddress));

	 if (status < 0) {
		 printf("client error: could not connect.\n");
		 exit(-1);
	 }

	//Create buffer to send
	char message[BUFFER_SIZE];
	message[0] = SHUTDOWN;

	//send message to client
	send(clientSocket, message, sizeof(message), 0);

	//Close connection
	 close(clientSocket); 
	 //printf("client: shutting down.\n");




}

