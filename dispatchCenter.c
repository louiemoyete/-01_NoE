#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
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
              /** ** ** *
      Definitions for REQUEST_TAXI Request
              * ** ** **/
#define PICKUP_LOCATION_REQUEST 1
#define DROPOFF_LOCATION_REQUEST 2


							/** ** ** *
			Function :
				Initialise Dispatch Server
			Parametres :
				int pointer : serverSocket
				pointer representing the socket matching the server
				struct sockaddr_in pointer : serverAddress
				provided protocol independent structure
			Returns :
			 	N/A
			Local Variables :
				INADDR_ANY : ( Global from imported library )
				Allows the server to accept all UDP packets and TCP connection requests made for its port
				SERVER_PORT : ( Global from simulator.h )
				Value of 6000
				AF_INET : ( Global from imported library )
				Communication over a network
			Functionality :
				Initialises the dispatch server by creating a strem socket using socket().
				Then it zeroes the entire thing with memset().
				It converts the long primitive in INADDR_ANY from host format to network format using htonl()
				It converts the short primitive in SERVER_PORT from host format to network format using htons()
	* ** ** **/
void initializeDispatchServer( int *serverSocket, struct sockaddr_in  *serverAddress )
{
	int status;

// Create Socket
	*serverSocket = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );
	if( serverSocket < EMPTY )
	{
		printf( "SERVER : *ERROR* : Could Not Open the Socket. \n" );
		exit( FAIL );
	}

// Set it to zero
	memset( serverAddress, EMPTY, sizeof( *serverAddress ));
	serverAddress->sin_addr.s_addr = htonl( INADDR_ANY );
	serverAddress->sin_family = AF_INET;
	serverAddress->sin_port = htons(( unsigned short ) SERVER_PORT );

// Bind the Socket
	status = bind( *serverSocket, ( struct sockaddr * )serverAddress, sizeof( *serverAddress ));
	if( status < SUCCESS )
	{
		printf( "SERVER : *ERROR* : Could Not Bind the Socket\n" );
		exit( FAIL );
	}

// Set up the Socket
	status = listen( *serverSocket, MAX_REQUESTS );
	if( status < EMPTY )
	{
		printf( "SERVER : *ERROR* : Could Not Listen the Socket\n" );
		exit( FAIL );
	}
}

							/** ** ** *
			Function :
				handleIncomingRequests
			Parametres :
			 	Function Pointer
			Returns :
				N/A
			Local Variables :
				MAX_REQUESTS ( Global from simulator.h )
				Value of 100
			Functionality :
				Calls initializeDispatchServer() to start the socket
				Once the socket is opened, we assign an IP address to it from which to accept messaged with bind()
				Once the socket is bound, we listen for incoming requests with listen()
				Then we go into an infinite loop that handles the incoming requests :
				From stop.c : SHUTDOWN :
					Causes dispatch centre to go offline
				From request.c : REQUEST_TAXI :
					Contians 2 bytes. These are ( index of ) area to be picked up in and ( index of ) area to be dropped off in.
					Check for limitations ( on MAX_REQUESTS ) send YES ( Global from simulator.h ) or NO ( Global from simulator.h )
				From request.c : UPDATE :
					Contains bytes representing the taxi's x, y, plate, status and dropoff area.
					Nothing is sent back.
				From taxi.c : REQUEST_CUSTOMER :
					Contains Nothing.
					Checls for prending customer requests.
					Sends back NO ( Global form simulator.h ) or YES ( Global from simulator.h ). Chooses request that has been in queue the longest
							* ** ** **/
