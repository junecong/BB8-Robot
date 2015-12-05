all:
	g++ -c Servo/motorControl.cpp
	g++ -g `pkg-config --libs libusb-1.0` -c Communication/receive.c
	cc motorControl.o receive.o 
	# g++ -Wall -g `pkg-config --libs libusb-1.0` Communication/receive.c -o Main/run

clean: 
	rm *o
	rm Main/run
