#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>

#define MAX_SIZE_REQUEST 30
#define EMPTY 0
#define SUCCESS 0
#define FAIL -1
              /** ** ** *
      Definitions for All Requests
              * ** ** **/
#define COMMAND 0
              /** ** ** *
      Definitions for SHUTDOWN Request
              * ** ** **/
#define CLOSE_DISPATCH 0
              /** ** ** *
      Definitions for UPDATE Request
              * ** ** **/
#define PLATE_NUM 1
#define X_COORDINATE_RESULT 2
#define X_COORDINATE_REMAINDER 3
#define Y_COORDINATE_RESULT 4
#define Y_COORDINATE_REMAINDER 5
#define STATUS 6
#define UPDATE_PICKUP_LOCATION 7
#define UPDATE_DROPOFF_LOCATION 8
#define RECONSTRUCT_INTEGERS 256
              /** ** ** *
      Definitions for REQUEST_CUSTOMER Request
              * ** ** **/
#define FIRST_REQ_IN_LINE 0
#define PICKUP_LOCATION_RESPONSE 1
#define DROPOFF_LOCATION_RESPONSE 2

int clientSocket, bytesRcv;
struct sockaddr_in  clientAddress;

							/** ** ** *
			Function :
				Connect to Dispatch Center
			Parametres :
				Pointer int : sock
					Address of integer to hold return values of socket()
					struct sockaddr_in pointer : address
					Something I copied from Lanthiers notes
			Local Variables :
				int : Status :
					integer to hold return of connect()
			Returns :
				SUCCESS ( 0 ) or FAIL ( -1 )
			Functionality :
				Opens a stream socket using sock ( Parametre ).
				Sets it up to be zeroes
				Connects it to the Dispatch Server
							* ** ** **/
int connectToDispatchCenter( int *sock,  struct sockaddr_in *address )
{
	int status;

// Create Socket
	*sock = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );
	if( sock < SUCCESS )
	{
		 printf( "CLIENT : *ERROR* : Could Not Open Socket\n" );
		 return( FAIL );
	}

// Setup Socket
	memset( address, EMPTY, sizeof( *address ));
	address->sin_addr.s_addr = inet_addr( SERVER_IP );
	address->sin_family = AF_INET;
	address->sin_port = htons(( unsigned short )SERVER_PORT );

// Connect Socket
	status = connect( *sock,( struct sockaddr * )address, sizeof( *address ));
	if( status < SUCCESS )
	{
		 return ( FAIL );
	 }
  return ( SUCCESS );
}


							/** ** ** *
			Function :
				Run Taxi
			Parametres :
				Pointer to Taxi Struct : Taxi
				Taxi Struct ot be manipulated, brought from DispatchCenter.c
			Local Variables :
				Taxi thisTaxi
				Creates new Taxi and equals the values of the taxi given in the parametre
			Return :
				VOID
				Functionality :
					Handle Request for Customer :
						Send Req to the server. Read the Response. If the response contians a request for taxi,
						set the values of the taxi to those of the request
					Handle Update :
						Change the taxi's X and Y locations.
						Set the new values.
						Check whether the taxi has arrived to the Pickup or the Dropoff.
						Set the new vlaues accordingly.
							Either changing the taxi's values to the dropoff location
							Or resetting the values so it may bea vailable again.
						Send the new information to the server
							* ** ** **/
