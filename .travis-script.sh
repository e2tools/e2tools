#!/bin/sh
# The script to be called from .travis.yml "script:"

set -e
set -x

if test "x$TRAVIS_OS_NAME" == "xosx"
then
    export PKG_CONFIG_PATH="/usr/local/opt/e2fsprogs/lib/pkgconfig"
    export PATH="/usr/local/opt/e2fsprogs/bin:$PATH"
fi

MAKE=make

mkdir _b && cd _b

../configure --prefix=$PWD/../_i

# cat config.h

${MAKE} all

${MAKE} check

if test -f "test-suite.log"
then
    cat "test-suite.log"
fi

${MAKE} install
${MAKE} installcheck

ls -lR

cd ..

ls -lR _i

cd _b
${MAKE} distcheck
