#!/bin/sh
# Run this to generate all the initial makefiles, etc.

(autoreconf --version) < /dev/null > /dev/null 2>&1 || {
  echo
  echo "Error: You must have \`autoreconf' installed to compile $PKG_NAME."
  echo "It is available at ftp://ftp.gnu.org/pub/gnu"
  exit 1;
}

if test -z "$*"; then
  echo "Running \`configure' with no arguments."
  echo "Arguments may be passed on the \`$0\` command line."
fi

autoreconf --install --symlink --include=./macros || exit 1

echo "Done with automake processing."

# Default configure flags:  Enable makefile dependency rules 
# and disable optimization.
conf_flags="--enable-maintainer-mode --disable-opt"

# Run configure unless NOCONFIGURE is set.
if test x$NOCONFIGURE = x; then
  echo Running $srcdir/configure $conf_flags "$@" ...
  echo "To not run configure, run this script prefixed with NOCONFIGURE=1"
  $srcdir/configure $conf_flags "$@" \
  && echo Now type \`make\' to compile $PKG_NAME || exit 1
else
  echo Skipping configure process.
fi