void runTaxi( Taxi *taxi )
{
  Taxi thisTaxi;
  thisTaxi.plateNumber = taxi->plateNumber;
  thisTaxi.currentArea = taxi->currentArea;
  thisTaxi.x = taxi->x;
  thisTaxi.y = taxi->y;
  thisTaxi.status = AVAILABLE;
  thisTaxi.pickupArea = UNKNOWN_AREA;
  thisTaxi.dropoffArea = UNKNOWN_AREA;
  thisTaxi.eta = 0;

	int deltaX, deltaY;
	unsigned char clientRequest[ MAX_SIZE_REQUEST ], serverResponse[ MAX_SIZE_REQUEST ], clientResponse[ MAX_SIZE_REQUEST ];

  while( 1 )
	{

//// // / Handle REQUEST CUSTOMER Request
		if( thisTaxi.status == AVAILABLE )
		{
			if( connectToDispatchCenter(&clientSocket, &clientAddress) == SUCCESS )
			{
// Build Client Request
				memset( clientRequest, EMPTY, sizeof( clientRequest ));
				clientRequest[ COMMAND ] = REQUEST_CUSTOMER;
				send( clientSocket, clientRequest, sizeof( clientRequest ), EMPTY );

// Read Server Response
				memset( serverResponse, EMPTY, sizeof( serverResponse ));
				bytesRcv = recv( clientSocket, serverResponse, 80, EMPTY );

// If the server allowed it
				if( serverResponse[ COMMAND ] == YES )
				{

// If the Pick up requested is the same as the taxi's current area
					if( serverResponse[ PICKUP_LOCATION_RESPONSE ] == thisTaxi.currentArea )
					{

// Set the values, ETA = 10 minutes
						thisTaxi.status = DROPPING_OFF;
						thisTaxi.currentArea = serverResponse[ PICKUP_LOCATION_RESPONSE ];
						thisTaxi.pickupArea = serverResponse[ PICKUP_LOCATION_RESPONSE ];
						thisTaxi.dropoffArea = serverResponse[ DROPOFF_LOCATION_RESPONSE ];
						thisTaxi.eta = TIME_ESTIMATES[ thisTaxi.pickupArea ][ thisTaxi.dropoffArea ];

// Else, if the pickup area is not the taxi's current area, ETA = Predefined Time Estimate
					}else if( serverResponse[ PICKUP_LOCATION_RESPONSE ] != thisTaxi.currentArea )
					{
						thisTaxi.status = PICKING_UP;
						thisTaxi.pickupArea = serverResponse[ PICKUP_LOCATION_RESPONSE ];
						thisTaxi.dropoffArea = serverResponse[ DROPOFF_LOCATION_RESPONSE ];
						thisTaxi.eta = TIME_ESTIMATES[ thisTaxi.currentArea ][ thisTaxi.pickupArea ];
					}
				}
// Close Socket
				close( clientSocket );
			}

// // // / Handle UPDATE Response / // // //
		}else{
			if( connectToDispatchCenter( &clientSocket, &clientAddress ) == SUCCESS )
			{

// If the taxi's status is Picking Up
				if( thisTaxi.status == PICKING_UP )
				{
					deltaX =( AREA_X_LOCATIONS[ thisTaxi.pickupArea ]- AREA_X_LOCATIONS[ thisTaxi.currentArea ])/( TIME_ESTIMATES[ thisTaxi.currentArea ][ thisTaxi.pickupArea ]);
					deltaY =( AREA_Y_LOCATIONS[ thisTaxi.pickupArea ]- AREA_Y_LOCATIONS[ thisTaxi.currentArea ])/( TIME_ESTIMATES[ thisTaxi.currentArea ][ thisTaxi.pickupArea ]);

// If the taxi's status is Dropping Off
				}else
				{
					deltaX =( AREA_X_LOCATIONS[ thisTaxi.dropoffArea ]- AREA_X_LOCATIONS[ thisTaxi.pickupArea ])/( TIME_ESTIMATES[ thisTaxi.pickupArea ][ thisTaxi.dropoffArea ]);
					deltaY =( AREA_Y_LOCATIONS[ thisTaxi.dropoffArea ]- AREA_Y_LOCATIONS[ thisTaxi.pickupArea ])/( TIME_ESTIMATES[ thisTaxi.pickupArea ][ thisTaxi.dropoffArea ]);
				}

// Update the taxi's position
				thisTaxi.x =( thisTaxi.x + deltaX );
				thisTaxi.y =( thisTaxi.y + deltaY );
				--thisTaxi.eta;

// If the Taxi has arrived to the drop off location
				if(( thisTaxi.eta == SUCCESS )&&( thisTaxi.status == DROPPING_OFF ))
				{

// Set the Taxi's values
					thisTaxi.status = AVAILABLE;
					thisTaxi.currentArea = thisTaxi.dropoffArea;
					taxi->pickupArea = UNKNOWN_AREA;
					taxi->dropoffArea = UNKNOWN_AREA;
					thisTaxi.x = AREA_X_LOCATIONS[ thisTaxi.dropoffArea ];
					thisTaxi.y =  AREA_Y_LOCATIONS[ thisTaxi.dropoffArea ];

// If the Taxi has arrived to the pick up location
				}else if(( thisTaxi.eta == SUCCESS )&&( thisTaxi.status == PICKING_UP ))
	 			{

// Set the Taxi's values
					thisTaxi.status = DROPPING_OFF;
					thisTaxi.currentArea = UNKNOWN_AREA;
					thisTaxi.x = AREA_X_LOCATIONS[ thisTaxi.pickupArea ];
					thisTaxi.y =  AREA_Y_LOCATIONS[ thisTaxi.pickupArea ];
					thisTaxi.eta = TIME_ESTIMATES[ thisTaxi.pickupArea ][ thisTaxi.dropoffArea ];

				}

// Create Client Response
				memset( clientRequest, EMPTY, sizeof( clientRequest ));
				clientResponse[ COMMAND ] = UPDATE;
				clientResponse[ PLATE_NUM ] = thisTaxi.plateNumber;
				clientResponse[ STATUS ] = thisTaxi.status;
				clientResponse[ UPDATE_PICKUP_LOCATION ] = thisTaxi.pickupArea;
				clientResponse[ UPDATE_DROPOFF_LOCATION ] = thisTaxi.dropoffArea;
				clientResponse[ X_COORDINATE_RESULT ] =( thisTaxi.x / RECONSTRUCT_INTEGERS );
				clientResponse[ X_COORDINATE_REMAINDER ] =( thisTaxi.x % RECONSTRUCT_INTEGERS );
				clientResponse[ Y_COORDINATE_RESULT ] =( thisTaxi.y / RECONSTRUCT_INTEGERS );
				clientResponse[ Y_COORDINATE_REMAINDER ] =( thisTaxi.y % RECONSTRUCT_INTEGERS );
				send( clientSocket, clientResponse, sizeof( clientResponse ), EMPTY );
			}

//Close the connection
		close( clientSocket );
		}
	  usleep( 50000 );
	}
}
