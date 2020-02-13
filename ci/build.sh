#!/bin/bash
# Usage: build.sh [SETUP]

./configure $1          || exit 1
./makew install         || exit 1

tar -cJf aplus-release-$1.tar.xz aplus.img

