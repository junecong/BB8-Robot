#ifndef FSM
#define FSM
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>

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