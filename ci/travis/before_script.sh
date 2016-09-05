#!/usr/bin/env sh

set -e

mkdir -p travis-build
cd travis-build

BUILD_TYPE="-DCMAKE_BUILD_TYPE=$CONFIGURATION"

if [ "$TRAVIS_OS_NAME" = "linux" ]; then
    $HOME/cmake/bin/cmake -G "Unix Makefiles" $BUILD_TYPE ..
elif [ "$TRAVIS_OS_NAME" = "osx" ]; then
    cmake -G "Xcode" -DUSING_SDL_FRAMEWORK=OFF $BUILD_TYPE ..
fi
