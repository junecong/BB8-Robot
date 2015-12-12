// State Machine (untested)
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <math.h>       /* floor */
#include "FSM.h"

using namespace std;

typedef enum {
	MAXWELL_IDLE = 0,
	MAXWELL_ORIENT,
	MAXWELL_DRIVE,
	MAXWELL_REVERSE,
	MAXWELL_OFFSCREEN,
	MAXWELL_RIGHT,
	MAXWELL_LEFT,
	MAXWELL_DONE
} robotState_t;

typedef enum {
	ORIENT_IDLE = 8,
	ORIENT_INITIAL_FORWARD,
	// ORIENT_INITIAL_BACKWARD,
	ORIENT_INITIAL_RIGHT,
	// ORIENT_INITIAL_LEFT
	ORIENT_FINISHED
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


void MaxwellStatechart(float orientAngle, float driveDistance, bool offscreen, bool start) {

	// local state
	// static bool robotstarted = false;
	static robotState_t robotState = MAXWELL_IDLE;
	static subState_t subState = ORIENT_IDLE;
	static bool direction = true; //TRUE means driving forwards


	switch(robotState){
		case MAXWELL_IDLE:
			if (start) {
				// robotstarted = true;
				robotState = MAXWELL_ORIENT;
			}

		case MAXWELL_ORIENT:
			if (offscreen) {
				//will be implemetned later
				// robotState = MAXWELL_OFFSCREEN_START; 
			}
			switch(subState){
				case ORIENT_IDLE:
					subState = ORIENT_INITIAL_FORWARD;
				case ORIENT_INITIAL_FORWARD:
					// drive(25);
					subState = ORIENT_INITIAL_RIGHT;
				// case ORIENT_INITIAL_BACKWARD:

				case ORIENT_INITIAL_RIGHT:
					// turn(90);
					// drive(25);
					subState = ORIENT_FINISHED;
				// case ORIENT_INITIAL_LEFT:
				case ORIENT_FINISHED:	
					robotState = MAXWELL_DRIVE;
					subState = ORIENT_IDLE;

				break;
			}
		case MAXWELL_DRIVE:
			direction = true; //you're currently driving forwards
			// TODO: IDK HOW TO CALL THIS, but call it to drive "driveDistance"
			// drive(driveDistance);
			if (offscreen) {
				robotState = MAXWELL_OFFSCREEN;
			}
			robotState = MAXWELL_ORIENT;

		case MAXWELL_REVERSE:
			direction = false; //you're currently driving backwards
			// TODO: IDK HOW TO CALL THIS, but call it to drive "driveDistance"
			// TODO: MAKE DRIVE WORK backwards
			// driveBackwards(driveDistance);
			if (offscreen) {
				robotState = MAXWELL_OFFSCREEN;
			}
			robotState = MAXWELL_ORIENT;
			
		case MAXWELL_OFFSCREEN:
			if (direction == true) {
				// driveBackwards(25);
			} else {
				// drive(25);
			}
			robotState = MAXWELL_ORIENT;

		case MAXWELL_LEFT:

		case MAXWELL_RIGHT:

		case MAXWELL_DONE:
			
		break;
	}
}
