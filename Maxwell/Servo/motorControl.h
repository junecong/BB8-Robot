// Test code for motor control

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

// Main function that runs the servos
int move (char data[], int speedVal);
