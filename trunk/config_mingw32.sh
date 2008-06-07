#!/bin/sh

./configure --prefix=/usr/i586-mingw32msvc --host=i586-mingw32msvc --target=i586-mingw32msvc --with-msw --enable-mingw32 --disable-allegrotest CXXFLAGS="-g -O2 -ffast-math -march=i686"
