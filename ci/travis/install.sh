#!/usr/bin/env sh

set -e

if [ "$TRAVIS_OS_NAME" = "linux" ]; then
    sudo apt-get install -y ninja-build libsdl2-dev
elif [ "$TRAVIS_OS_NAME" = "osx" ]; then
    gem install xcpretty xcpretty-travis-formatter

    brew install wxmac
    brew install openal-soft
    brew install sdl2
fi

pip install --user markdown future
