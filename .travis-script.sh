#!/bin/sh
# The script to be called from .travis.yml "script:"

set -e

if test "x$TRAVIS_OS_NAME" == "xosx"
then
    # ls -l /usr/local/bin
    # ls -l /usr/local/sbin
    # ls -lR /usr/local/opt/{coreutils,e2fsprogs}/{bin,lib,sbin}
    export PKG_CONFIG_PATH="/usr/local/opt/e2fsprogs/lib/pkgconfig"
    for d in /usr/local/opt/e2fsprogs/{sbin,bin}
    do
       if test -d "$d"
       then
           ls -l "$d"
           PATH="$d:$PATH"
       fi
    done
    export PATH
    echo "e2t osx PATH=$PATH"
    echo "e2t osx PKG_CONFIG_PATH=$PKG_CONFIG_PATH"
    # brew info
    # brew list
fi

MAKE=make

set -x

mkdir _b && cd _b

../configure --prefix=$PWD/../_i

# cat config.h

${MAKE} all

if ${MAKE} check; then
    :
else
    s="$?"
    cat "test-suite.log"
    exit "$s"
fi

${MAKE} install
${MAKE} installcheck

ls -lR

cd ..

ls -lR _i

cd _b
${MAKE} distcheck
