#!/bin/bash

VEXPPARSER_VERSION=5493131f5c8f9aa95e0ff2768c33a91cae35f9f1

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
DEPS=$DIR/../deps

mkdir -p $DEPS

if [ ! -d "$DEPS/vexpparser" ]; then
    cd $DEPS
    git clone https://github.com/WBChe/vexpparser.git
    cd vexpparser
    git checkout -f $VEXPPARSER_VERSION
    mkdir -p build 
    cd build
    cmake .. 
    cmake --build .
else
    echo "$DEPS/vexpparser already exists. If you want to rebuild, please remove it manually."
fi

if [ -f $DEPS/vexpparser/build/src/libvexpparser.a ] ; then \
    echo "It appears vexpparser was successfully built in $DEPS/vexpparser/build/src."
    echo "You may now build wasim with: ./configure.sh && cd build && make"
else
    echo "Building vexpparser failed."
    echo "You might be missing some dependencies."
    echo "Please see their github page for installation instructions: https://github.com/zhanghongce/vexpparser"
    exit 1
fi

