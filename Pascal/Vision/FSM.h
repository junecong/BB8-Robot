#ifndef FSM_H
#define FSM_H
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <vector>

using namespace std;

float orient (float ogBBx, float ogBBy, float newBBx, float newBBy, float destx, float desty);
vector<string> MaxwellStatechart(
	float driveDistance,
	bool isOffscreen,
	float bbx,
	float bby,
	float bbR,
	float destx,
	float desty,
	float destR,
	string direction);
#endif