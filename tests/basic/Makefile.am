
TESTS = regress

check_PROGRAMS = par1

par1_SOURCES = par1.pa

LDADD = -L$(top_srcdir)/src -lplasma -L$(top_srcdir)/qt -lqt

CXXLD = $(CXX)
LINK = $(CXXLD) $(AM_CXXFLAGS) $(CXXFLAGS) $(AM_LDFLAGS) $(LDFLAGS) -o $@

include $(top_srcdir)/tests/Makefile.rules
