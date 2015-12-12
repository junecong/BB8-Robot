// Test code for robot control
#include "motorControl.cpp"
#include <thread>
#include <chrono>
#include <math.h>
#include <iostream>
#include <string>
#include <cstring>
#include <libusb-1.0/libusb.h>
#include <unistd.h>
#include <stdlib.h>

#ifndef ROBOTCONTROL_H
#define ROBOTCONTROL_H

using namespace std;

extern "C" int move(char data[], int speedVal);

// Sends direction control to motors
int startMotors (libusb_device_handle *handle, float speed);

// Moves 2 motors with respective speed
int motorOneDirection (libusb_device_handle *handle, float rSpeed, float lSpeed);

// Two-Wheel method
int motorTurn (libusb_device_handle *handle, float speed);

// Angles output are in range [-180, 180]
// 
// Arguments:
// - angle: angle to turn in degrees
static int normalizeAngle(int angle);

// Turn a certain amount of degrees.
// Positive angles turn the robot right
// while negative angles turn left.
// 
// Arguments:
// - handle: used for communication with
// USB devices
// - angle: angle to turn in degrees
int turn(libusb_device_handle *handle, int angle);

// Drive a certain amount of centimeters.
// Positive distances drive forward
// while negative distances drive back.
// 
// Arguments:
// - handle: used for communication with
// USB devices
// - distance: distance to travel in cm
int drive(libusb_device_handle *handle, float distance);

#endif




