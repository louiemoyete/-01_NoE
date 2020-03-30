#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include "simulator.h"

#define BUFFER_SIZE 30

void main() {
  // Set up the random seed
  srand(time(NULL));

  while(1) {
    	//generate two random areas indeces
	int randPickupArea = rand() % NUM_CITY_AREAS;
	int randDropoffArea = rand() % NUM_CITY_AREAS;
	
	char customer[BUFFER_SIZE];
	snprintf(customer, sizeof(customer), "./customer %d %d", randPickupArea, randDropoffArea);
	system(customer);

	printf(" %d and %d \n", randPickupArea, randDropoffArea );
    
    usleep(250000);   /// do not remove this
  }
}
