#!/bin/bash

VEXPPARSER_VERSION=17ca3ea74402df92701c55d3eef9b18726a1fec5

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
DEPS=$DIR/../deps

mkdir -p $DEPS

if [ ! -d "$DEPS/vexpparser" ]; then
    cd $DEPS
    git clone --depth 1 https://github.com/zhanghongce/vexpparser.git
    cd vexpparser
    git checkout -f $BTOR2TOOLS_VERSION
    mkdir -p build 
    cd build
    cmake .. 
    cmake --build .
else
    echo "$DEPS/vexpparser already exists. If you want to rebuild, please remove it manually."
fi

if [ -f $DEPS/vexpparser/build/src/libvexpparser.a ] ; then \
    echo "It appears btor2tools was successfully built in $DEPS/vexpparser/build/src."
    echo "You may now build wasim with: ./configure.sh && cd build && make"
else
    echo "Building vexpparser failed."
    echo "You might be missing some dependencies."
    echo "Please see their github page for installation instructions: https://github.com/zhanghongce/vexpparser"
    exit 1
fi

