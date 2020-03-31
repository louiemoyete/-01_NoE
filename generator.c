#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include "simulator.h"

#define MAX_SIZE_REQUEST 30
#define EMPTY 0

              /** ** ** *
      Main in generator.c creates random numbers,
        calls on system() start the customer.c file
        with these random numbers as the Pick up and the Drop off locaitons
        for the Taxi in simulator
              * ** ** **/
void main()
{
  srand( time( NULL ));
  int random_Pickup, random_Dropoff;
	char clientRequest[ MAX_SIZE_REQUEST ];

  while( 1 )
  {
	random_Pickup = rand() % NUM_CITY_AREAS;
	random_Dropoff = rand() % NUM_CITY_AREAS;
  memset( clientRequest, EMPTY, sizeof( clientRequest ));

// Set the random values and create a string to use with system()
	snprintf( clientRequest, sizeof( clientRequest ), "./customer %d %d", random_Pickup, random_Dropoff );
	system( clientRequest );
  usleep( 250000 );
  }
}
