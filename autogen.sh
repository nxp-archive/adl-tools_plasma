#!/bin/sh

PKG_NAME="Plasma"

MAKE=`which gnumake 2>/dev/null`
if test ! -x "$MAKE" ; then MAKE=`which gmake` ; fi
if test ! -x "$MAKE" ; then MAKE=`which make` ; fi
HAVE_GNU_MAKE=`$MAKE --version|grep -c "Free Software Foundation"`

if test "$HAVE_GNU_MAKE" != "1"; then 
echo !!!! ERROR: You need GNU make to build $PKG_NAME; 
echo !!!! $MAKE is not GNU make;
exit 1; 
fi

echo Found GNU Make at $MAKE ... good.
echo This script runs configure and make...
echo Put remember necessary arguments for configure on command line.

srcdir=`dirname $0`
echo "srcdir $srcdir"
test -z "$srcdir" && srcdir=.

(test -f $srcdir/configure.ac \
  && test -d $srcdir/src ) || {
    echo  "Error: Directory "\`$srcdir\'" does not look like the"
    echo " top-level $PKG_NAME directory"
    exit 1
}

. $srcdir/scripts/autogen.sh

