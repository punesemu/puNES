#!/bin/sh
libtoolize --force --copy
#for macos
glibtoolize --force --copy
aclocal
automake -a
autoconf
./configure $*
