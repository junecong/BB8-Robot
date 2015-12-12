// State Machine (untested)
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include "FSM.h"

using namespace std;

typedef enum {
	MAXWELL_DRIVE,
	MAXWELL_REVERSE,
	MAXWELL_ORIENT,
	MAXWELL_IDLE,
	MAXWELL_OFFSCREEN,
} robotState_t;

typedef enum {
	ORIENT_INITIAL_FORWARD,
	ORIENT_TURN_RAND,
	ORIENT_TURN_TO_TARGET,
	ORIENT_IDLE
} subState_t;


void MaxwellStatechartTest(string direction) {
	if (direction == "North") {
		cout << "N" << endl;
	} else if (direction == "South") {
		cout << "S" << endl;
	} else if (direction == "West") {
		cout << "W" << endl;
	} else if (direction == "East") {
		cout << "E" << endl;
	}
}

void MaxwellStatechart(float orientAngle, float driveDistance, bool offscreen) {

	robotState_t robotState = MAXWELL_IDLE;
	subState_t subState = ORIENT_IDLE;

	switch(robotState){
		case MAXWELL_IDLE:

		case MAXWELL_ORIENT:

			switch(subState){
				case ORIENT_IDLE:

				case ORIENT_INITIAL_FORWARD:

				case ORIENT_TURN_RAND:

				case ORIENT_TURN_TO_TARGET:

				break;
			}
		case MAXWELL_DRIVE:
			
		case MAXWELL_OFFSCREEN:
			
		break;
	}
}
