#ifndef FSM_H
#define FSM_H
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>

using namespace std;

int test();
void MaxwellStatechartTest(string direction);
void MaxwellStatechart (
	float orientAngle,
	float driveDistance,
	bool offscreen,
	float bbx,
	float bby,
	float bbR,
	float destx,
	float desty,
	float destR,
	string direction1
	);
#endif