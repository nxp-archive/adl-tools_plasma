## Process this file with autoconf to produce a configure script.
AC_INIT(src/Interface.C)

PLASMA_MAJOR_VERSION=0
PLASMA_MINOR_VERSION=1
PLASMA_MICRO_VERSION=0
PLASMA_VERSION=$PLASMA_MAJOR_VERSION.$PLASMA_MINOR_VERSION.$PLASMA_MICRO_VERSION

AC_SUBST(PLASMA_MAJOR_VERSION)
AC_SUBST(PLASMA_MINOR_VERSION)
AC_SUBST(PLASMA_MICRO_VERSION)
AC_SUBST(PLASMA_VERSION)

VERSION=$PLASMA_VERSION

AM_INIT_AUTOMAKE($PACKAGE, $VERSION)
AM_CONFIG_HEADER(config.h)
AM_MAINTAINER_MODE
AC_PROG_RANLIB
AC_PROG_CC
AC_PROG_CPP
AC_PROG_CXX
AM_PROG_AS
AM_CPP_SETUP(3,2)
AC_LANG_CPLUSPLUS

#########################################################################
#  Configure arguments 
#########################################################################

## Check for the location of Perl
AC_PATH_PROG(PERLPROG,perl)
if test "x$PERLPROG" = x ; then
        AC_ERROR("Could not find perl.  Check your path to make sure that it is present.")
fi


## Configure Quickthreads.
AM_QT

## Configure OpenC++
AM_OCC

## Do final substitions and generation.
CXXFLAGS="-g -Wall $CPPSETUP"

AC_SUBST(PROFILE)
AC_SUBST(ACLOCAL_FLAGS)
AC_SUBST(CXXFLAGS)
AC_PROG_INSTALL
AC_STDC_HEADERS

AC_CONFIG_FILES([\
   Makefile \
   macros/Makefile \
   qt/Makefile \
   src/Makefile \
   parser/Makefile \
   tests/Makefile \
   tests/basic/Makefile
])

AC_OUTPUT
