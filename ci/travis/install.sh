#!/usr/bin/env sh

set -e

LINUX_CMAKE_BINS=http://www.cmake.org/files/v3.4/cmake-3.4.3-Linux-x86_64.tar.gz
CMAKE_BINS_TMP=/tmp/cmake.tar.gz

if [ "$TRAVIS_OS_NAME" = "linux" ]
then
    sudo apt-get install -y libwxgtk$wxWidgets_ver-dev

    if [ ! -x $HOME/cmake/bin/cmake ]
    then
	    mkdir -p $HOME/cmake/
	    wget -O $CMAKE_BINS_TMP --no-check-certificate $LINUX_CMAKE_BINS
	    tar -xzf $CMAKE_BINS_TMP -C $HOME/cmake/ --strip-components=1
    fi
elif [ "$TRAVIS_OS_NAME" = "osx" ]
then
    gem install xcpretty xcpretty-travis-formatter

    brew install wxmac
    brew install openal-soft
    brew install sdl2
fi

pip install --user markdown future
