#!/usr/bin/env sh

set -e

cd travis-build

if [ "$TRAVIS_OS_NAME" = "linux" ]; then
    make -j$(nproc)
elif [ "$TRAVIS_OS_NAME" = "osx" ]; then
    set -o pipefail && cmake --build . --config "$CONFIGURATION" -- ARCHS=x86_64 ONLY_ACTIVE_ARCH=NO | xcpretty
fi
