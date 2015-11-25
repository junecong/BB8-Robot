#! /bin/sh

export PKG_CONFIG_PATH="/usr/local/lib/pkgconfig"

echo -n "g++ compiles set_target.cpp.."
g++ -Wall -g `pkg-config --libs libusb-1.0` testRun.cpp -o testRun
echo "Done!"
