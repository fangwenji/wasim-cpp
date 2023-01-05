#!/bin/bash

GLOG_VERSION=v0.6.0

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
DEPS=$DIR/../deps

mkdir -p $DEPS

if [ ! -d "$DEPS/glog" ]; then
    cd $DEPS
    git clone https://github.com/google/glog.git glog
    cd glog
    git checkout -f $GLOG_VERSION
    cmake -S . -B build -G "Unix Makefiles" -DBUILD_SHARED_LIBS=OFF
    cmake --build build
else
    echo "$DEPS/glog already exists. If you want to rebuild, please remove $DEPS/glog and start over."
fi


