#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <signal.h>
#include "simulator.h"

#include "taxi.c"
#include "display.c"
#include "dispatchCenter.c"

int main() {
  DispatchCenter     ottawaDispatch;

  
  srand(time(NULL));
  ottawaDispatch.numTaxis = 0;
  ottawaDispatch.numRequests = 0;

  // Create a taxi in a random area of the city. 
  // ...

	//Create a taxi and allocate memory
	Taxi *taxi;
	taxi = calloc(1, sizeof(Taxi));

	//=====FIX THIS AT THE END=====//
	//Store it in the dispatch center
	ottawaDispatch.taxis[ottawaDispatch.numTaxis] = taxi;
	//increment taxis by 1
	ottawaDispatch.numTaxis++;

	//Place taxi in a random area
	taxi->plateNumber = 0;
	taxi->pickupArea = UNKNOWN_AREA;
	taxi->dropoffArea = UNKNOWN_AREA;
	//New taxis are available 
	taxi->status = AVAILABLE;
	//random area, set Y and X locations
	int random = rand() % NUM_CITY_AREAS;
	taxi->x = AREA_X_LOCATIONS[random];
	taxi->y = AREA_Y_LOCATIONS[random];
	taxi->currentArea = random;

	//start process for the taxi
	int PID;
	int status; 
	PID = fork();

	if (PID == 0 ) {
		runTaxi(taxi);
		exit(0);
	}else{
		taxi->pID = PID;
	}
  
  // Spawn a thread to handle incoming requests
  // ...

	//Create a thread
	pthread_t threadRequests;
	pthread_create( &threadRequests, NULL, handleIncomingRequests, &ottawaDispatch);

  // Spawn a thread to handle display
  // ...

	//Create a thread
	pthread_t threadDisplay;
	pthread_create( &threadDisplay, NULL, showSimulation, &ottawaDispatch);

  // Wait for the dispatch center thread to complete, from a STOP command
  // ...
	pthread_join(threadRequests, NULL);
	//pthread_join(threadDisplay, NULL);

  // Kill all the taxi processes
  // ...

	kill(PID, SIGKILL);

  // Free the memory
  // ...
  	free(taxi);
	
  printf("Simulation complete.\n");
}
