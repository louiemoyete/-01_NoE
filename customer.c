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

#define MAX_SIZE_REQUEST 30
#define EMPTY 0
#define SUCCESS 0
#define FAIL -1
#define TOTAL_NUM_AREAS 6
/** ** ** *
Definitions for All Requests
* ** ** **/
#define COMMAND 0
/** ** ** *
Definitions for REQUEST_TAXI Request
* ** ** **/
#define PICKUP_LOCATION_REQUEST 1
#define DROPOFF_LOCATION_REQUEST 2

              /** ** ** *
      main in customer.c opens a stream socket with the server.
      Functionality :
        Communicating a REQUEST_TAXI.
        Sending a Request_buffer character array with :
        Index 0 : REQUEST_TAXI
        Index 1 : Number representing index in Area Names to be picked up from
        Index 2 : Number representing index in Area Names to be dropped off to
        Received a Response ot either confirm or deny the processed request
              * ** ** **/
void main( int argc, char *argv[] )
{
  int clientSocket,status, pickupArea, dropoffArea, bytesRcv;
  struct sockaddr_in clientAddress;
  unsigned char clientRequest[ MAX_SIZE_REQUEST ], serverResponse[ MAX_SIZE_REQUEST ];

// Create Socket
	clientSocket = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );
	if( clientSocket < SUCCESS )
  {
		 printf( "CUSTOMER : *ERROR* : Could Not Open Socket\n");
		 exit( FAIL );
	}

// Setup Socket
	memset( &clientAddress, EMPTY, sizeof( clientAddress ));
	clientAddress.sin_addr.s_addr = inet_addr( SERVER_IP );
  clientAddress.sin_family = AF_INET;
	clientAddress.sin_port = htons(( unsigned short )SERVER_PORT );

// Connect Socket
	status = connect( clientSocket, ( struct sockaddr * )&clientAddress, sizeof( clientAddress ));

	 if( status < SUCCESS )
  {
		 printf( "CLIENT : *ERROR* : Could Not Connect Socket\n");
		 exit( FAIL );
	 }

// Get the values given in the terminal
	pickupArea = ( unsigned char )atoi( argv[ PICKUP_LOCATION_REQUEST ] );
	dropoffArea = ( unsigned char )atoi( argv[ DROPOFF_LOCATION_REQUEST ] );
	if(( pickupArea >= NUM_CITY_AREAS )||( dropoffArea >= NUM_CITY_AREAS ))
  {
		printf( "CUSTOMER : *ERROR* : Out of Bounds ( Digits must be betwwen 0 and 5 )\n");
		exit( FAIL );
	}

// Build Request
  clientRequest[ COMMAND ] = REQUEST_TAXI;
	clientRequest[ PICKUP_LOCATION_REQUEST ] = pickupArea;
	clientRequest[ DROPOFF_LOCATION_REQUEST ] = dropoffArea;
	send( clientSocket, &clientRequest, sizeof( clientRequest ), EMPTY );

// Build Response
	bytesRcv = recv( clientSocket, serverResponse, sizeof( serverResponse ), EMPTY );
  if( serverResponse[ COMMAND ] == NO ){ printf( "Request Denied\n" );}

// Close and finish
	close( clientSocket );
}
