#!/bin/bash

: ${CC:=/opt/llvm-11.1.0/bin/clang}
: ${CXX:=/opt/llvm-11.1.0/bin/clang++}
: ${CXXFLAGS:="-std=c++20 -stdlib=libc++"}
: ${CMAKE_BUILD_TYPE:="Release"}

export CC CXX CXXFLAGS

ROOT_DIR=$(dirname $0)
mkdir -p ${ROOT_DIR}/build
pushd ${ROOT_DIR}/build
cmake .. -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
make
popd # ${ROOT_DIR}/build
