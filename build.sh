#!/bin/sh

mkdir build
cd build
cmake -GNinja -DBUILD_COVERAGE=ON -DBUILD_DEPENDENCIES=OFF -DBUILD_DOCUMENTATION=ON ..
cmake --build .
