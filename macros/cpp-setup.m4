## AM_CPP_SETUP
##
## This performs a variety of checks related to C++ usage and sets various
## variables accordingly.  The idea is that you include the variable CPPSETUP
## when defining CXXFLAGS.
##
## If the --enable-opt option is set, then OPT is set to a flag.
##
## Test to see if --enable-profile is set.  If it is, we clear FPIC (if
## on Solaris).
##
## If --enable-profile is set, PROFILE is set to a flag.
## 
## If --enable-efence is set, we set EFENCE to the location of the library
## and substitute that variable.  The user can specify a location of Electric Fence
## by supplying a value for this option.
##
## Test to see if fpic is needed.  We assume it is required Solaris.
## If it is required, we set the variable FPIC to the contents of the flag.
##
## We also define, and substitute, the variable RFLAG, which is set to something
## that should act as an option to encode a path for library lookup.  You can use
## this in a config file, e.g. "@RFLAG@/foo/bar".
##
## AC_DEFUN([MAJOR],[MINOR])
##
## If major and minor are ommitted, no version checking is performed.  If included,
## GCC must have at least this version number in order to pass.
##
AC_DEFUN([AM_CPP_SETUP],
[
AC_MSG_CHECKING( to see if -fPIC is needed)
MY_OS=`/usr/bin/env uname -s`
if test x$MY_OS = xSunOS ; then
   AC_MSG_RESULT([yes])
   FPIC="-fPIC"
else
   FPIC=
   AC_MSG_RESULT([no])
fi

RFLAG="-Wl,-R"
AC_SUBST(RFLAG)

AC_ARG_ENABLE(opt,
[  --enable-opt    Enable optimization flag (-O3), disables debug],
 OPT="$CXXFLAGS -O3"
 AC_MSG_RESULT([Optimizing with -O3]),
)

AC_ARG_ENABLE(static,
[  --enable-static    Enable static building],
 FPIC=""
 AC_MSG_RESULT([Static build selected.]),
)

PROFILE=
AC_ARG_ENABLE(profile,
[  --enable-profile    Enable profiling],
 PROFILE="-pg"
 AC_MSG_RESULT("Profiling with -pg")
 if test x$OS = xSunOS ; then
   echo "Turning off position-independent code b/c we're on Solaris and we are profiling."
   FPIC=
 fi,
)

default_efence="/nfs/ibmoto/tools/tlv4/raptor/lib/obj.SunOS/libefence.a"

AC_ARG_ENABLE(efence,
[  --enable-efence=<location>    link test programs with electric fence. If no location
                                 is given, /nfs/ibmoto/tools/tlv4/raptor/lib/obj.SunOS/libefence
                                 is used],
 if test x$enableval = x; then
   EFENCE=$default_efence
 else
   EFENCE=$enableval
 fi
 if test -f $EFENCE; then
   AC_MSG_RESULT([using '$EFENCE' for electric fence])
 else
   AC_MSG_ERROR(electric fence could not be found as file $EFENCE)
 fi
AC_MSG_RESULT(linking with electric fence - $EFENCE),
EFENCE=""
AC_MSG_RESULT(efence will not be used)
)
AC_SUBST(EFENCE)

##
## Include this flag in CXXFLAGS.
##
CPPSETUP="$OPT $FPIC $PROFILE"

##
## Remember GCC major, minor, and micro versions. Sometimes
## micro is missing. In those cases, set micro to 0.
##
GCC_VERSION=`$CC -dumpversion`
## Note the use of quadrigraphs below to get autoconf to put $1 and $2
## into the awk script.  Refer to the autoconf manual for more information.
GCC_MAJOR=`echo $GCC_VERSION | awk -F. ' { print @S|@1; }'`
GCC_MINOR=`echo $GCC_VERSION | awk -F. ' { print @S|@2; }'`
AC_MSG_RESULT([GCC Version:  $GCC_MAJOR.$GCC_MINOR])

REQUIRED_MAJOR=$1
REQUIRED_MINOR=$2

## Do version checking.
AC_MSG_CHECKING([that gcc has the required version ($REQUIRED_MAJOR.$REQUIRED_MINOR)])
if [[ x$REQUIRED_MAJOR != x ]]; then
  if [[ $GCC_MAJOR -lt $REQUIRED_MAJOR ]]; then
    AC_MSG_ERROR([GCC must have a minimum major version of $REQUIRED_MAJOR.])
  fi
  if [[ x$REQUIRED_MINOR != x -a $GCC_MAJOR -eq $REQUIRED_MAJOR ]]; then
    if [[ $GCC_MAJOR -lt $REQUIRED_MINOR ]]; then
      AC_MSG_ERROR([GCC must have a minimum minor version of $REQUIRED_MINOR.])
    fi
  fi
fi
AC_MSG_RESULT([okay])

AC_SUBST(GCC_VERSION)
AC_SUBST(GCC_MAJOR)
AC_SUBST(GCC_MINOR)

])
