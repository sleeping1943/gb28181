#!/usr/bin/sh
xmake clean
xmake config -m release
xmake project -k make
xmake project -k cmake
xmake -j6