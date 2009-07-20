## Process this file with autoconf to produce a configure script.
AC_INIT([plasma],[0.8.45])
AM_INIT_AUTOMAKE
AM_MAINTAINER_MODE
AC_CONFIG_MACRO_DIR([./macros])

AC_CANONICAL_BUILD
AC_PROG_CC
AC_PROG_CXX
AC_PROG_INSTALL
AC_HEADER_STDC
AC_LANG([C++])
AM_PROG_AS

AM_CPP_SETUP(3,4)

#########################################################################
#  Configure arguments 
#########################################################################

## Configure flex.
FlexRequiredVersion=2.5.27
AM_FLEX($FlexRequiredVersion, , AC_MSG_ERROR([flex $FlexRequiredVersion is required in order to build this project.]))

## Check for the location of Perl
AC_PATH_PROG(PERLPROG,perl)
if test "x$PERLPROG" = x ; then
  AC_MSG_ERROR(["Could not find perl.  Check your path to make sure that it is present."])
fi

##
## Check to see if we should build the language front-end.
##
AC_MSG_CHECKING([if the user wants the language front-end])
lang_frontend=true
AC_ARG_ENABLE(frontend,AS_HELP_STRING([--enable-frontend],[Create the language front-end libraries.  Default is true.]),
[case "${enableval}" in
  yes) 
	lang_frontend=true
	;;
  no)  
	lang_frontend=false
	;;
  *) 
	AC_MSG_ERROR(bad value ${enableval} for --enable-frontend)
	;;
esac],[lang_frontend=true])

AH_TEMPLATE([LANG_FRONTEND],[Defined and set to 1 if we're creating the language front-end.])	
if [[ $lang_frontend = true ]] ; then
	AC_MSG_RESULT([yes]) 
	AC_DEFINE([LANG_FRONTEND],1)
else
	AC_MSG_RESULT([no]) 
fi

AM_CONDITIONAL([LANG_FRONTEND],[test x$lang_frontend = xtrue])

##
## Check to see if we should build the language runtime libraries.
##
AC_MSG_CHECKING([if the user wants the language runtime])
lang_runtime=true
AC_ARG_ENABLE(runtime,AS_HELP_STRING([--enable-runtime],[Create the language runtime libraries.  Default is true.]),
[case "${enableval}" in
  yes) 
	lang_runtime=true
	;;
  no)  
	lang_runtime=false
	;;
  *) 
	AC_MSG_ERROR(bad value ${enableval} for --enable-runtime)
	;;
esac],[lang_runtime=true])

# We don't bother with the runtime if the front-end isn't enabled.
if [[ $lang_frontend = false ]]; then
	lang_runtime=false;
fi

