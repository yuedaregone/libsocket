#!/bin/sh
cd ./script

../tools/bin/pi/genie --platform=ARM --gcc=linux-arm-gcc gmake

cd ../build/projects/gmake-linux-arm-gcc
make