void *handleIncomingRequests( void *d )
{
  DispatchCenter  *dispatchCenter = d;
  int addressSize, bytesRcv, clientSocket, serverSocket, status;
  struct sockaddr_in clientAddress, serverAddress;
	unsigned char incomingRequest[ MAX_SIZE_REQUEST ];

// Start the Server
  initializeDispatchServer( &serverSocket, &serverAddress );

	while( 1 )
	{
		unsigned char serverResponse[ MAX_SIZE_REQUEST ];

// Set up the client
		addressSize = sizeof( clientAddress );
		clientSocket = accept( serverSocket,( struct sockaddr * )&clientAddress,&addressSize );
		if( clientSocket < EMPTY )
		{
			printf( "SERVER : *ERROR* : Could Not Accept Client Socket\n");
			exit( FAIL );
		}

// Incoming request
		bytesRcv = recv( clientSocket, incomingRequest, sizeof( incomingRequest ), EMPTY );

// // // / Handle Request : SHUTDOWN / // // //
		if( incomingRequest[ COMMAND ] == SHUTDOWN )
		{
			dispatchCenter->online = CLOSE_DISPATCH;
			break;
			close(serverSocket);
		}

// // // / Handle Request : REQUEST_TAXI / // // //
		if( incomingRequest[ COMMAND ] == REQUEST_TAXI )
		{
			if( dispatchCenter->numRequests != MAX_REQUESTS )
			{
				serverResponse[ COMMAND ] = YES;

// Create a new response, add it to the Response Array in DispatchCenter
				Request newRequest;
				newRequest.pickupLocation = incomingRequest[ PICKUP_LOCATION_REQUEST ];
				newRequest.dropoffLocation = incomingRequest[ DROPOFF_LOCATION_REQUEST ];
				dispatchCenter->requests[dispatchCenter->numRequests] = newRequest;
				++dispatchCenter->numRequests;
				send( clientSocket, serverResponse, sizeof( serverResponse ), EMPTY );
			}else
			{
				serverResponse[ COMMAND ] = NO;
				send( clientSocket, serverResponse, sizeof( serverResponse ), EMPTY );
			}
		}

// // // / Handle Request : UPDATE / // // //
		if ( incomingRequest[ COMMAND ] == UPDATE)
		{
			int taxiSendingUpdate;
			for( int i = EMPTY; i < dispatchCenter->numTaxis; i++ )
			{

// Look for the Taxi, Check the Plate Number
				if( dispatchCenter->taxis[ i ]->plateNumber == incomingRequest[ PLATE_NUM ] )
				{
					taxiSendingUpdate = i;
					break;
				}
			}

// Update its values, reconstructing the broken integer than was sent
			dispatchCenter->taxis[ taxiSendingUpdate ]->status = incomingRequest[ STATUS ];
			dispatchCenter->taxis[ taxiSendingUpdate ]->pickupArea = incomingRequest[ UPDATE_PICKUP_LOCATION ];
			dispatchCenter->taxis[ taxiSendingUpdate ]->dropoffArea = incomingRequest[ UPDATE_DROPOFF_LOCATION ];
			dispatchCenter->taxis[ taxiSendingUpdate ]->x =( incomingRequest[ X_COORDINATE_RESULT ] * RECONSTRUCT_INTEGERS ) + incomingRequest[ X_COORDINATE_REMAINDER ];
			dispatchCenter->taxis[ taxiSendingUpdate ]->y =( incomingRequest[ Y_COORDINATE_RESULT ] * RECONSTRUCT_INTEGERS ) + incomingRequest[ Y_COORDINATE_REMAINDER ];
		}

// // // / Handle Request : REQUEST_CUSTOMER / // // //
		if( incomingRequest[ COMMAND ] == REQUEST_CUSTOMER)
		{
			if ( dispatchCenter->numRequests != EMPTY )
			{
				serverResponse[ COMMAND ] = YES;
				serverResponse[ PICKUP_LOCATION_RESPONSE ] = dispatchCenter->requests[ FIRST_REQ_IN_LINE ].pickupLocation;
				serverResponse[ DROPOFF_LOCATION_RESPONSE ] = dispatchCenter->requests[ FIRST_REQ_IN_LINE ].dropoffLocation;
				send( clientSocket, serverResponse, sizeof( serverResponse ), EMPTY );

// Delete Request by shiftinf all requests in the array one spot to the left
				for( int i = EMPTY; i < dispatchCenter->numRequests-1; i++ )
				{
					dispatchCenter->requests[ i ] = dispatchCenter->requests[ i+1 ];
				}
				--dispatchCenter->numRequests;
			}else
			{
				serverResponse[ COMMAND ] = NO;
				send( clientSocket, serverResponse, sizeof( serverResponse ), EMPTY );
			}
		}

// Close the client after every response
		close(clientSocket);
	}

// Close the Server if the while loop ends
	close(serverSocket);
}
