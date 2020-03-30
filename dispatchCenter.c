#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>

#define BUFFER_SIZE 30
#define OFFLINE 0


// Initialize the dispatch center server by creating the server socket, setting up the server address,
// binding the server socket and setting up the server to listen for taxi and customer clients.
void initializeDispatchServer(int *serverSocket, struct sockaddr_in  *serverAddress) {
	int status;

	*serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	
	//Created socket, check for error. 
	if (serverSocket < 0 ){
		printf(" Server error : could not open socket. \n");
		exit(-1);
	}

	memset(serverAddress, 0, sizeof(serverAddress));
	//serverAddress->sin_family = AF_LOCAL;
	serverAddress->sin_family = AF_INET;
	serverAddress->sin_addr.s_addr = htonl(INADDR_ANY);
	//serverAddress.sin_addr.s_addr = htonl("127.0.0.1");
	serverAddress->sin_port = htons( (unsigned short) SERVER_PORT);

}

// Handle client requests coming in through the server socket.  This code should run
// indefinitiely.  It should wait for a client to send a request, process it, and then
// close the client connection and wait for another client.  The requests that may be
// handled are SHUTDOWN (from stop.c), REQUEST_TAXI (from request.c) and UPDATE or
// REQUEST_CUSTOMER (from taxi.c) as follows:

//   SHUTDOWN - causes the dispatch center to go offline.

//   REQUEST_TAXI - contains 2 additional bytes which are the index of the area to be
//                  picked up in and the index of the area to be dropped off in. If
//                  the maximum number of requests has not been reached, a single YES
//                  byte should be sent back, otherwise NO should be sent back.

//   REQUEST_CUSTOMER - contains no additional bytes.  If there are no pending customer
//                      requests, then NO should be sent back.   Otherwise a YES should
//                      be sent back followed by the pickup and dropoff values for the
//                      request that has been waiting the logest in the queue.

//   UPDATE - contains additional bytes representing the taxi's x, y plate, status and
//            dropoff area.  Nothing needs to be sent back to the taxi.

