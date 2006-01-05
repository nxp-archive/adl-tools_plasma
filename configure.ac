## Process this file with autoconf to produce a configure script.
AC_INIT([plasma],[0.8.23])
AM_INIT_AUTOMAKE
AM_MAINTAINER_MODE

AC_CANONICAL_BUILD
AC_PROG_CC
AC_PROG_CXX
AC_PROG_INSTALL
AC_STDC_HEADERS
AC_LANG_CPLUSPLUS
AM_PROG_AS
AC_PROG_LEX
AM_CPP_SETUP(3,4)

#########################################################################
#  Configure arguments 
#########################################################################

## Make sure flex exists.
if [[ x$LEX = x: ]] ; then
  AC_ERROR("Flex not found- this tool is required.")
fi

## Check for the location of Perl
AC_PATH_PROG(PERLPROG,perl)
if test "x$PERLPROG" = x ; then
  AC_ERROR("Could not find perl.  Check your path to make sure that it is present.")
fi

## Check for web directory (documentation destination).
AC_MSG_CHECKING([for web destination directory])
ac_web_default=/proj/ppctools/GSTM/www/plasma
ac_weburl_default=http://www.somerset.sps.mot.com/projects/tools/plasma
WEBHOME=
WEBURL=
DOCDIR=doc
AC_ARG_WITH([web],AS_HELP_STRING([--with-web],[specify a web destination directory.  The default is /proj/ppctools/GSTM/www/plasma.]),
	    WEBHOME=$withval,WEBHOME=$ac_web_default)
AC_ARG_WITH([weburl],AS_HELP_STRING([--with-weburl],[specify a the URL to web documentation.  The default is http://www.somerset.sps.mot.com/projects/tools/plasma.]),
	    WEBURL=$withval,WEBURL=$ac_weburl_default)

if [[ -d $WEBHOME ]]; then
  AC_MSG_RESULT([$WEBHOME])
else
  AC_MSG_RESULT([Directory $WEBHOME not found.  Documentation will not be installed.])
  DOCDIR=
fi

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
AC_PROG_LIBTOOL

##
## Configure libltdl
##
AC_LTDL_ENABLE_INSTALL
AC_LIB_LTDL
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
  DOCDIR=
fi

if [[ x$PDFLATEX = x -o x$BIBTEX = x ]] ; then
  echo "! PDF documentation will not be build because pdflatex could not be found."
fi

## Do final substitions and generation.
CXXFLAGS="$CXXFLAGS -g -Wall $CPPSETUP"

## Add on macro directory to ACLOCAL_FLAGS.
ACLOCAL_FLAGS="${ACLOCAL_FLAGS} -I ./macros"

AC_SUBST(PROFILE)
AC_SUBST(ACLOCAL_FLAGS)
AC_SUBST(CXXFLAGS)
AC_SUBST(CXXPROG)
AC_SUBST(WEBHOME)
AC_SUBST(WEBURL)
AC_SUBST(DOCDIR)

# If compiling on x86, we can compile test programs generated
# by the sample compiler in the regression suite.
if [[ $build_cpu = i686 ]]; then
  COMPILE_OK=1
  # We also set the -m32 flag so that if we use this on an Opteron,
  # Plasma will know to force 32-bit code generation.
  CONFOPTS="-m32"
else
  COMPILE_OK=0
fi
AM_CONDITIONAL(CompileOk, [ test x$COMPILE_OK = x1 ])
AC_SUBST(COMPILE_OK)
AC_SUBST(CONFOPTS)

AC_CONFIG_COMMANDS([ptree-gen],
    [(cd opencxx/parser && chmod +x ptree-gen && ./ptree-gen)],
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
	scripts/plasma-config \
	tests/cc/compile_check.pm
],[chmod +x,-w scripts/plasma scripts/plasma-config tests/cc/compile_check.pm ])

# We always want to enable C++ support (argument taken by GC library).
export enable_cplusplus=yes
AC_CONFIG_SUBDIRS(gc libltdl)

AC_OUTPUT
