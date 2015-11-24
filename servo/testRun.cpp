// Test code for motor control

#include <iostream>
#include <string>
#include <cstring>
#include <libusb-1.0/libusb.h>
#include <unistd.h>
#include <stdlib.h>

using namespace std;

// Sends direction control to motors
int startMotors (libusb_device_handle *handle, float speed) {
	int r;
	// Assumed 4 servos
	for (int i = 0; i < 8; i+=2) {
			r = libusb_control_transfer( handle,
							0x40,	  //request type
							0x85,	  //request
							4*speed,  //speed/value
							i,		  //servo number
							NULL,
							0,
							5000);
			if (r < 0) {
				cout << "Error starting motor " << i << endl;
				return -1;
			}
	}
	return r;
}

// Two-Wheel method
int motorOneDirection (libusb_device_handle *handle, float speed) {
	int r;
	// Assumed 4 servos
	r = libusb_control_transfer( handle,
				0x40,	  //request type
				0x85,	  //request
				4*speed,  //speed/value
				0,		  //servo number
				NULL,
				0,
				5000);
	if (r < 0) {
		cout << "Error starting motor " << 0 << endl;
		return -1;
	}
	r = libusb_control_transfer( handle,
				0x40,	  //request type
				0x85,	  //request
				4*((3000-speed)),  //speed/value
				4,		  //servo number
				NULL,
				0,
				5000);
	if (r < 0) {
		cout << "Error starting motor " << 2 << endl;
		return -1;
	}

	return r;

}

// Omni-wheel method
// int motorOneDirection (libusb_device_handle *handle, float speed) {
// 	int r;
// 	// Assumed 4 servos
// 	r = libusb_control_transfer( handle,
// 				0x40,	  //request type
// 				0x85,	  //request
// 				4*speed,  //speed/value
// 				0,		  //servo number
// 				NULL,
// 				0,
// 				5000);
// 	if (r < 0) {
// 		cout << "Error starting motor " << 0 << endl;
// 		return -1;
// 	}
// 	r = libusb_control_transfer( handle,
// 				0x40,	  //request type
// 				0x85,	  //request
// 				4*(3000-speed),  //speed/value
// 				2,		  //servo number
// 				NULL,
// 				0,
// 				5000);
// 	if (r < 0) {
// 		cout << "Error starting motor " << 0 << endl;
// 		return -1;
// 	}


// 	r = libusb_control_transfer( handle,
// 				0x40,	  //request type
// 				0x85,	  //request
// 				4*(3000-speed),  //speed/value
// 				4,		  //servo number
// 				NULL,
// 				0,
// 				5000);
// 	if (r < 0) {
// 		cout << "Error starting motor " << 2 << endl;
// 		return -1;
// 	}
// 	r = libusb_control_transfer( handle,
// 				0x40,	  //request type
// 				0x85,	  //request
// 				4*(speed),  //speed/value
// 				6,		  //servo number
// 				NULL,
// 				0,
// 				5000);
// 	if (r < 0) {
// 		cout << "Error starting motor " << 0 << endl;
// 		return -1;
// 	}

// 	return r;

// }

int main(int argc, char *argv[]) {

	if (argc < 3) {
		cout << "Not enough arguments: set args as ./testRun ['forward'/'backward/stop'] [speedVal]" << endl;
	}

	char *direction = argv[1];

	int speedInput = atoi(argv[2]);

	if (speedInput < 0 || speedInput > 100) {
		cout << "Speed must be between 0-100" << endl;
		return -1;
	}

	// Value of 1000 is max in forward direction
	// Value of 1500 and 0 is neutral
	// Value of 2000 is max in backward direction
	float percentSpeed = 500 * (speedInput/100.0);

	libusb_context *ctx = NULL;
	libusb_device_handle *handle;

	// PID and VID of Maestro
	uint16_t pid = 0x008a;
	uint16_t vid = 0x1ffb;

	// error code
	int r;

	r = libusb_init(&ctx);

	if (r < 0 && r != LIBUSB_ERROR_NOT_FOUND) {
		cout << "Error: detach kernel driver failed" << endl;
		return -1;
	}

	handle = libusb_open_device_with_vid_pid(ctx, vid, pid);

	if (!handle) {
		cout << "Error: handle incorrect" << endl;
		return -1;
	}

	r = libusb_claim_interface(handle, 0);

	int speed = 0;
	if (strcmp(direction, "forward") == 0) {
		speed = 1500 - percentSpeed;
		// startMotors(handle, speed);
		motorOneDirection(handle, speed);
	} else if (strcmp(direction, "backward") == 0) {
		speed = 1500 + percentSpeed;
		motorOneDirection(handle, speed);
		// startMotors(handle, speed);
	} else if (strcmp(direction, "stop") == 0) {
		startMotors(handle, speed);
	} else {
		cout << "invalid string" << endl;
	}

	r = libusb_release_interface(handle, 0);
	libusb_close(handle);
	libusb_exit(ctx);
	return 0;
}