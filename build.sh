#!/bin/bash

set -e
set -x

BASEDIR=$(dirname "$0")
pushd "$BASEDIR"

rm -rf build

./vcpkg/bootstrap-vcpkg.sh
./vcpkg/vcpkg install

cmake -B build
cmake --build build
