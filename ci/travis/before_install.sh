#!/usr/bin/env sh

set -e

if [ "$TRAVIS_OS_NAME" = "linux" ]; then
    # Nothing to do here
    :
elif [ "$TRAVIS_OS_NAME" = "osx" ]; then
    brew update
fi
