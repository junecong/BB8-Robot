#! /bin/sh

export PKG_CONFIG_PATH="/usr/local/lib/pkgconfig"

echo -n "g++ compiles robotControl.cpp.."
g++ -Wall -g -std=c++11 `pkg-config --libs libusb-1.0` robotControl.cpp -o robotControl
echo "g++ compiles motorControl.cpp.."
g++ -Wall -g -std=c++11 `pkg-config --libs libusb-1.0` motorControl.cpp -o motorControl
echo "Done!"
