##
## Configures OpenC++.  Use the --with-occ option to specify a path
## to libocc-config.  By default, we will try to run the program in
## the current path.
##
## Input:   --with-occ (optional configure option).
## Output:  OCCPATH:   Path to the OpenC++ installation.  Run occ with $OCCPATH/bin/occ.
##          OCCFLAGS:  OpenC++ C flags.
##          OCCLIBS:   OpenC++ link flags.
##          OCC:       Full path of executable.
##
AC_DEFUN(AM_OCC,
[
  AC_ARG_WITH([occ],AC_HELP_STRING([--with-occ],[specify a path to use to find the OpenC++ configuration program.]),ac_occ=$withval)
  if [[ x$ac_occ = x ]] ; then
    ac_occ=$PATH
  fi
  AC_PATH_PROG(OccConfig, [libocc-config], no, $ac_occ)

  if [[ $OccConfig = "no" ]] ; then
    AC_MSG_ERROR([Could not find libocc-config in the path.  Please modify your path or use the --with-occ option.])
  fi

  OCCPATH=`$OccConfig --instprefix`
  OCCFLAGS=`$OccConfig --cflags`
  OCCLIBS=`$OccConfig --libs`
  OCC="$OCCPATH/bin/occ"

  AC_MSG_CHECKING([that we can link against libocc])
  orig_CXXFLAGS="$CXXFLAGS"
  CXXFLAGS="$CXXFLAGS $($OccConfig --cflags)"
  orig_LIBS="$LIBS"
  LIBS="$LIBS $($OccConfig --libs)"
  orig_CXX="$CXX"
  CXX="./libtool --mode=link $CXX"
  AC_LINK_IFELSE([
    #include <opencxx/ptree-core.h>

    using namespace Opencxx;

    int main() {
      Ptree *p = Ptree::Make("foo!");
      return 0;
    }
  ],AC_MSG_RESULT([ok.]),AC_MSG_ERROR([failed.]))

  CXXFLAGS="$orig_CXXFLAGS"
  LIBS="$orig_LIBS"
  CXX="$orig_CXX"

  AC_SUBST(OCCPATH)
  AC_SUBST(OCCFLAGS)
  AC_SUBST(OCCLIBS)
  AC_SUBST(OCC)

])
