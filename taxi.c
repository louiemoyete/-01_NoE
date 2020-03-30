#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define BUFFER_SIZE 30
#define ARRIVED 0
#define CENTER_X_LOCATION 4
#define CENTER_Y_LOCATION 4

// The dispatch center server connection ... when made
int                 clientSocket;  // client socket id
struct sockaddr_in  clientAddress; // client address
int bytesRcv;


// Set up a client socket and connect to the dispatch center server.  Return -1 if there was an error.
int connectToDispatchCenter(int *sock,  struct sockaddr_in *address) {

	int status;

	// Create the client socket
	*sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sock < 0) {
		 printf("Client error : could not open socket.\n");
		 return -1;
	}

	// Setup address
	memset(address, 0, sizeof(address));
	address->sin_family = AF_INET;
	address->sin_addr.s_addr = inet_addr(SERVER_IP);
	address->sin_port = htons((unsigned short) SERVER_PORT);

	//Connect to Dispatch server
	status = connect(*sock, (struct sockaddr *) address, sizeof(clientAddress));

	 if (status < 0) {
		 printf("client taxi error: could not connect.\n");
		 exit(-1);
	 }


  return 0;
}



// This code runs the taxi forever ... until the process is killed
void runTaxi(Taxi *taxi) {
  // Copy the data over from this Taxi to a local copy
  Taxi   thisTaxi;
  thisTaxi.plateNumber = taxi->plateNumber;
  thisTaxi.currentArea = taxi->currentArea;
  thisTaxi.x = taxi->x;
  thisTaxi.y = taxi->y;
  thisTaxi.status = AVAILABLE;
  thisTaxi.pickupArea = UNKNOWN_AREA;
  thisTaxi.dropoffArea = UNKNOWN_AREA;
  thisTaxi.eta = 0;

	//Moving taxi X / Y locations
	int deltaX;
	int deltaY;

  // Go into an infinite loop to request customers from dispatch center when this taxi is available
  // as well as sending location updates to the dispatch center when this taxi is picking up or dropping off.
  while(1) {
    // WRITE SOME CODE HERE

	//If taxi available - contact dispatch to get request
	if (thisTaxi.status == AVAILABLE){
		//Connect with Dispatch
		if ( connectToDispatchCenter(&clientSocket, &clientAddress) == 0 ){

			//Create buffer to send
			char message[BUFFER_SIZE];
			message[0] = REQUEST_CUSTOMER;

			//send request to client
			send(clientSocket, message, sizeof(message), 0);


			//Get response from server
			char response[BUFFER_SIZE];
			bytesRcv = recv(clientSocket, response, 80, 0);
			//printf("%d\n", response[0]);
			//printf("%d\n", response[1]);
			//printf("%d\n", response[2]);

			//===REQUEST CUSTOMER====//
			if ( response[0] == YES ){
				//printf("Taxi : the customer request was accepted.\n");
				//If pickup area is the same as the current
				if ( response[1] == thisTaxi.currentArea ){			
					//=========CHANGE THIS SO THAT IS USES THE 2D TIME_ESTIMATES ARRAY=======//
					//estimated time of arrival, in this case the value should be 10min or 0min
					//thisTaxi.eta = TIME_ESTIMATES[thisTaxi.pickupArea][thisTaxi.dropoffArea];
					thisTaxi.eta = 0;
					//Pick area is the current area
					thisTaxi.currentArea = response[1];
					//set pick area ll
					thisTaxi.pickupArea = response[1];
					//set dropoffArea
					thisTaxi.dropoffArea = response[2];
					//set taxi status
					thisTaxi.status = DROPPING_OFF;
					//printf("ETA ( PickupArea == currentArea ) : %d\n", thisTaxi.eta);

				}else if (response[1] != thisTaxi.currentArea){ //If the pick area is not the current area

					//set taxi as Picking up
					thisTaxi.status = PICKING_UP;
					//set pick area 
					printf(" CURRENT AREA BEFORE : %d\n", thisTaxi.currentArea );
					printf(" PICKUPAREA BEFORE: %d\n", thisTaxi.pickupArea );
					printf(" DROPOFFAREA BEFORE : %d\n", thisTaxi.dropoffArea );
					thisTaxi.pickupArea = response[1];
					//set dropoffArea
					thisTaxi.dropoffArea = response[2];
					printf(" PICKUP response : %d\n", response[1] );
					printf(" DROPOFF response : %d\n", response[2] );
					printf(" CURRENT AREA AFTER : %d\n", thisTaxi.currentArea );
					printf(" PICKUPAREA AFTER: %d\n", thisTaxi.pickupArea );
					printf(" DROPOFFAREA AFTER : %d\n", thisTaxi.dropoffArea );
					//set time estimate - pick up area to drop off area
					/// ==== IS THIS THE TIME FROM TAXI CURRENT AREA TO PICK UP AREA ===////
					thisTaxi.eta = TIME_ESTIMATES[thisTaxi.currentArea][thisTaxi.pickupArea];
					printf("ETA ( PickupArea != currentArea ) : %d\n", thisTaxi.eta);
				}
			}/*else if ( response[0] == NO ){
				printf("Taxi error : the customer request was denied.\n");
				//=======NOT SURE IF I SHOULD BREAK=======//
				break;
			}*/

		}

	} else { //If taxi is Dropping off or Picking up - contact dispatch to send an update
		
		//Connect with Dispatch
		if ( connectToDispatchCenter(&clientSocket, &clientAddress) == 0 ){

			//(x, y) locations of moving taxi
			//Create buffer to send
			char message[BUFFER_SIZE];
			message[0] = UPDATE;
			message[1] = thisTaxi.plateNumber;
			message[2] = thisTaxi.x;
			message[3] = thisTaxi.y;
			message[4] = thisTaxi.status;
			message[5] = thisTaxi.dropoffArea;

			if(thisTaxi.status == PICKING_UP){
				deltaX = ( AREA_X_LOCATIONS[thisTaxi.pickupArea] - AREA_X_LOCATIONS[thisTaxi.currentArea]) / thisTaxi.eta;
				deltaY = ( AREA_Y_LOCATIONS[thisTaxi.pickupArea] - AREA_Y_LOCATIONS[thisTaxi.currentArea]) / thisTaxi.eta;

			} else if (thisTaxi.status == DROPPING_OFF){

				deltaX = ( AREA_X_LOCATIONS[thisTaxi.dropoffArea] - AREA_X_LOCATIONS[thisTaxi.pickupArea]) / thisTaxi.eta;
				deltaY = ( AREA_Y_LOCATIONS[thisTaxi.dropoffArea] - AREA_Y_LOCATIONS[thisTaxi.pickupArea]) / thisTaxi.eta;

			}

			//new position
			thisTaxi.x = thisTaxi.x + deltaX;
			thisTaxi.y = thisTaxi.y + deltaY;
			thisTaxi.eta -= 1;

			printf("TIME ESTIMATE REMAINING %d\n", thisTaxi.eta);


			if(thisTaxi.status == DROPPING_OFF && thisTaxi.eta == ARRIVED ){
				//Set X and Y location to DROPOFF AREA
				thisTaxi.x = AREA_X_LOCATIONS[thisTaxi.dropoffArea];
				thisTaxi.y =  AREA_Y_LOCATIONS[thisTaxi.dropoffArea];
				//set taxi available
				thisTaxi.status = AVAILABLE;
				//set currentArea to dropoffArea
				thisTaxi.currentArea = thisTaxi.dropoffArea;
				//set pickup and dropoff areas to unkown
				taxi->pickupArea = UNKNOWN_AREA;
				taxi->dropoffArea = UNKNOWN_AREA;

			} else if (thisTaxi.status == PICKING_UP && thisTaxi.eta == ARRIVED) {
				//Set X and Y location to PICK UP AREA
				thisTaxi.x = AREA_X_LOCATIONS[thisTaxi.pickupArea];
				thisTaxi.y =  AREA_Y_LOCATIONS[thisTaxi.pickupArea];
				//set taxi to droppping off status
				thisTaxi.status = DROPPING_OFF;
				//set currentArea to dropoffArea
				thisTaxi.currentArea = UNKNOWN_AREA;
				//set time estimate - pick up area to drop off area
				thisTaxi.eta = TIME_ESTIMATES[thisTaxi.pickupArea][thisTaxi.dropoffArea];			

			}


			//send request to client
			send(clientSocket, message, sizeof(message), 0);

		}
	}


    usleep(50000);  // A delay to slow things down a little
  }
}


