// Test code for motor control
#ifndef MOTORCONTROL_H
#define MOTORCONTROL_H
#include <iostream>
#include <string>
#include <cstring>
#include <libusb-1.0/libusb.h>
#include <unistd.h>
#include <stdlib.h>

using namespace std;

extern "C" int move(char data[], int speedVal);

// Sends direction control to motors
int startMotors (libusb_device_handle *handle, float speed);

// Moves 2 motors with respective speed
int motorOneDirection (libusb_device_handle *handle, float rSpeed, float lSpeed);

// Two-Wheel method
int motorTurn (libusb_device_handle *handle, float speed);

void pause_thread(int n);

int turn(libusb_device_handle *handle, int angle);

int drive(libusb_device_handle *handle, float distance, float percent);

static int normalizeAngle(int angle);

// Main function that runs the servos
int move (char *data, float dist_angle, float percentSpeed);

#endif