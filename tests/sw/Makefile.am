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
	nalt \
	ptimeout \
	queues \
	share \
	fhr \
	example \
	cim

DEPENDENCIES = $(top_srcdir)/src/libplasma.a

alt_SOURCES = alt.pa
alt_DEPENDENCIES = $(DEPENDENCIES)

comm_SOURCES = comm.pa
comm_DEPENDENCIES = $(DEPENDENCIES)

nalt_SOURCES = nalt.pa
nalt_DEPENDENCIES = $(DEPENDENCIES)

ptimeout_SOURCES = ptimeout.pa
ptimeout_DEPENDENCIES = $(DEPENDENCIES)

queues_SOURCES = queues.pa
queues_DEPENDENCIES = $(DEPENDENCIES)

share_SOURCES = share.pa Random.C
share_DEPENDENCIES = $(DEPENDENCIES)

fhr_SOURCES = fhr.pa Random.C
fhr_DEPENDENCIES = $(DEPENDENCIES)

example_SOURCES = example.pa Random.C
example_DEPENDENCIES = $(DEPENDENCIES)

cim_SOURCES = cim.pa Random.C
cim_DEPENDENCIES = $(DEPENDENCIES)

AM_CXXFLAGS = $(CXXFLAGS) -I$(top_srcdir)/src -I$(top_srcdir)/gc/include

DEP_FILES=\
	./$(DEPDIR)/alt.Po \
	./$(DEPDIR)/comm.Po \
	./$(DEPDIR)/nalt.Po \
	./$(DEPDIR)/ptimeout.Po \
	./$(DEPDIR)/queues.Po \
	./$(DEPDIR)/Random.Po \
	./$(DEPDIR)/fhr.Po \
	./$(DEPDIR)/example.Po \
	./$(DEPDIR)/cim.Po

include ./$(DEPDIR)/alt.Po
include ./$(DEPDIR)/comm.Po
include ./$(DEPDIR)/nalt.Po
include ./$(DEPDIR)/ptimeout.Po
include ./$(DEPDIR)/queues.Po
include ./$(DEPDIR)/fhr.Po
include ./$(DEPDIR)/example.Po
include ./$(DEPDIR)/cim.Po

AM_LDFLAGS = -L$(top_srcdir)/src -L$(top_srcdir)/qt -L$(top_srcdir)/gc/.libs -Wl,-R$(top_srcdir)/gc/.libs
LDADD =  -lplasma -lqt  -lgc -ldl

CXXLD = $(CXX)
LINK = $(CXXLD) $(AM_CXXFLAGS) $(CXXFLAGS) $(AM_LDFLAGS) $(LDFLAGS) -o $@

include $(top_srcdir)/tests/Makefile.rules
