#!/bin/sh
# Run this to generate all the initial makefiles, etc.

(autoconf --version) < /dev/null > /dev/null 2>&1 || {
  echo
  echo "Error: You must have \`autoconf' installed to compile $PKG_NAME."
  echo "It is available at ftp://ftp.gnu.org/pub/gnu/"
  exit 1;
}

# uncomment this to check for libtool
# (grep "^AM_PROG_LIBTOOL" $srcdir/configure.ac >/dev/null) && {
#   (libtool --version) < /dev/null > /dev/null 2>&1 || {
#     echo
#     echo "Error: You must have \`libtool' installed to compile $PKG_NAME."
#     echo "It is available at ftp://ftp.gnu.org/pub/gnu/"
#     exit 1;
#   }
# }

(automake --version) < /dev/null > /dev/null 2>&1 || {
  echo
  echo "Error: You must have \`automake' installed to compile $PKG_NAME."
  echo "It is available at ftp://ftp.gnu.org/pub/gnu/automake-1.3.tar.gz"
  exit 1;
}

# if no automake, don't bother testing for aclocal
test -n "$NO_AUTOMAKE" || (aclocal --version) < /dev/null > /dev/null 2>&1 || {
  echo
  echo "Error: Missing \`aclocal'. Make sure you have \`automake'"
  echo "installed version 1.3 or later."
  echo "It is available at ftp://ftp.gnu.org/pub/gnu/automake-1.3.tar.gz"
  exit 1;
}

if test -z "$*"; then
  echo "Running \`configure' with no arguments."
  echo "Arguments may be passed on the \`$0\` command line."
fi

case $CC in
    xlc )
	am_opt=--include-deps;;
esac

for configure_ac in `find $srcdir -name configure.ac -print`
do 
  directory=`dirname $configure_ac`
  if test -f $directory/NO-AUTO-GEN; then
    echo skipping $directory -- flagged as no auto-gen
  else
    echo processing $directory
    macrodirs=`sed -n -e 's,AM_ACLOCAL_INCLUDE(\(.*\)),\1,gp' < $configure_ac`
    ( cd $directory
      aclocalinclude="-I ./macros $ACLOCAL_FLAGS"
      for k in $macrodirs; do
  	if test -d $k; then
          aclocalinclude="$aclocalinclude -I $k"
  	else 
	  echo "Warning: No such directory \`$k'.  Ignored."
        fi
      done
      if grep "^AM_PROG_LIBTOOL" configure.ac >/dev/null; then
	echo "Running libtoolize..."
	libtoolize --force --copy
      fi
      echo "Running aclocal $aclocalinclude ..."
      aclocal $aclocalinclude
      if grep "^AM_CONFIG_HEADER" configure.ac >/dev/null; then
	echo "Running autoheader..."
	autoheader
      fi
      echo "Running automake --gnu $am_opt"
      automake --add-missing --gnu $am_opt
      echo "Running autoconf ..."
      autoconf
    )
  fi
done

conf_flags="--enable-maintainer-mode --enable-compile-warnings" #--enable-iso-c

if test x$NOCONFIGURE = x; then
  echo Running $srcdir/configure $conf_flags "$@" ...
  $srcdir/configure $conf_flags "$@" \
  && echo Now type \`make\' to compile $PKG_NAME || exit 1
else
  echo Skipping configure process.
fi
