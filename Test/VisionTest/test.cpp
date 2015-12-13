// State Machine (untested)
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <math.h>       /* floor */
#include "FSM.h"
// #include <numeric>
// #include <cmath>

using namespace std;

typedef enum {
	MAXWELL_IDLE = 0,
	MAXWELL_ORIENT,
	MAXWELL_TURN,
	MAXWELL_DRIVE,
	MAXWELL_OFFSCREEN,
	MAXWELL_DONE
} robotState_t;

typedef enum {
	ORIENT_IDLE = 6,
	ORIENT_INITIAL_FORWARD,
	ORIENT_AWAITING,
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


float orient (float ogBBx, float ogBBy, float newBBx, float newBBy, float destx, float desty){
	float angleRad;
	float angleDegree;
	float turn;

	float oldNewVector[] = {newBBx-ogBBx , -(newBBy-ogBBy)};
	float oldDestVector[] = {destx-ogBBx, -(desty-ogBBy)};
	float newDestVector[] = {destx-newBBx, -(desty-newBBy)};

	cout << "oldNewVector: " << oldNewVector[0] <<  ", " << oldNewVector[1] << endl;
	cout << "oldDestVector: " << newDestVector[0] <<  ", " << newDestVector[1] << endl;
	cout << "newDestVector: " << newDestVector[0] << ", " << newDestVector[1] << endl;

	float num = (oldNewVector[0] * newDestVector[0]) + (oldNewVector[1] * newDestVector[1]);
	float denom = sqrt(pow(oldNewVector[0],2) + pow(oldNewVector[1],2)) * sqrt(pow(newDestVector[0],2) + pow(newDestVector[1],2));

	angleRad = acos ( num / denom );
	angleDegree = (angleRad *180)/M_PI ;

	turn = (oldNewVector[0]*oldDestVector[1]) - (oldNewVector[1]*oldDestVector[0]);
	cout << "Turn: " << turn << endl;
	if (turn > 0) {
		angleDegree = -angleDegree;
	} 

	cout << "Angle to Turn: " << angleDegree << endl;
	cout << " " << endl;
	return angleDegree;
}


void MaxwellStatechart(float driveDistance, 
	bool offscreen, 
	// bool start,
	float bbx,
	float bby,
	float bbR,
	float destx,
	float desty,
	float destR,
	string direction, string output[]){

	// local state
	static robotState_t robotState = MAXWELL_IDLE;
	static subState_t subState = ORIENT_IDLE;
	// static bool forward = true; //TRUE means driving forwards

	static float ogBBx;
	static float ogBBy;
	static float ogBBRad;
	static float newBBx;
	static float newBBy;
	static float newBBRad;
	static float dest_x_var;
	static float dest_y_var;
	static float dest_rad_var;
	static float degreeToTurn;
	static string speed = ".85";


	switch(robotState){
		case MAXWELL_IDLE:
			// if (start) {
				robotState = MAXWELL_ORIENT;
			// }

		case MAXWELL_ORIENT:
			if (offscreen) {
				//TODO: Improve this
				robotState = MAXWELL_OFFSCREEN; 
			}
			switch(subState){
				case ORIENT_IDLE:
					ogBBx = bbx;
					ogBBy = bby;
					ogBBRad = bbR;
					dest_x_var = desty;
					dest_y_var = destx;
					dest_rad_var = destR;
					subState = ORIENT_INITIAL_FORWARD;

				case ORIENT_INITIAL_FORWARD:
					output[0] = "drive";
					output[1] = "25";
					output[2] = speed;
					while (direction != "stablized");
					subState = ORIENT_AWAITING;

				case ORIENT_AWAITING:
					newBBx = bbx;
					newBBy = bby;
					newBBRad = bbR;
					subState = ORIENT_FINISHED;

				case ORIENT_FINISHED:	
					degreeToTurn = orient (ogBBx, ogBBy, newBBx, newBBy, dest_x_var, dest_y_var); 
					robotState = MAXWELL_DRIVE;
					subState = ORIENT_IDLE;
				break;
			}
		case MAXWELL_TURN:
			// TODO: Ask minh if turning == stabilized
			output[0] = "turn";
			output[1] = to_string(degreeToTurn);
			output[2] = speed;
			// output = {"turn", degreeToTurn, 0};
			while (direction != "stablized"); //wait for it to stop moving
			robotState = MAXWELL_DRIVE;

		case MAXWELL_DRIVE:
			// forward = true; //you're currently driving forwards
			output[0] = "drive";
			output[1] = to_string(driveDistance);
			output[2] = speed;
			while (direction != "stablized"); //wait for it to stop moving
			if (offscreen) {
				robotState = MAXWELL_OFFSCREEN;
			}
			if (driveDistance <= 10) {
				robotState = MAXWELL_DONE;
			} else {
				robotState = MAXWELL_ORIENT;
			}

		case MAXWELL_OFFSCREEN:
			output[0] = "drive";
			output[1] = "-25";
			output[2] = speed;
			while (direction != "stablized");
			while (direction != "stablized");
			robotState = MAXWELL_ORIENT;

		case MAXWELL_DONE:
			cout << "Destination Reached" << endl;

		break;
	}
}



int main(int argc, char **argv) {
	// // return orientation(10.2, 12.5, 16.2);
	// orient (50, 100, 100, 50, 10, 10); //negative

	// // orient (10, 100, 10, 50, 10, 10); //0

	// orient (0, 100, 0, 50, 10, 10); //positive

	// // orient (0, 10, 0, 0, 10, 10); //positive

	// orient (0, 3, 2, 0, 2, 1); //positive

	orient (0, 3, 2, 0, 2, 1); //positive

	orient (0, 3, 2, 1, 2, 0); //negative

	return 1;
}

