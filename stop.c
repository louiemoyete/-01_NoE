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
#define SUCCESS 0
#define FAIL -1
#define EMPTY 0
#define COMMAND 0

							/** ** ** *
			Main Function Starts stop.c with the purpose of shutting down the simulator
			Funcitonality :
				Creates socket for client ( this program ) to communicate with server
				Sets up the address for the socket to communicate to ( ServerAddress )
				Connects over to the Dispatch Centre
				Sends the SHUTDOWN command
				Finally, it closes the connection with the client
							* ** ** **/
void main()
{
	int clientSocket, status;
	struct sockaddr_in serverAddress;
	char clientRequest[ MAX_SIZE_REQUEST ];

// Create Socket
	clientSocket = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );
	if( clientSocket < SUCCESS )
	{
		 printf( "STOP : *ERROR* : Could Not Open Socket\n" );
		 exit( FAIL );
	}

// Setup Socket
	memset( &serverAddress, EMPTY, sizeof( serverAddress ));
	serverAddress.sin_addr.s_addr = inet_addr( SERVER_IP );
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_port = htons(( unsigned short ) SERVER_PORT );

// Connect Socket
	status = connect( clientSocket, ( struct sockaddr * ) &serverAddress, sizeof( serverAddress ));
	if( status < SUCCESS )
	{
		 printf( "STOP : *ERROR* : Could Not Connect\n" );
		 exit( FAIL );
	}

// Create Request
	clientRequest[ COMMAND ] = SHUTDOWN;
	send( clientSocket, clientRequest, sizeof( clientRequest ), EMPTY );
	close( clientSocket );
}
