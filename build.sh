#!/bin/bash

ROOT_DIR=$(dirname $0)
mkdir -p ${ROOT_DIR}/build
pushd ${ROOT_DIR}/build
cmake .. -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release
make
popd # ${ROOT_DIR}/build
