#!/bin/sh
#
# Simple wrapper script around autoreconf and configure, with some
# standard developer flags set.  This is designed to be a decent default
# setup for a developer:  No optimization, no shared objects, no usage tracking,
# autoconf caching, and mainter-mode is enabled.
#
# To just run autoreconf and not configure, set NOCONFIGURE=1
# To just run configure and not autoreconf, set NORECONF=1
#

package_name="plasma"
# Default configure flags:  
#   Enable makefile dependency rules
#   Disable optimization
#   Use a cache file.
conf_flags="--enable-maintainer-mode --disable-opt --disable-shared -C"

MAKE=`which make 2>/dev/null`
if test ! -x "$MAKE" ; then MAKE=`which gmake` ; fi
if test ! -x "$MAKE" ; then MAKE=`which make` ; fi
HAVE_GNU_MAKE=`$MAKE --version|grep -c "Free Software Foundation"`

if test "$HAVE_GNU_MAKE" != "1"; then 
	echo !!!! ERROR: You need GNU make to build $package_name; 
	echo !!!! $MAKE is not GNU make;
	exit 1; 
fi

echo Found GNU Make at $MAKE ... good.
echo This script runs autoreconf and configure.
echo To skip the reconfigure step, set NORECONF=1.
echo To skip the configure step, set NOCONFIGURE=1.
echo Put necessary arguments for configure on the command line.

srcdir=`dirname $0`
echo "Source dir (srcdir):  $srcdir"
test -z "$srcdir" && srcdir=.

(test -f $srcdir/configure.ac) || {
	echo  "Error: Directory "\`$srcdir\'" does not look like the"
    echo " top-level $PKG_NAME directory"
    exit 1
}

(autoreconf --version) < /dev/null > /dev/null 2>&1 || {
	echo
	echo "Error: You must have \`autoreconf' installed to compile $package_name."
	echo "It is available at ftp://ftp.gnu.org/pub/gnu"
	exit 1;
}

if test -z "$*"; then
	echo "Running \`configure' with no arguments."
	echo "Arguments may be passed on the \`$0\` command line."
fi

if [ x$NORECONF = x ]; then
	echo "Running autoreconf.  To skip this step, prefix script with NORECONF=1"
	autoreconf --install --force || exit 1
else
	echo "Skipping autoreconf process."
fi

echo "Done with automake processing."

# Run configure unless NOCONFIGURE is set.
if [ x$NOCONFIGURE = x ]; then
	echo Running $srcdir/configure $conf_flags "$@" ...
	echo "To not run configure, run this script prefixed with NOCONFIGURE=1"
	$srcdir/configure $conf_flags "$@" \
		&& echo "Now type \'make\' to compile $package_name" || exit 1
else
	echo "Skipping configure process."
fi
