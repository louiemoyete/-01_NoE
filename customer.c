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

// This represents a customer request.  There should be two command line arguments representing the
// pickup location and dropoff location.  Each should be a number from 0 to the NUM_CITY_AREAS.
void main(int argc, char *argv[]) {
  int                 clientSocket;  // client socket id
  struct sockaddr_in  clientAddress; // client address
	int status;
	int pickupArea;
	int dropoffArea;
	int bytesRcv;
	
	// Create the client socket
	clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (clientSocket < 0) {
		 printf("Customer error : could not open socket.\n");
		 exit(-1);
	}

	// Setup address
	memset(&clientAddress, 0, sizeof(clientAddress));
	clientAddress.sin_family = AF_INET;
	clientAddress.sin_addr.s_addr = inet_addr(SERVER_IP);
	clientAddress.sin_port = htons((unsigned short) SERVER_PORT);

	
	//Connect to Dispatch server
	status = connect(clientSocket, (struct sockaddr *) &clientAddress, sizeof(clientAddress));

	 if (status < 0) {
		 printf("client error: could not connect.\n");
		 exit(-1);
	 }

	//printf("Customer : connected.\n");

	//Get pickup and dropoff area from commands
	pickupArea = atoi ( argv[1] );
	dropoffArea = atoi ( argv[2] );
	

	printf(" PICKUPAREA CUSTOMER: %d\n", pickupArea );
	printf(" DROPOFFAREA CUSTOMER : %d\n", dropoffArea );

	//Check for command erros
	if ( (pickupArea >= NUM_CITY_AREAS) || (dropoffArea >= NUM_CITY_AREAS) ){
		printf("Customer error : Choose an area between 0 - 5\n");
		exit(-1);

	}

	//Create buffer to send
	char message[BUFFER_SIZE];
	message[0] = REQUEST_TAXI;
	message[1] = pickupArea;
	message[2] = dropoffArea;
	/*char message[BUFFER_SIZE];
	snprintf(message, sizeof(message), "%d %d %d", REQUEST_TAXI, pickupArea, dropoffArea);*/

	//printf(" PICKUPAREA CUSTOMER buffer: %d\n", message[1] );
	//printf(" DROPOFFAREA CUSTOMER buffer : %d\n", message[2] );


	//printf("Customer : the message was created\n");
	//printf("Customer : message[0] : %d\n", message[0]);

	//send message to client
	send(clientSocket, message, sizeof(message), 0);
	//printf("Customer : the message was sent\n");
	
	//Create buffer to recieve responde
	char response[BUFFER_SIZE];
	//Get response from Dispatch center
	bytesRcv = recv(clientSocket, response, sizeof(response), 0);

	printf("Customer : a response from Dispatch has been received\n");
	if ( response[0] == NO){
		printf("Customer error : the taxi request was denied.\n");
		exit(-1);
	}


	//Close connection
	 close(clientSocket); 
	 printf("customer : shutting down.\n");

}

