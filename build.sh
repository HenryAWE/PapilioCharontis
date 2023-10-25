#! /usr/bin/env bash

mkdir -p build
cd build

cmake -GNinja "$@" ..
cmake --build .