void *handleIncomingRequests(void *d) {
  DispatchCenter  *dispatchCenter = d;

  int                 serverSocket;
  struct sockaddr_in  serverAddress;

	struct sockaddr_in  clientAddr;
	int clientSocket;
	int bytesRcv;
	int status;
	int addrSize;
	char buffer[BUFFER_SIZE];
	

  // Initialize the server
  initializeDispatchServer(&serverSocket, &serverAddress);

	//BIND THE SERVER SOCKET
	 status = bind(serverSocket, (struct sockaddr *) &serverAddress, sizeof(serverAddress));
	//Check for error
	if (status < 0){
		printf("Server error: could not bind socket. %s \n");
		//strerror(errno);
		exit(-1);

	}
	
	//Socket listens to incomming request
	status = listen(serverSocket, MAX_REQUESTS);
	//check for error
	if ( status < 0){
		printf("Server error could not listen on socket. \n");
		exit(-1);
	}
	

  // REPLACE THE CODE BELOW WITH YOUR OWN CODE
	 while (1) {
		addrSize = sizeof(clientAddr);
	 	clientSocket = accept(serverSocket,(struct sockaddr *)&clientAddr,&addrSize);

		 if (clientSocket < 0) {
			 printf(" Server error:  could accept incoming client connection.\n");
			 exit(-1);
		 }
		//printf(" Connected.\n");
		//Talk to client
		//while(1){
			
			//Get message
			bytesRcv = recv(clientSocket, buffer, sizeof(buffer), 0);
			// put zero at the end 
			//buffer[bytesRcv] = 0;
			//printf(" Server : Request received \n");
			
			
			//======= CUSTOMER REQUESTED ======= // 
			if(buffer[0] == REQUEST_CUSTOMER){

				//create response buffer
				char response[BUFFER_SIZE];
				//printf("Number of requests %d\n", dispatchCenter->numRequests);
				if (dispatchCenter->numRequests != 0){ 

					// Construct Buffer with message to send
					response[0] = YES; //Yes to taxi
					response[1] = dispatchCenter->requests[0].pickupLocation;
					response[2] = dispatchCenter->requests[0].dropoffLocation;

					//store update command
					//response[3] = UPDATE;
					//SEND BUFFER
					send(clientSocket, response, sizeof(response), 0);

					//remove request
					for (int i = 0; i < dispatchCenter->numRequests-1; i++){
						dispatchCenter->requests[i] = dispatchCenter->requests[i+1];
					}
					//printf("A request for a customer has been made\n");
				
				}else{
					// Construct Buffer with message to send
					response[0] = NO; //No to taxi
					send(clientSocket, response, sizeof(response), 0);
					//printf("Server : A request for a customer has benn denied\n");
				}
				
			}

			//======= UPDATE REQUESTED ======= //
			if ( buffer[0] == UPDATE){
				//Information recieved from Taxi client
				int taxiIndex;
				//Find taxi
				for (int i = 0; i < dispatchCenter->numTaxis; i++){
					//check if the plate number matches the one that was recieved
					if (dispatchCenter->taxis[i]->plateNumber = buffer[1]){
						//stores taxi index of Array of taxis in Dispatch Centre
						taxiIndex = i;

					}
				}
				//Taxi plate number (I GUESS I DONT HAVE TO UPDATE THE PLATE NUMBER)
				//dispatchCenter->taxis[taxiIndex].x = buffer[2];
		
				//Current taxi x location 
				//(the x and y are int, and we need to send char)
				dispatchCenter->taxis[taxiIndex]->x = buffer[2];

				//Current taxi y location
				dispatchCenter->taxis[taxiIndex]->y = buffer[3];

				//Current taxi status
				dispatchCenter->taxis[taxiIndex]->status = buffer[4];

				//Current taxi dropoffArea
				//===========NOT SURE ABOUT THIS============//
				//dispatchCenter->taxis[taxiIndex]->dropoffArea = AREA_NAMES[(buffer[5])];

				//printf("An update for the taxi has been made\n");


			}
			//========= TAXI REQUESTED ======== //
			if(buffer[0] == REQUEST_TAXI){

				//printf("Server : A taxi request has been recieved.");
				//Create response buffer
				char response[BUFFER_SIZE];			

				if(dispatchCenter->numRequests != MAX_REQUESTS){
					
					// Construct Buffer with message to send
					response[0] = YES; //Yes to customer
					// Create a new request 
					Request newRequest;
					newRequest.pickupLocation = AREA_NAMES[(buffer[1])];
					newRequest.dropoffLocation = AREA_NAMES[(buffer[2])];	
					// Add the request to the request array ( first in - first out queue)
					dispatchCenter->requests[dispatchCenter->numRequests] = newRequest;
					// Increment the number of requests
					dispatchCenter->numRequests +=1;	
					//send message
					send(clientSocket, response, sizeof(response), 0);

					//printf("Server : The request for a taxi has been handled.");
				}else{
					// Construct Buffer with message to send
					response[0] = NO; //No to customer

					//send message
					send(clientSocket, response, sizeof(response), 0);

					//printf("Server : The request for a taxi has been denied.");				
				}
					
			}		
			
			//======= SHUTDOWN REQUESTED ======= //
			if(buffer[0] == SHUTDOWN){
				
				//sets status to offline 
				dispatchCenter->online = OFFLINE;
				//printf("The Ottawa dispatch center is offline.");
				//break the loop
				break;
		
			}
			
		//}
		
		//printf("Closing client connection\n");
		close(clientSocket);
		/*// If the client shutsdown, the server shuts down
		if(buffer[0] == SHUTDOWN){
			break;
		}*/
	}	
	close(serverSocket);
	//printf("Shutting down server\n");

	
}
