
TESTS = regress

check_PROGRAMS = par1 par2 par3 qsort chan1 chan2 chan3 chan4 chan5 chan6 chan7

par1_SOURCES = par1.pa

par2_SOURCES = par2.pa

par3_SOURCES = par3.pa

qsort_SOURCES = qsort.pa

chan1_SOURCES = chan1.C

chan2_SOURCES = chan2.pa

chan3_SOURCES = chan3.pa

chan4_SOURCES = chan4.pa

chan5_SOURCES = chan5.pa

chan6_SOURCES = chan6.pa

chan7_SOURCES = chan7.pa

chan1_CXXFLAGS = -I$(top_srcdir)/src

LDADD = -L$(top_srcdir)/src -lplasma -L$(top_srcdir)/qt -lqt

CXXLD = $(CXX)
LINK = $(CXXLD) $(AM_CXXFLAGS) $(CXXFLAGS) $(AM_LDFLAGS) $(LDFLAGS) -o $@

include $(top_srcdir)/tests/Makefile.rules
