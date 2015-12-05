# BB8-Robot
There are two designated boards for this project:
-Maxwell is the board on the robot that will control the main movement of the robot while receiving motor commands from Pascal
-Pascal is the board that will handle the vision control and send commands to Maxwell

There are two main directories: Maxwell and Pascal

In Maxwell, there is Communication, Servo, Vision, and Main subdirectories. Communication holds the files for packet transfer between boards as well as a wifi setup script. Servo holds the motor code and USB control. Vision should be in Pascal, not Maxwell.

To compile, type 'make' in the upper directory and the executable will appear in subdirectory Main.

In Pascal, we have Communication, which holds the wifi setup script and the packet transfer code.

To run the communication executable between boards, run the exe on Maxwell first, and then run it on Pascal. The messages you can send are:
-forward
-backward
-right
-left
-stop
-exit (exits the connection)
The default speed is set at 50%

To test out motor control without the use of the Pascal board, there should be a test subdirectory on the board already. Inside should be a testRun executable. To compile, type 'gcc -o testRun testRun.c' and run the executable using './testRun [forward/backward/right/left/stop] [percentSpeed]'

To test out the communication without the use of Pascal, change the default ip inside send.c to the current ip of Maxwell and compile and run the exe. This is untested.
