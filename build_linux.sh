#!/bin/sh
cd ./script

../tools/bin/linux/genie --gcc=linux-gcc gmake

cd ../build/projects/gmake-linux
make

