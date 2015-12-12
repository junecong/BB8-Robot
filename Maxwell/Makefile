# all:
# 	g++ -Wall -std=c++11 -c `pkg-config --libs libusb-1.0` Servo/motorControl.cpp
# 	g++ -Wall -std=c++11 -g `pkg-config --libs libusb-1.0` -c Communication/receive.c
# 	g++ motorControl.o receive.o 
# 	# g++ -Wall -g `pkg-config --libs libusb-1.0` Communication/receive.c -o Main/run

# clean: 
# 	rm *o
# 	rm Main/run


all:
	g++ -std=c++11 -c Servo/motorControl.cpp
	g++ -Wall -std=c++11 -c Communication/receive.c
	g++ `pkg-config --libs libusb-1.0` receive.o motorControl.o -o Main/runBB8
	rm *.o

clean: 
	rm *.o
	rm Main/runBB8