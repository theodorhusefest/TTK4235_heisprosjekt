#ifndef __IO_USER_H__
#define __IO_USER_H__

#include <stdbool.h>
#include "elev.h"


/** 
Stop_handler is the function that opens the door, turns off
lights at current floor and removes from queue. WILL TAKE 3 SECONDS
*/

void door_and_lights_handler( int current_floor); 

/**
Set_lights takes in the queue and set lights
*/

void set_ordered_lights(bool queue[N_FLOORS][N_BUTTONS]);

/**
Delay function
Param ms is milliseconds the function should take. 

*/
void delay(int ms);

#endif


