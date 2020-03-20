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
