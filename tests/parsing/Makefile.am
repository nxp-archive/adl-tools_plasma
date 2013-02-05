
TESTS = regress

check_PROGRAMS = parse-test

EXTRA_DIST = regress $(wildcard in*.*)

OccBuildDir = $(top_builddir)/opencxx
GcBuildDir = $(top_builddir)/gc
GcSrcDir = $(top_srcdir)/gc

OCC = $(OccBuildDir)/occ
OCCFLAGS = -I$(top_srcdir) -I$(GcSrcDir)

parse_test_SOURCES = \
	ParseTest.C

parse_test_CXXFLAGS = -I$(top_srcdir) -I$(GcSrcDir)

parse_test_LDADD = $(OccBuildDir)/libocc.la $(LIBLTDL)

noinst_HEADERS = \
	ParseTest.h

CLEANFILES= *.ii *-init.cc *.so
