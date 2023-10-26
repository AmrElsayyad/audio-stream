#!/bin/bash

set -e
set -x

BASEDIR=$(dirname "$0")
pushd "$BASEDIR"

if [ -d "build" ]
then
    rm -rf build
fi

if [ ! -e "./vcpkg/vcpkg" ]
then
    ./vcpkg/bootstrap-vcpkg.sh
fi
./vcpkg/vcpkg install

cmake -B build
cmake --build build
