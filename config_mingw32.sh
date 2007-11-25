#!/bin/sh

./configure --prefix=/usr/i586-mingw32msvc --host=i586-mingw32msvc --target=i586-mingw32msvc --with-msw --enable-mingw32 CXXFLAGS="-g -O2 -ffast-math -march=i686"
