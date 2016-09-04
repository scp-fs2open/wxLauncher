#!/usr/bin/env sh

set -e

if [ "$TRAVIS_OS_NAME" = "linux" ]; then
    sudo apt-get install -y ninja-build libsdl2-dev  libwxgtk$wxWidgets_ver-dev

    mkdir -p $HOME/cmake/

    wget -O /tmp/cmake.tar.gz --no-check-certificate http://www.cmake.org/files/v3.4/cmake-3.4.3-Linux-x86_64.tar.gz
    tar -xzf /tmp/cmake.tar.gz -C $HOME/cmake/ --strip-components=1
elif [ "$TRAVIS_OS_NAME" = "osx" ]; then
    gem install xcpretty xcpretty-travis-formatter

    brew install wxmac
    brew install openal-soft
    brew install sdl2
fi

pip install --user markdown future