AH_TEMPLATE([LANG_RUNTIME],[Defined and set to 1 if we're creating the language runtime.])	
if [[ $lang_runtime = true ]] ; then
	AC_MSG_RESULT([yes]) 
	AC_DEFINE([LANG_RUNTIME],1)
else
	AC_MSG_RESULT([no]) 
fi

AM_CONDITIONAL([LANG_RUNTIME],[test x$lang_runtime = xtrue])

# If compiling on x86, we can compile test programs generated
# by the sample compiler in the regression suite.  However, we
# turn this off for Cygwin right now b/c we end up with link errors.
if [[ $build_cpu = i686 ]]; then
  COMPILE_OK=1
elif [[ $build_cpu = x86_64 ]]; then
  COMPILE_OK=1
  MINI_CC_FLAGS="-m32"	
else
  COMPILE_OK=0
fi
case ${build} in
	*cygwin)
	COMPILE_OK=0
	;;
esac 
# We need the front-end and runtime to create the compiler.
if [[ $lang_runtime = false -o $lang_frontend = false ]]; then
	COMPILE_OK=0
fi
AM_CONDITIONAL(CompileOk, [ test x$COMPILE_OK = x1 ])
AC_SUBST(COMPILE_OK)
AC_SUBST(CONFOPTS)
AC_SUBST(MINI_CC_FLAGS)

## Check for web directory (documentation destination).
WEBURL=
ac_weburl_default=http://compass.freescale.net/go/PlasmaSite
AC_ARG_WITH([weburl],AS_HELP_STRING([--with-weburl],[specify a the URL to web documentation.  The default is $ac_weburl_default.]),
	    WEBURL=$withval,WEBURL=$ac_weburl_default)
AC_SUBST(WEBURL)
DISTURL=
ac_disturl_default=http://compass.freescale.net/go/PlasmaDist
AC_ARG_WITH([disturl],AS_HELP_STRING([--with-disturl],[specify a the URL to the distribution.  The default is $ac_disturl_default.]),
	    DISTURL=$withval,DISTURL=$ac_disturl_default)
AC_SUBST(DISTURL)

AC_MSG_CHECKING([for info destination directory])
ac_infohome_default=/proj/ppctools/www/a-z/plasma
InfoHome=
AC_ARG_WITH([infohome],AS_HELP_STRING([--with-infohome],[specify a web destination directory for the info.xml file.  The default is $ac_infohome_default.]),InfoHome=$withval,InfoHome=$ac_infohome_default)
if [[ -d $InfoHome ]]; then
  AC_MSG_RESULT([$InfoHome])
else
  AC_MSG_RESULT([Directory $InfoHome not found.  Documentation will not be installed.])
fi
AC_SUBST(InfoHome)

## Check to see if compass exists.  If not, then we don't install documentation.
AC_MSG_CHECKING([to see if Compass exists])
CompassPlasmaDir=162987954
CompassDistDir=162979417
CompassExists=1
`ping -c 1 compass.freescale.net > /dev/null` || CompassExists=
if [[ !$CompassExists ]] ; then
	AC_MSG_RESULT([yes.  Documentation will be installed to Compass.])
else
	AC_MSG_RESULT([no.  Documentation will not be installed.])
fi

AC_SUBST(CompassPlasmaDir)
AC_SUBST(CompassDistDir)
AC_SUBST(CompassParam1)
AC_SUBST(CompassExists)

## Get the full compiler path.
AC_PATH_PROG(CXXPROG,$CXX)
## If this is a symbolic link, try to read the link.
if [[ -L $CXXPROG ]]; then
  CXX_temp=`readlink $CXXPROG`
  if [[ ! -z $CXX_temp ]]; then
    CXXPROG="$CXX_temp"
  fi
fi
echo "Compiler is located at:  $CXXPROG"

##
## Libtool ltdl library will be built as "convenience library"
## (see Autobook for more info)
##
# We want to use our version of libltdl.
export enable_ltdl_install=yes
AC_LIBLTDL_INSTALLABLE

## Setup to use libtool.
LT_INIT
LTDL_INIT

##
## Configure libltdl
##
AC_LTDL_ENABLE_INSTALL
AC_LIB_LTDL
AC_SUBST(top_build_prefix)
AC_SUBST(LIBLTDL)
AC_SUBST(INCLTDL)

## Configure Quickthreads.
AM_QT

## Configure the documentation tools.
AM_DOCS

if [[ x$SKRIBE = x ]]; then
  echo "! The skribe program was not found.  Documentation will not be built."
  echo "! To build the documentation, make sure that skribe is installed and in your path."
  echo "! You can find skribe here:  http://www-sop.inria.fr/mimosa/fp/Skribe/"
fi

if [[ x$PDFLATEX = x -o x$BIBTEX = x ]] ; then
  echo "! PDF documentation will not be build because pdflatex could not be found."
fi

# If we're using a newer version of tar, then we have to tweak the options so
# that tar doesn't use V7 mode, which has a problem with the use of big UIDs.
# Unfortunately, newer verions of tar automatically switch into v7 mode if they
# see the -o option being used, which is automake's default.  We have to use the
# --no-same-user option, instead, which is the same flag, just the long-form.  This
# doesn't cause the switch.
AC_MSG_CHECKING([if uid conversion will be done by tar])
tar_version=$(tar --version | grep tar | awk '{print $[4]}')
DPP_VERSION_CHECK(${tar_version},1.15,uid_conversion=no,uid_conversion=yes)
if [[ $uid_conversion = no ]]; then
	am__tar=$(echo $am__tar | sed -e 's/chof -/ch --no-same-owner -f -/')
	AC_MSG_RESULT([no.  New tar invocation:  ${am__tar}])
else
	AC_MSG_RESULT([yes.  No change required to tar arguments.])
fi

## Do final substitions and generation.
CXXFLAGS="-g -Wall $CPPSETUP"

## Add on macro directory to ACLOCAL_FLAGS.
ACLOCAL_FLAGS="${ACLOCAL_FLAGS} -I ./macros"

AC_SUBST(PROFILE)
AC_SUBST(ACLOCAL_FLAGS)
AC_SUBST(CXXFLAGS)
AC_SUBST(CXXPROG)

# The basic system libraries needed are just -ld.  If compiling w/o
# shared libraries, however, we need pthread as well.
SYS_LIBS="-ldl"
EXTRA_LDFLAGS=

if [[ x${enable_shared} = xno ]]; then
  SYS_LIBS="${SYS_LIBS} -lpthread"
  EXTRA_LDFLAGS=" -Wl,--allow-multiple-definition"
fi

# We need the multiple-definition stuff in Cygwin as well.
case ${build} in
	*cygwin)
	EXTRA_LDFLAGS="${EXTRA_LDFLAGS} -Wl,--allow-multiple-definition"
	;;
esac

AC_SUBST(SYS_LIBS)
AC_SUBST(EXTRA_LDFLAGS)



AC_CONFIG_COMMANDS([ptree-gen],
    [(cd ${srcdir}/opencxx/parser && chmod +x ptree-gen && ./ptree-gen)],
    [])

# Generate configuration files.
AC_CONFIG_FILES([\
   Makefile \
   macros/Makefile \
   scripts/Makefile \
   qt/Makefile \
   qt/md/Makefile \
   src/Makefile \
   opencxx/Makefile \
   parser/Makefile \
   doc/Makefile \
   tests/Makefile \
   tests/parsing/Makefile \
   tests/basic/Makefile \
   tests/sw/Makefile \
   tests/eav/Makefile \
   tests/ecu/Makefile \
   tests/lemon/Makefile \
   tests/cc/Makefile \
   tests/cc/tests/Makefile
])

AC_CONFIG_FILES([\
	scripts/plasma \
],[chmod +x,-w scripts/plasma ])

AC_CONFIG_FILES([\
	scripts/plasma-config \
],[chmod +x,-w scripts/plasma-config ])

AC_CONFIG_FILES([\
	tests/cc/compile_check.pm
],[chmod +x,-w tests/cc/compile_check.pm ])

AC_CONFIG_SUBDIRS(gc)

AC_OUTPUT
