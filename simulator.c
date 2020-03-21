#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include "simulator.h"

#include "taxi.c"
#include "display.c"
#include "dispatchCenter.c"
/*********************************************************************************
The simulator.c file contains the code for the main application.
You need to add code so that the function does the following:

• It must use fork() to start a new process for that taxi … calling the runTaxi()
function in the taxi.c file … passing newly created taxi as its parameter.  Upon
 returning from the runTaxi() function, the taxi process should simply exit.

• It should spawn a thread to handle incoming requests.   This thread should call
the handleIncomingRequests() function in the dispatchCenter.c file and pass in a
 pointer to the ottawaDispatch dispatch center.

• It should spawn a thread to handle the displaying of the city.   This thread
should call the showSimulation() function in the display.c file and pass in a
pointer to the ottawaDispatch dispatch center.

• The code should then wait for the request-handling thread to complete and it
should then kill all running taxi processes.

• The code should free all allocated memory so that valgrind shows no leaks.
********************************************************************************/

int main() {
  DispatchCenter     ottawaDispatch;


  srand( time( NULL ));
  ottawaDispatch.numTaxis = 0;
  ottawaDispatch.numRequests = 0;

  Taxi *taxi;
  // Dynamically Allocate a Taxi
  taxi = calloc( 1, sizeof( Taxi ));
                                          //taxi->x = 50;
                                          // printf("%d", taxi->x);
                                          // This is how we tap into taxi
  // Store it in Dispatch Centre
  ottawaDispatch.taxis[ 0 ] = taxi;

  // Plate number is equal to zero
  taxi->plateNumber = 0;
  // Pickup/Dropoff areas should be UNKNOWN_AREA.
  taxi->pickupArea = UNKNOWN_AREA;
  taxi->dropoffArea = UNKNOWN_AREA;
  // New taxis should start off being available for customers.
  taxi->status = AVAILABLE;
  // Create a random variable to hold the random area of the city chosen
  int random_index = ( rand()%6 );
  // Set the taxi to that location
  taxi->x = AREA_X_LOCATIONS[ random_index ];
  taxi->y = AREA_Y_LOCATIONS[ random_index ];
  taxi->currentArea = AREA_NAMES[ random_index ];
  /** to Delete : This Checks **/
  printf( "ottawaDispatch.taxis x %d y %d ", ottawaDispatch.taxis[ 0 ]->x, ottawaDispatch.taxis[ 0 ]->y );
  printf( "location AREA : %s ; of taxi : %c  ", AREA_NAMES[ random_index ], taxi->currentArea );
  /** to Delete : This Checks **/

  //  Create int childPID for fork()
  int status, childPID;
  childPID = fork();
  // Call runTaxi on the Child and kill it with the parent once runTaxi is done
  if( childPID == 0 )
  {
    runTaxi( taxi );        // NOTEE, THE CHILD SHOULD BE KILLED BY THE PARENT ONCE runTaxi IS OVER. HOW TO KILL INFINITE LOOP ?
    exit( 0 );
  }

  // Create integers of pthread_t variable ( handlers of new threads )
  pthread_t t1;
  // Create a thread with pthread_t, NULL, pointer to a function that starts the thread, a parametre to that function
  pthread_create( &t1, NULL, handleIncomingRequests, &ottawaDispatch );

  // Spawn a thread to display the city
  pthread_create( &t1, NULL, showSimulation, &ottawaDispatch );
  // Spawn a thread to handle incoming requests
  // ...

  // Spawn a thread to handle display
  // ...

  // Wait for the dispatch center thread to complete, from a STOP command
  // ...

  // Kill all the taxi processes
  // ...

  // Free the memory
  // ...

  printf("Simulation complete.\n");
}
