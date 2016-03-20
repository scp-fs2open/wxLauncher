#!/usr/bin/env sh

set -e

mkdir -p travis-build
cd travis-build

if [ "$TRAVIS_OS_NAME" = "linux" ]; then
    cmake -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=$CONFIGURATION ..
elif [ "$TRAVIS_OS_NAME" = "osx" ]; then
    cmake -G "Xcode" -DUSING_SDL_FRAMEWORK=OFF ..
fi
