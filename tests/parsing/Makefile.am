
TESTS = regress

check_PROGRAMS = parse-test

OccBuildDir = $(top_builddir)/opencxx
GcBuildDir = $(top_builddir)/gc
GcSrcDir = $(top_srcdir)/gc

OCC = $(OccBuildDir)/occ
OCCFLAGS = -I$(top_builddir) -I$(GcSrcDir)

parse_test_SOURCES = \
	ParseTest.mc

parse_test_LDADD = \
	$(OccBuildDir)/libocc.la $(LIBLTDL)

parse_test_LINK = $(LIBTOOL) --mode=link $(CXX) -o $@

noinst_HEADERS = \
	ParseTest.h

CLEANFILES= *.ii *-init.cc *.so

am__depfiles_maybe = depfiles

DEP_FILES = \
	./$(DEPDIR)/ParseTest.Po

include ./$(DEPDIR)/ParseTeset.Po

.mc.o:
	if $(OCC) --static-mc=$@ $(OCCFLAGS) $< -c -d"-MT $@" -d-MD -d-MP -d"-MF$(DEPDIR)/$*.Tpo" -- $(CXXFLAGS) ;\
	then mv "$(DEPDIR)/$*.Tpo" "$(DEPDIR)/$*.Po" ; \
	else rm -f "$(DEPDIR)/$*.Tpo"; exit 1; \
	fi	
