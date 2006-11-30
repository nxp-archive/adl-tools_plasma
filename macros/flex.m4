##
## AM_FLEX([version], [, action-if-found [,action-if-not-found]])
##   Find flex, check its version, and export any necessary variables for its use.
##
##   version:  Minimum required version,
##   action-if-found:  Executed if flex is found.
##   action-if-not-found:  Executed if flex is not found.
##
## Command-line options:
##   --with-flex-include: Specify location of flex's include file.
##
## Output:
##
##   LEX:             flex program.
##   LEX_CFLAGS:      Include directive/CFlags for using flex.
## 
##
AC_DEFUN([AM_FLEX],
[
  AC_ARG_WITH()

  FlexExists=yes
  AM_PROG_LEX
  if test "$LEX" != flex; then
    AC_MSG_WARN([Could not find flex in the path.  Please modify your path or use the --with-flex option.])
	FlexExists=no
  fi

  ac_flex_include=
  AC_ARG_WITH([flex-include],AC_HELP_STRING([--with-flex-include],[specify the location of flex's header file FlexLexer.h.]),ac_flex_include=$withval)

  if [[ $FlexExists != no ]] ; then

	# First, check for the version.
	FLEX_VERSION=`$LEX --version | awk '{print $[2] }'`
    AC_MSG_CHECKING([for flex's version ($1 required).])
    DPP_VERSION_CHECK([$FLEX_VERSION], [$1], , [FlexExists=no])	
    if [[ $FlexExists = no ]] ; then	
      AC_MSG_RESULT([Found $FLEX_VERSION but $1 is the minimum required.])
    else
      AC_MSG_RESULT([$FLEX_VERSION (okay).])
    fi

	AC_MSG_CHECKING([for FlexLexer.h])
	if [[ $FlexExists = yes ]] ; then
	  # Now try and find the header file required for creating C++ scanners.
	
	  # If the user has specified a path, then use it.  Otherwise, try to find flex's
	  # location and locate the include file from there.
  	  if [[ x$ac_flex_include = x ]] ; then
    	
	    # Try to discover the location using which.  If it's a symbolic link,
	    # we'll follow that.
	    loc=$(which $LEX)
	    if [[ -L $loc ]] ; then
		  loc=$(readlink -n $loc)
	    fi
		loc=$(dirname $loc)
	    if [[ -d $loc/../include ]] ; then
	  	  ac_flex_include=$(cd $loc/../include && pwd)
	    else

          # Wasn't found, so maybe we can use the wrapper.
	      loc=$(dirname $(w.which $LEX))

		  if [[ -d $loc/../include ]] ; then
	    	  loc=$(cd $loc && pwd)
	 	  else
			  AC_MSG_ERROR([Could not find FlexLexer.h's location either by using which or w.which.  You must specify its location manually using --with-flex-include])
		  fi

	    fi

  	  fi

	fi

  fi

  if [[ ! -e $ac_flex_include/FlexLexer.h ]] ; then
    AC_MSG_ERROR([Could not find FlexLexer.h's location.  You must specify its location manually using --with-flex-include])
  else
	AC_MSG_RESULT([$ac_flex_include/FlexLexer.h ])
  fi

  if [[ $FlexExists = yes ]]; then
    ifelse([$2], , :, [$2])
  else
    ifelse([$3], , :, [$3])
  fi

  LEX_CFLAGS="-I$ac_flex_include"
  AC_SUBST(LEX_CFLAGS)

])
