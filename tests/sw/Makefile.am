#
# Adding new Plasma tests is more complicated than it should be because
# Automake doesn't know that Plasma's dependencies can be handled like those
# of C++.  To add a new test, add an entry to check_PROGRAMS (call it <test> for
# our purposes).  Then create a variable called <test>_SOURCES and list source
# files.  If adding a plasma test, add a new entry to DEP_FILES and include the
# relevant dependency file (.Po).
#

TESTS = regress

check_PROGRAMS = \
	alt \
	comm \
	nalt

DEPENDENCIES = $(top_srcdir)/src/libplasma.a

alt_SOURCES = alt.pa
alt_DEPENDENCIES = $(DEPENDENCIES)

comm_SOURCES = comm.pa
comm_DEPENDENCIES = $(DEPENDENCIES)

nalt_SOURCES = nalt.pa
nalt_DEPENDENCIES = $(DEPENDENCIES)

AM_CXXFLAGS = $(CXXFLAGS) -I$(top_srcdir)/src -I$(top_srcdir)/gc/include

DEP_FILES=\
	./$(DEPDIR)/alt.Po \
	./$(DEPDIR)/comm.Po \
	./$(DEPDIR)/nalt.Po

include ./$(DEPDIR)/alt.Po
include ./$(DEPDIR)/comm.Po
include ./$(DEPDIR)/nalt.Po

AM_LDFLAGS = -L$(top_srcdir)/src -L$(top_srcdir)/qt -L$(top_srcdir)/gc/.libs -Wl,-R$(top_srcdir)/gc/.libs
LDADD =  -lplasma -lqt  -lgc -ldl

CXXLD = $(CXX)
LINK = $(CXXLD) $(AM_CXXFLAGS) $(CXXFLAGS) $(AM_LDFLAGS) $(LDFLAGS) -o $@

include $(top_srcdir)/tests/Makefile.rules
