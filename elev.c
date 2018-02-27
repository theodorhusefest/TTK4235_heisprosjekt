// Wrapper for libComedi Elevator control.
// These functions provides an interface to the elevators in the real time lab
//
// 2007, Martin Korsgaard


#include "channels.h"
#include "elev.h"
#include "io.h"

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

static const int lamp_channel_matrix[N_FLOORS][N_BUTTONS] = {
    {LIGHT_UP1, LIGHT_DOWN1, LIGHT_COMMAND1},
    {LIGHT_UP2, LIGHT_DOWN2, LIGHT_COMMAND2},
    {LIGHT_UP3, LIGHT_DOWN3, LIGHT_COMMAND3},
    {LIGHT_UP4, LIGHT_DOWN4, LIGHT_COMMAND4},
};


static const int button_channel_matrix[N_FLOORS][N_BUTTONS] = {
    {BUTTON_UP1, BUTTON_DOWN1, BUTTON_COMMAND1},
    {BUTTON_UP2, BUTTON_DOWN2, BUTTON_COMMAND2},
    {BUTTON_UP3, BUTTON_DOWN3, BUTTON_COMMAND3},
    {BUTTON_UP4, BUTTON_DOWN4, BUTTON_COMMAND4},
};

int elev_init(void) {
    int i;

    // Init hardware
    if (!io_init())
        return 0;

    // Zero all floor button lamps
    for (i = 0; i < N_FLOORS; ++i) {
        if (i != 0)
            elev_set_button_lamp(BUTTON_CALL_DOWN, i, 0);

        if (i != N_FLOORS - 1)
            elev_set_button_lamp(BUTTON_CALL_UP, i, 0);

        elev_set_button_lamp(BUTTON_COMMAND, i, 0);
    }

    // Clear stop lamp, door open lamp, and set floor indicator to ground floor.
    elev_set_stop_lamp(0);
    elev_set_door_open_lamp(0);
    elev_set_floor_indicator(0);

    // Return success.
    return 1;
}

void elev_set_motor_direction(elev_motor_direction_t dirn) {
    if (dirn == 0){
        io_write_analog(MOTOR, 0);
    } else if (dirn > 0) {
        io_clear_bit(MOTORDIR);
        io_write_analog(MOTOR, 2800);
    } else if (dirn < 0) {
        io_set_bit(MOTORDIR);
        io_write_analog(MOTOR, 2800);
    }
}

int elev_get_motor_direction(void) {
    if (io_read_analog(MOTOR) == 0) {
	return DIRN_STOP;
    }
    if (io_read_analog(MOTORDIR) == 0) {
	return DIRN_UP;
    }
    return DIRN_DOWN;
}

void elev_set_door_open_lamp(int value) {
    if (value)
        io_set_bit(LIGHT_DOOR_OPEN);
    else
        io_clear_bit(LIGHT_DOOR_OPEN);
}

int elev_get_obstruction_signal(void) {
    return io_read_bit(OBSTRUCTION);
}

int elev_get_stop_signal(void) {
    return io_read_bit(STOP);
}

void elev_set_stop_lamp(int value) {
    if (value)
        io_set_bit(LIGHT_STOP);
    else
        io_clear_bit(LIGHT_STOP);
}

int elev_get_floor_sensor_signal(void) {
    if (io_read_bit(SENSOR_FLOOR1))
        return 0;
    else if (io_read_bit(SENSOR_FLOOR2))
        return 1;
    else if (io_read_bit(SENSOR_FLOOR3))
        return 2;
    else if (io_read_bit(SENSOR_FLOOR4))
        return 3;
    else
        return -1;
}

void elev_set_floor_indicator(int floor) {
    assert(floor >= 0);
    assert(floor < N_FLOORS);

    // Binary encoding. One light must always be on.
    if (floor & 0x02)
        io_set_bit(LIGHT_FLOOR_IND1);
    else
        io_clear_bit(LIGHT_FLOOR_IND1);

    if (floor & 0x01)
        io_set_bit(LIGHT_FLOOR_IND2);
    else
        io_clear_bit(LIGHT_FLOOR_IND2);
}

int elev_get_button_signal(elev_button_type_t button, int floor) {
    assert(floor >= 0);
    assert(floor < N_FLOORS);
    assert(!(button == BUTTON_CALL_UP && floor == N_FLOORS - 1));
    assert(!(button == BUTTON_CALL_DOWN && floor == 0));
    assert(button == BUTTON_CALL_UP || button == BUTTON_CALL_DOWN || button == BUTTON_COMMAND);

    if (io_read_bit(button_channel_matrix[floor][button]))
        return 1;
    else
        return 0;
}

void elev_set_button_lamp(elev_button_type_t button, int floor, int value) {
    assert(floor >= 0);
    assert(floor < N_FLOORS);
    assert(!(button == BUTTON_CALL_UP && floor == N_FLOORS - 1));
    assert(!(button == BUTTON_CALL_DOWN && floor == 0));
    assert(button == BUTTON_CALL_UP || button == BUTTON_CALL_DOWN || button == BUTTON_COMMAND);

    if (value)
        io_set_bit(lamp_channel_matrix[floor][button]);
    else
        io_clear_bit(lamp_channel_matrix[floor][button]);
}


int elev_get_button_lamp(elev_button_type_t button, int floor) {
    return lamp_channel_matrix[floor][button];
}


int elev_get_last_floor(int last_floor) {
    // Gets last floor

    int floor_sensor_signal = elev_get_floor_sensor_signal();

    if ((floor_sensor_signal != last_floor)
	    && (floor_sensor_signal > -1)) {

				return floor_sensor_signal;
    }
    
    return last_floor;
}


int elev_set_direction(int direction, int next_floor, int last_floor) {

    // Start elevator in the right direction
    if (next_floor > last_floor) {
	direction = DIRN_UP;
    }
    else {
	direction = DIRN_DOWN;
    }

    elev_set_motor_direction(direction);

    return direction;
}


int elev_initialize_hardware(int direction) {
    // Initialize hardware
    
    if (!elev_init()) {
	printf("Unable to initialize elevator hardware!\n");
	return 1;
    }
    
    elev_set_motor_direction(direction);

    printf("Press STOP button to stop elevator and exit program.\n");
    return 0;
}


void elev_stop_button_handler(int current_floor) {
	//Stops motor
	elev_set_motor_direction(DIRN_STOP);

	//Turns all lights off since queue is cleared
	for (i = 0; i < N_FLOORS; ++i) {
        if (i != 0)
            elev_set_button_lamp(BUTTON_CALL_DOWN, i, 0);

        if (i != N_FLOORS - 1)
            elev_set_button_lamp(BUTTON_CALL_UP, i, 0);

        elev_set_button_lamp(BUTTON_COMMAND, i, 0);
    }
	
	//If elev currently on floor, door opens
	if (current_floor != -1) {
		elev_set_door_open(1);
	
	
}

