#!/bin/sh

echo "Building paho library"
mkdir -p $1/paho.mqtt.c/build
cd $1/paho.mqtt.c/build
cmake -DCMAKE_BUILD_TYPE=Debug -DPAHO_ENABLE_TESTING=FALSE -DPAHO_ENABLE_CPACK=FALSE ..

make -j$(nproc)
