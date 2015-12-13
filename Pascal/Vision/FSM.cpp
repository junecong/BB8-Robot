// State Machine (untested)
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <math.h>       /* floor */
#include "FSM.h"

#include <time.h>
// #include <cmath>

using namespace std;

typedef enum {
	MAXWELL_IDLE = 0,
	MAXWELL_ORIENT,
	MAXWELL_WAIT_1,
	MAXWELL_TURN,
	MAXWELL_WAIT_2,
	MAXWELL_DRIVE,
	MAXWELL_OFFSCREEN,
	MAXWELL_TURN_180,
	MAXWELL_DONE
} robotState_t;

typedef enum {
	ORIENT_IDLE = 9,
	ORIENT_INITIAL_FORWARD,
	ORIENT_WAIT,
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
	float num = (oldNewVector[0] * newDestVector[0]) + (oldNewVector[1] * newDestVector[1]);
	float denom = sqrt(pow(oldNewVector[0],2) + pow(oldNewVector[1],2)) * sqrt(pow(newDestVector[0],2) + pow(newDestVector[1],2));

	angleRad = acos ( num / denom );
	angleDegree = (angleRad *180)/M_PI ;
	turn = (oldNewVector[0]*oldDestVector[1]) - (oldNewVector[1]*oldDestVector[0]);
	if (turn > 0) {
		angleDegree = -angleDegree;
	} 
	cout << "Orient Function input: " << ogBBx << " , " << ogBBy << " , " << newBBx << " , "  << newBBy << " , " << destx << " , " << desty << endl;
	cout << "Orient Function Called and result: " << angleDegree << endl;
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
	static string speed = ".7";
	static string degreeToTurnStr;
	static string driveDistanceStr;

	switch(robotState){
		case MAXWELL_IDLE:
			if (direction == "Stationary") {
				robotState = MAXWELL_ORIENT;
			}
			break;
		case MAXWELL_ORIENT:
			cout << "MAXWELL_ORIENT" << endl;
			if (offscreen) {
				//TODO: Improve this
				robotState = MAXWELL_OFFSCREEN; 
			}
			switch(subState){
				case ORIENT_IDLE:
					cout << "ORIENT_IDLE" << endl;
					ogBBx = bbx;
					ogBBy = bby;
					ogBBRad = bbR;
					dest_x_var = desty;
					dest_y_var = destx;
					dest_rad_var = destR;
					subState = ORIENT_INITIAL_FORWARD;
					break;

				case ORIENT_INITIAL_FORWARD:
					cout << "ORIENT_INITIAL_FORWARD" << endl;
					output[0] = "drive";
					output[1] = "5";
					output[2] = speed;
					
					subState = ORIENT_WAIT;
					break;
					
				case ORIENT_WAIT:
					cout << "ORIENT_WAIT" << endl;
					if (direction == "Stationary") {
						subState = ORIENT_FINISHED;
					}
					break;

				case ORIENT_FINISHED:	
					cout << "ORIENT_FINISHED" << endl;
					newBBx = bbx;
					newBBy = bby;
					newBBRad = bbR;
					degreeToTurn = orient (ogBBx, ogBBy, newBBx, newBBy, dest_x_var, dest_y_var); 
					robotState = MAXWELL_WAIT_1;
					subState = ORIENT_IDLE;
					break;
			}
			break;

		case MAXWELL_WAIT_1:
			cout << "MAXWELL_WAIT_1" << endl;
			if (direction == "Stationary") {
				robotState = MAXWELL_TURN;
			}
			break;

		case MAXWELL_TURN:
			cout << "MAXWELL_TURN" << endl;
			degreeToTurnStr = to_string(degreeToTurn);
			output[0] = "turn";
			output[1] = degreeToTurnStr;
			output[2] = speed;
			robotState = MAXWELL_WAIT_2;
			cout << "degree turning in FSM: " << degreeToTurnStr << ", out0: " << output[0] << ", out1: " << output[1] << ", out2: " << output[2] << endl;
			cout << " " << endl;
			break;

		case MAXWELL_WAIT_2:
			cout << "MAXWELL_WAIT_2" << endl;
			if (direction == "Stationary") {
				robotState = MAXWELL_DRIVE;
			}
			break;

		case MAXWELL_DRIVE:
			cout << "MAXWELL_DRIVE" << endl;
			driveDistanceStr = to_string(driveDistance);
			output[0] = "drive";
			output[1] = driveDistanceStr;
			output[2] = speed;
			cout << "Dist to target: " << driveDistance << endl;
			
			if (driveDistance <= 10) {
				robotState = MAXWELL_DONE;
				output[0] = "stop";
			} else if (offscreen) {
				robotState = MAXWELL_OFFSCREEN;
			} else {
				robotState = MAXWELL_IDLE;
			}

			break;

		case MAXWELL_OFFSCREEN:
			cout << "MAXWELL_OFFSCREEN" << endl;
			output[0] = "stop";
			// robotState = MAXWELL_ORIENT;
			if (!offscreen) {
				robotState = MAXWELL_IDLE;
			} else {
				robotState = MAXWELL_TURN_180;
			}
			break;

		case MAXWELL_TURN_180:
			cout << "MAXWELL_TURN_180" << endl;
			output[0] = "turn";
			output[1] = "180";
			output[2] = speed;
			robotState = MAXWELL_IDLE;

			break;

		case MAXWELL_DONE:
			cout << "MAXWELL_DONE" << endl;
			cout << "Dist to target: " << driveDistance << endl;
			output[0] = "exit";

		break;
	}
}
