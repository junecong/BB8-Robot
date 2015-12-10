// State Machine (untested)


typedef enum {
	MAXWELL_DRIVE,
	MAXWELL_REVERSE,
	MAXWELL_ORIENT,
	MAXWELL_IDLE,
	MAXWELL_OFFSCREEN,
} robotState_t;

typedef enum {
	ORIENT_INITIAL_FORWARD,
	ORIENT_TURN_RAND
	ORIENT_TURN_TO_TARGET,
	ORIENT_IDLE
} subState_t;

void MaxwellStatechart(
	float orientAngle,
	float driveDistance,
	bool offscreen,

	){

	robotState_t robotState = MAXWELL_IDLE;
	subState_t subsState = ORIENT_IDLE;

	switch(robotState){
		case MAXWELL_IDLE:

		case MAXWELL_ORIENT:

			switch(subState){
				case ORIENT_IDLE:

				case ORIENT_INITIAL_FORWARD:

				case ORIENT_TURN_RAND:

				case ORIENT_TURN_TO_TARGET:

				break;
			}
		case MAXWELL_DRIVE:
			
		case MAXWELL_OFFSCREEN:
			
		break;
	}




}
