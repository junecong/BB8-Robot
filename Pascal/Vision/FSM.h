#ifndef FSM
#define FSM
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>

#define MAXQUEUESIZE 32
#define MAXSIZE 5
#define MAX_OBJ_DIST_BW_FRAMES 10
#define ACTUAL_DIAMETER_IN_CM 16.f

using namespace std;

int test();
void MaxwellStatechartTest(string direction);
float orient (float ogBBx, float ogBBy, float newBBx, float newBBy, float destx, float desty);
void MaxwellStatechart(
	float driveDistance,
	bool isOffscreen,
	float bbx,
	float bby,
	float bbR,
	float destx,
	float desty,
	float destR,
	string direction, 
	string output[]);
#endif