##
## Configures Quickthreads.  Use the --with-qt option to specify a Quickthreads
## host explicitly.  Otherwise, we'll try and figure it out from host_type.
##
## Input:   --with-qt (optional configure option).
##
AC_DEFUN(AM_QT,
[

## Get host type.
AC_CANONICAL_HOST

## Quickthreads setup.  Runs the Quickthreads configuration
## program with what it hopes is the right platform identifier.
## We have to do this mapping manually, unfortunately, so we'll need to
## add platforms here as we go.
AC_MSG_CHECKING([for Quickthreads host type])
ac_qt_host=
AC_ARG_WITH([qt],AC_HELP_STRING([--with-qt],[specify a Quickthreads host type explicitly.]),ac_qt_host=$withval)
if [[ x$ac_qt_host = x ]] ; then
  case $host_cpu in 
    i686) 
	ac_qt_host="iX86"
	;;
    *)  
	echo ""
	echo "Could not deduce the proper host type for Quickthreads configuration."
     	echo "Refer to qt/INSTALL for the list of valid host types, then use the --with-qt option to specify this value."
      	echo ""
	AC_MSG_ERROR([Failed to figure out Quickthreads host type.])
	;;
  esac
fi

AC_MSG_RESULT([$ac_qt_host])

# Now run Quickthread's configure program.
AC_MSG_NOTICE([Running config with host type $ac_qt_host])
(cd $srcdir/qt && ./config $ac_qt_host > /dev/null 2>&1) ||
  AC_MSG_ERROR([Failed to configure Quickthreads.])

])
