## Process this file with automake to produce Makefile.in
#
# OpenC++
# src/Makefile.am
#
# Copyright (C) 2001 Other Contributors.
#
# Permission to use, copy, distribute and modify this software and its
# documentation for any purpose is hereby granted without fee, provided that
# the above copyright notice appear in all copies and that both that copyright
# notice and this permission notice appear in supporting documentation.
#
# Other Contributors make no representations about the suitability of this
# software for any purpose. It is provided "as is" without express or implied
# warranty.
# 
# Other Contributors are: Grzegorz Jakacki.


#
# See 'info autoconf' for explanation on srcdir (source directory),
# builddir (build directory, aka. objdir) and bindir, libdir, includedir
# etc. (directories, in which the built things will be instaled).
#
# See 'info automake' for exaplanation on pkgincludedir, recursive
# makefiles etc.
#

# Auxiliary definitions
# =====================

#
# Those two are obvious
#
TOP_BUILDDIR = ..
TOP_SRCDIR = @top_srcdir@

#
# The directory to include gc.h and gc_cpp.h from
#
GC_INCLUDEDIR = $(TOP_SRCDIR)/gc

#
# The location of libgc; 
# note, that we are using libtool, so it is libgc.la, not libgc.a
#
LIBGC = $(TOP_BUILDDIR)/gc/libgc.la

#
# all OpenC++ sources
#
SOURCES_all = buffer.cc hash.cc token.cc ptree.cc ptree-core.cc \
	encoding.cc env.cc pattern.cc walker.cc typeinfo.cc parse.cc \
	 main-con.cc mop.cc classwalk.cc metaclass.cc quote-class.cc \
	member.cc driver.cc driver2.cc cbodywalk.cc

#
# all OpenC++ headers
#
HEADERS_all = \
	buffer.h cbodywalk.h classwalk.h encoding.h env.h hash.h member.h\
	metaclass.h mop.h parse.h ptree-core.h ptree.h quote-class.h \
	token.h typeinfo.h types.h walker.h


# Rules for 'occ'
# ===============

#
# we are building excatly one executable, namely 'occ'
# (the definition bellow means, that it will also be installed
# by 'make install')
#
bin_PROGRAMS		= occ

occ_LINK = $(LIBTOOL) --mode=link $(CXX) -o $@

#
# occ sources are:
#
occ_SOURCES		= $(SOURCES_all)

#
# when linking occ we need to add libltdl (Libtool dlopen library)
# and libgc (Boehm's Garbage Collector)
#
occ_LDADD		= $(LIBLTDL) $(LIBGC)

#
# :KLUDGE: object files are linked into occ executable
#     and libocc library; libocc is linked with libtool;
#     since version 1.6 Automake does not allow to
#     link the same object file with and without libtool;
#     we pretend, that compiling occ objects needs special
#     flags, so that Automake creates two object files
#     for each source---for linking with and without libtool;
#     this kludge can be possibly fixed by rearranging linking
#     process, so that executable is obtained from libtool library.
#
occ_CXXFLAGS		= $(CXXFLAGS)

# Rules for 'libocc'
# ==================

#
# We want to make libtool library (.la)
# (the definition below means, that it also will be installed
# by 'make install')
#
lib_LTLIBRARIES		= libocc.la

#
# libocc sources are:
#
libocc_la_SOURCES	= $(SOURCES_all)

#
# when building libocc we need to add libltdl (Libtool dlopen library)
# and libgc (Boehm's Garbage Collector)
#
libocc_la_LIBADD	= $(LIBLTDL) $(LIBGC)



# Installing
# ==========

#
# the following headers have to be installed in the usual location
# for headers with 'opencxx' (package name) appended, e.g.
# '/usr/include/opencxx'
#
pkginclude_HEADERS	= env.h member.h mop.h \
			  ptree-core.h ptree.h token.h buffer.h typeinfo.h types.h walker.h

# Distribution
# ============

# Normally all sources and files, which we require to install go
# into the distribution tarball (opencxx-<version>.tar.gz).
# Nevertheless, the following files has to go there too:
#
EXTRA_DIST = $(HEADERS_all)


# Additional dependencies
# =======================

#
# We want the following declarations to be included verbatim in Makefile.in
# (from which Autoconf will generate Makefile)
#

#
# How to find Boehm's GC and Libtool's libltdl include files
#
INCLUDES = -I$(GC_INCLUDEDIR) $(INCLTDL)

#
# The End
#
