## Process this file with autoconf to produce a configure script.
AC_INIT(src/Interface.C)

PLASMA_MAJOR_VERSION=0
PLASMA_MINOR_VERSION=3
PLASMA_MICRO_VERSION=0
PLASMA_VERSION=$PLASMA_MAJOR_VERSION.$PLASMA_MINOR_VERSION.$PLASMA_MICRO_VERSION

AC_SUBST(PLASMA_MAJOR_VERSION)
AC_SUBST(PLASMA_MINOR_VERSION)
AC_SUBST(PLASMA_MICRO_VERSION)
AC_SUBST(PLASMA_VERSION)

VERSION=$PLASMA_VERSION
PACKAGE=plasma

AM_INIT_AUTOMAKE($PACKAGE, $VERSION)
AM_CONFIG_HEADER(config.h)
AM_MAINTAINER_MODE
AC_CANONICAL_BUILD
AC_PROG_CC
AC_PROG_CPP
AC_PROG_CXX
AC_PROG_LIBTOOL
AC_PROG_INSTALL
AC_STDC_HEADERS
AC_LANG_CPLUSPLUS
AM_PROG_AS
AM_CPP_SETUP(3,2)
AC_PROG_LEX
AC_PROG_YACC

#########################################################################
#  Configure arguments 
#########################################################################

## Check for the location of Perl
AC_PATH_PROG(PERLPROG,perl)
if test "x$PERLPROG" = x ; then
  AC_ERROR("Could not find perl.  Check your path to make sure that it is present.")
fi

## Check for web directory (documentation destination).
AC_MSG_CHECKING([for web destination directory])
ac_web_default=/proj/ppctools/tlv/system-modelling/www
WEBHOME=
DOCDIR=doc
AC_ARG_WITH([web],AC_HELP_STRING([--with-web],[specify a web destination directory.  The default is $ac_web_default.]),WEBHOME=$withval,WEBHOME=$ac_web_default)

if [[ -d $WEBHOME ]]; then
  AC_MSG_RESULT([$WEBHOME])
else
  AC_MSG_RESULT([Directory $WEBHOME not found.  Documentation will not be installed.])
  DOCDIR=
fi

AC_PATH_PROG(TRIP,trip)
if test "x$TRIP" = x ; then
  echo "The trip program was not found.  Documentation will not be built."
  echo "To build the documentation, makee sure that trip is installed and in your path."
  DOCDIR=
fi

## Get the full compiler path.
AC_PATH_PROG(CXXPROG,$CXX)
echo "Compiler is located at:  $CXXPROG"

## Configure Quickthreads.
AM_QT

## Configure OpenC++
AM_OCC

## Do final substitions and generation.
CXXFLAGS="-g -Wall $CPPSETUP"

## Add on macro directory to ACLOCAL_FLAGS.
ACLOCAL_FLAGS="${ACLOCAL_FLAGS} -I ./macros"

AC_SUBST(PROFILE)
AC_SUBST(ACLOCAL_FLAGS)
AC_SUBST(CXXFLAGS)
AC_SUBST(CXXPROG)
AC_SUBST(WEBHOME)
AC_SUBST(DOCDIR)
AC_SUBST(TRIP)

# We always want to enable C++ support.
export enable_cplusplus=yes
AC_CONFIG_SUBDIRS(gc)

# If compiling on x86, we can compile test programs generated
# by the sample compiler in the regression suite.
if [[ $build_cpu = i686 ]]; then
  COMPILE_OK=1
else
  COMPILE_OK=0
fi
AM_CONDITIONAL(CompileOk, [ test x$COMPILE_OK = x1 ])
AC_SUBST(COMPILE_OK)

AC_CONFIG_FILES([\
   Makefile \
   macros/Makefile \
   scripts/Makefile \
   qt/Makefile \
   src/Makefile \
   parser/Makefile \
   doc/Makefile \
   tests/Makefile \
   tests/basic/Makefile \
   tests/sw/Makefile \
   tests/eav/Makefile \
   tests/ecu/Makefile \
   tests/lemon/Makefile \
   tests/cc/Makefile
])

AC_CONFIG_FILES([\
	scripts/plasma \
	scripts/plasma-config \
	tests/cc/compile_check.pm
],[chmod +x,-w scripts/plasma scripts/plasma-config tests/cc/compile_check.pm ])

AC_OUTPUT
