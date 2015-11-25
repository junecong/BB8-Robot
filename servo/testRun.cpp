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

// moves 2 motors with respective speed
int motorOneDirection (libusb_device_handle *handle, float rSpeed, float lSpeed) {
	int r;
	// Assumed 4 servos
	r = libusb_control_transfer( handle,
				0x40,	  //request type
				0x85,	  //request
				4*rSpeed,  //speed/value
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
				4*lSpeed,  //speed/value
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

// Two-Wheel method
int motorTurn (libusb_device_handle *handle, float speed) {
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
				4*speed,  //speed/value
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

	// Value of 1040 is max in forward direction
	// Value of 1490 and 0 is neutral
	// Value of 1940 is max in backward direction
	float percentSpeed = 450 * (speedInput/100.0);

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

	// lSpeed has a bias of 5
	int rSpeed = 0;
	int lSpeed = 0;
	int speed = 0;
	if (strcmp(direction, "forward") == 0) {
		rSpeed = 1490 - percentSpeed;
		lSpeed = 1495 + percentSpeed;
		motorOneDirection(handle, rSpeed, lSpeed);
	} else if (strcmp(direction, "backward") == 0) {
		rSpeed = 1490 + percentSpeed;
		lSpeed = 1495 - percentSpeed;
		motorOneDirection(handle, rSpeed, lSpeed);
	} else if (strcmp(direction, "right") == 0) {
		speed = 1490 + percentSpeed;
		motorTurn(handle, speed);
	} else if (strcmp(direction, "left") == 0) {
		speed = 1490 - percentSpeed;
		motorTurn(handle, speed);
	} else if (strcmp(direction, "stop") == 0) {
		startMotors(handle, 0);
	} else {
		cout << "invalid string" << endl;
	}

	r = libusb_release_interface(handle, 0);
	libusb_close(handle);
	libusb_exit(ctx);
	return 0;
}