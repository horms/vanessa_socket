#!/bin/bash
# Running bash because /bin/sh is a complete piece of shit on solaris
# I wonder how long it will take the fuckwits at Sun to wake up

# Run this to generate all the initial makefiles, etc.

srcdir=`dirname $0`
test -z "$srcdir" && srcdir=.

THEDIR=`pwd`
cd $srcdir

DIE=0

(autoconf --version) < /dev/null > /dev/null 2>&1 || {
	echo
	echo "You must have autoconf installed to compile vanessa_socket."
	echo "Download the appropriate package for your distribution,"
	echo "or get the source from ftp://ftp.gnu.org/pub/gnu/autoconf/"
	DIE=1
}

(automake --version) < /dev/null > /dev/null 2>&1 || {
	echo
	echo "You must have automake installed to compile vanessa_socket."
	echo "Download the appropriate package for your distribution,"
	echo "or get the source from ftp://ftp.gnu.org/pub/gnu/automake/"
	DIE=1
}

(libtoolize --version) < /dev/null > /dev/null 2>&1 || {
	echo
	echo "You must have libtool installed to compile vanessa_socket."
	echo "Download the appropriate package for your distribution,"
	echo "or get the source from ftp://ftp.gnu.org/pub/gnu/libtool/"
	DIE=1
}

if test "$DIE" -eq 1; then
	exit 1
fi

if test -z "$*"; then
	echo "I am going to run ./configure with no arguments - if you wish "
        echo "to pass any to it, please specify them on the $0 command line."
fi

case $CC in
xlc )
    am_opt=--include-deps;;
esac

aclocal $ACLOCAL_FLAGS
(autoheader --version)  < /dev/null > /dev/null 2>&1 && autoheader
automake --add-missing $am_opt
autoconf
libtoolize --ltdl
cd $THEDIR

$srcdir/configure "$@" || exit $?

echo 
echo "Now type 'make' to compile vanessa_socket."
