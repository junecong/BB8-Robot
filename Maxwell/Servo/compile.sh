#! /bin/sh

export PKG_CONFIG_PATH="/usr/local/lib/pkgconfig"

echo -n "g++ compiles set_target.cpp.."
g++ -Wall -g -std=c++11 `pkg-config --libs libusb-1.0` robotControl.cpp -o robotControl
echo "Done!"
