#! /bin/sh -ex
aclocal
autoheader
automake -a
autoconf
