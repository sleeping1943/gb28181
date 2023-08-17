#!/usr/bin/sh
xmake clean
xmake config -m debug
xmake project -k make
xmake project -k cmake
xmake -j6