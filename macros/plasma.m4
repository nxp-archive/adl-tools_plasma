##
## Configure a program to use plasma.
##
## Adds --with-plasma to let a user specify the location of the
## plasma program.
##
## Output:
##
##   PLASMA:  The Plasma executable.
##   PLASMAPATH:  The Plasma install prefix.
##
AC_DEFUN([AM_PLASMA],
[
  AC_ARG_WITH([plasma],AC_HELP_STRING([--with-plasma],[specify a path to use to find the Plasma program.]),ac_plasma=$withval)
  if [[ x$ac_plasma = x ]] ; then
    ac_plasma=$PATH
  fi
  AC_PATH_PROG(PlasmaConfig, [plasma-config], no, $ac_plasma)
  AC_PATH_PROG(PLASMA, [plasma], no, $ac_plasma)

  if [[ $PLASMA = "no" ]] ; then
    AC_MSG_ERROR([Could not find plasma in the path.  Please modify your path or use the --with-plasma option.])
  fi
  if [[ $PlasmaConfig = "no" ]] ; then
    AC_MSG_ERROR([Could not find plasma-config in the path.  Please modify your path or use the --with-plasma option.])
  fi

  PLASMAPATH=`$PlasmaConfig --prefix`

  AC_MSG_CHECKING([that we can compile a plasma program.])
  ac_ext=pa
  ac_compile='$PLASMA -c $CXXFLAGS $CPPFLAGS conftest.$ac_ext >&5'
  ac_link='$PLASMA -o conftest$ac_exeext $CFLAGS $CPPFLAGS $LDFLAGS conftest.$ac_ext $LIBS >&5'
  AC_RUN_IFELSE([

#include "plasma/plasma.h"

    using namespace plasma;

    int pMain(int argc,const char *argv[[]]) {
	par {
	  mfprintf (stderr,"Thread 1.\n");
	  mfprintf (stderr,"Thread 2.\n");
	}
	return 0;
    }
  ],AC_MSG_RESULT([ok.]),AC_MSG_ERROR([failed.]))

  CXX="$orig_CXX"

  AC_SUBST(PLASMA)
  AC_SUBST(PLASMAPATH)

])