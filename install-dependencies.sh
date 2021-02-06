#!/bin/sh

ORIG_DIR=${PWD}

set -e

set -a
: "${BASE_DIR:=$(dirname "$(readlink -f "$0")")}"
: "${DEPENDENCIES_FILE:=$(realpath "${BASE_DIR}/.deps")}"
: "${DEPENDENCIES_DIR:=$(cat "${DEPENDENCIES_FILE}" 2>/dev/null || realpath "${BASE_DIR}/../deps-linux")}"
: "${REBUILD:="false"}"
set +a

if [ -f "${DEPENDENCIES_DIR}" ]; then
  echo "${DEPENDENCIES_FILE} contains a path to a non-directory ${DEPENDENCIES_DIR} - this must be the path to the dependencies folder"
  exit 2
fi
if [ -d "${DEPENDENCIES_DIR}" ]; then
  if [ "${REBUILD}" = "false" ]; then
    echo "Using existing ${DEPENDENCIES_DIR}"
    echo -n "${DEPENDENCIES_DIR}" > ${DEPENDENCIES_FILE}
    exit 0;
  fi
else
  echo -n "${DEPENDENCIES_DIR}" > ${DEPENDENCIES_FILE}
fi

on_exit()
{
    cd ${ORIG_DIR}
}
trap on_exit EXIT

mkdir -p ${DEPENDENCIES_DIR}
cd ${DEPENDENCIES_DIR}

if [ -d date ]; then
  cd date;
  git pull origin master;
  cd ..
else 
  git clone https://github.com/HowardHinnant/date.git date
fi

if [ -d uri ]; then
  cd uri;
  git pull origin master;
  cd ..
else
  git clone https://github.com/cpp-netlib/uri/ uri
fi
cd uri
  git submodule init
  git submodule update
  mkdir -p _build
    cd _build
      cmake ..
      make
    cd ..
  mkdir -p lib
  cp _build/src/libnetwork-uri.a lib
cd ..
