#
# EAV build.
#

TESTS = eav

check_PROGRAMS = \
	eav

DEPENDENCIES = $(top_srcdir)/src/libplasma.a

eav_SOURCES = Random.C eav.pa 
eav_DEPENDENCIES = $(DEPENDENCIES)

AM_CXXFLAGS = $(CXXFLAGS) -I$(top_srcdir)/src -I$(top_srcdir)/gc/include

DEP_FILES=\
	./$(DEPDIR)/eav.Po \
	./$(DEPDIR)/Random.Po

include ./$(DEPDIR)/eav.Po

AM_LDFLAGS = -L$(top_srcdir)/src -L$(top_srcdir)/qt -L$(top_srcdir)/gc/.libs -Wl,-R$(top_srcdir)/gc/.libs
LDADD =  -lplasma -lqt  -lgc -ldl

CXXLD = $(CXX)
LINK = $(CXXLD) $(AM_CXXFLAGS) $(CXXFLAGS) $(AM_LDFLAGS) $(LDFLAGS) -o $@

include $(top_srcdir)/tests/Makefile.rules
