dnl ************* <auto-copyright.pl BEGIN do not edit this line> *************
dnl Doozer++ is (C) Copyright 2000-2003 by Iowa State University
dnl
dnl Original Author:
dnl   Patrick Hartling
dnl
dnl This library is free software; you can redistribute it and/or
dnl modify it under the terms of the GNU Library General Public
dnl License as published by the Free Software Foundation; either
dnl version 2 of the License, or (at your option) any later version.
dnl
dnl This library is distributed in the hope that it will be useful,
dnl but WITHOUT ANY WARRANTY; without even the implied warranty of
dnl MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
dnl Library General Public License for more details.
dnl
dnl You should have received a copy of the GNU Library General Public
dnl License along with this library; if not, write to the
dnl Free Software Foundation, Inc., 59 Temple Place - Suite 330,
dnl Boston, MA 02111-1307, USA.
dnl
dnl -----------------------------------------------------------------
dnl File:          general.m4,v
dnl Date modified: 2003/10/07 16:52:44
dnl Version:       1.44.2.16
dnl -----------------------------------------------------------------
dnl ************** <auto-copyright.pl END do not edit this line> **************

dnl ---------------------------------------------------------------------------
dnl Compare two version numbers separated into their components.
dnl
dnl Usage:
dnl     DPP_VERSION_COMPARE(known-major, known-minor, known-patch,
dnl                         required-major, required-minor, required-patch
dnl                         [, action-if-found [, action-if-not-found]])
dnl
dnl Arguments:
dnl     known-major         - The known major number value to be compared.
dnl     known-minor         - The known minor number value to be compared.
dnl     known-patch         - The known patch number value to be compared.
dnl     required-major      - The minimum required major number.
dnl     required-minor      - The minimum required minor number.
dnl     required-patch      - The minimum required patch number.
dnl     action-if-found     - The action to take if the known version number
dnl                           is at least the required version number.  This
dnl                           is optional.
dnl     action-if-not-found - The action to take if the known version number
dnl                           is not at least the required version number.
dnl                           This is optional.
dnl ---------------------------------------------------------------------------
define(DPP_VERSION_COMPARE,
[
   if test `expr $3 + $2 \* 1000 + $1 \* 1000000 \>= $6 + $5 \* 1000 + $4 \* 1000000` = 1
   then
      $7
      true
   else
      $8
      true
   fi
])

dnl ---------------------------------------------------------------------------
dnl Compare two version numbers.  The first is the version number in question,
dnl and the second is the minimum required value.
dnl
dnl Usage:
dnl     DPP_VERSION_CHECK(known-version, required-version
dnl                       [, action-if-found [, action-if-not-found]])
dnl
dnl Arguments:
dnl     known-version       - The known version number to be tested.  It must
dnl                           be of the form major.minor[.patch]
dnl     required-version    - The minimum required version number.  It must
dnl                           be of the form major.minor[.patch]
dnl     action-if-found     - The action to take if the known version number
dnl                           is at least the required version number.  This
dnl                           is optional.
dnl     action-if-not-found - The action to take if the known version number
dnl                           is not at least the required version number.
dnl                           This is optional.
dnl ---------------------------------------------------------------------------
AC_DEFUN([DPP_VERSION_CHECK],
[
   split_known_ver=`echo $1 | sed -e 's/\./ /g'`
   split_req_ver=`echo $2 | sed -e 's/\./ /g'`

   known_major=`echo $split_known_ver | awk '{ print $[1] }' -`
   known_minor=`echo $split_known_ver | awk '{ print $[2] }' -`
   known_patch=`echo $split_known_ver | awk '{ print $[3] }' -`

   if test "x$known_patch" = "x" ; then
      known_patch=0
   fi

   req_major=`echo $split_req_ver | awk '{ print $[1] }' -`
   req_minor=`echo $split_req_ver | awk '{ print $[2] }' -`
   req_patch=`echo $split_req_ver | awk '{ print $[3] }' -`

   if test "x$req_patch" = "x" ; then
      req_patch=0
   fi

   DPP_VERSION_COMPARE([$known_major], [$known_minor], [$known_patch],
                       [$req_major], [$req_minor], [$req_patch],
                       [$3], [$4])
])

dnl ---------------------------------------------------------------------------
dnl Wrap a call to DPP_VERSION_CHECK inside a call to AC_CACHE_CHECK.  This is
dnl a helper macro to simplify user-level code down to the bare minimum.
dnl
dnl Usage:
dnl     DPP_VERSION_CHECK_MSG(pkg-name, known-version, required-version, cache_var [, action-if-found [, action-if-not-found]])
dnl
dnl Arguments:
dnl     pkg-name            - The name of the package whose version number
dnl                           will be tested.
dnl     known-version       - The known version number to be tested.  It must
dnl                           be of the form major.minor[.patch]
dnl     required-version    - The minimum required version number.  It must
dnl                           be of the form major.minor[.patch]
dnl     cache-var           - A variable to be used for caching the comparison
dnl                           results.
dnl     action-if-found     - The action to take if the known version number
dnl                           is at least the required version number.  This
dnl                           is optional.
dnl     action-if-not-found - The action to take if the known version number
dnl                           is not at least the required version number.
dnl                           This is optional.
dnl ---------------------------------------------------------------------------
AC_DEFUN([DPP_VERSION_CHECK_MSG],
[
   AC_CACHE_CHECK([whether $1 version is >= $3], [$4],
      [DPP_VERSION_CHECK([$2], [$3], [$4="$2"],
         [$4='no'
          AC_MSG_RESULT([$2])])])

   dnl Successful version comparison.  Note that $4 is a variable name, not
   dnl the result of evaluating a variable.  As such, we have to prepend a '$'
   dnl to its name here in order to get its value.
   if test "x$$4" != "xno" ; then
      ifelse([$5], , :, [$5])
      true
   dnl Failed version comparison.
   else
      ifelse([$6], , :, [$6])
      true
   fi
])
