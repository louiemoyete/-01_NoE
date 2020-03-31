#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <signal.h>
#include "simulator.h"
#include "taxi.c"
#include "display.c"
#include "dispatchCenter.c"

#define EMPTY 0
#define SUCCESS 0
              /** ** ** *
      Main Function Starts simulator.c with the purpose of starting the simulator
      Funcitonality :
        Sets Local variable for the random index, the ids, and the threads
        Sets the dispatch numTaxi var and numRequests var to EMPTY ( 0 )
        Creates 10 Taxi struct. Dynamically Allocates it. Sets all its properties
        Forks the taxi, running ruNTaxi() with the child
        Upon returning from the process, the taxi process exits
        Spawns a thread to handle incoming requests
        Spawns a thread to display the city
        Waits for the handleIncomingRequests thread to complete and kill all taxis
        Frees all memory
              * ** ** **/
int main()
{
  DispatchCenter ottawaDispatch;
  srand( time( NULL ));
  int child_PID, status, randInt;
  pthread_t threadRequests, threadDisplay;

  ottawaDispatch.numTaxis = EMPTY;
  ottawaDispatch.numRequests = EMPTY;

	for (int i = EMPTY; i < 10; i++){
// Create Taxi
		Taxi *newTaxi;
		randInt = rand() % NUM_CITY_AREAS;

    newTaxi = calloc( 1, sizeof( newTaxi ));
		ottawaDispatch.taxis[ottawaDispatch.numTaxis] = newTaxi;
		newTaxi->plateNumber = ottawaDispatch.numTaxis;
		ottawaDispatch.numTaxis++;
		newTaxi->status = AVAILABLE;
		newTaxi->pickupArea = UNKNOWN_AREA;
    newTaxi->dropoffArea = UNKNOWN_AREA;
		newTaxi->currentArea = randInt;
		newTaxi->x = AREA_X_LOCATIONS[randInt];
		newTaxi->y = AREA_Y_LOCATIONS[randInt];

// Start the process
		child_PID = fork();
		if( child_PID == SUCCESS )
    {
			runTaxi( newTaxi );
			exit( SUCCESS );
		}else
    {
			newTaxi->pID = child_PID;
		}
	}
// Handle Threads
	pthread_create( &threadRequests, NULL, handleIncomingRequests, &ottawaDispatch );
	pthread_create( &threadDisplay, NULL, showSimulation, &ottawaDispatch );
	pthread_join( threadRequests, NULL );

// Kill Taxis
	for( int i = EMPTY; i < ottawaDispatch.numTaxis; i++ )
  {
		kill( ottawaDispatch.taxis[ i ]->pID, SIGKILL );
  }
// Free Taxis
	for( int i = EMPTY; i < ottawaDispatch.numTaxis; i++ )
  {
		free( ottawaDispatch.taxis[ i ] ) ;
	}
  printf( "Simulation over and done with\n" );
}
