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

autoreconf --install --symlink --include=./macros

echo "Done with automake processing."

conf_flags="--enable-maintainer-mode --enable-compile-warnings" #--enable-iso-c

if test x$NOCONFIGURE = x; then
  echo Running $srcdir/configure $conf_flags "$@" ...
  $srcdir/configure $conf_flags "$@" \
  && echo Now type \`make\' to compile $PKG_NAME || exit 1
else
  echo Skipping configure process.
fi
