
TESTS = regress

check_PROGRAMS = \
	par1 \
	par2 \
	par3 \
	qsort \
	chan1 \
	chan2 \
	chan3 \
	chan4 \
	chan5 \
	chan6 \
	chan7 \
	spawn1 \
	spawn2 \
	spawn3 \
	spawn4

DEPENDENCIES = $(top_srcdir)/src/libplasma.a $(top_srcdir)/parser/Plasma.so

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

spawn1_SOURCES = spawn1.C

spawn2_SOURCES = spawn2.pa

spawn3_SOURCES = spawn3.pa

spawn4_SOURCES = spawn4.pa

AM_CXXFLAGS = $(CXXFLAGS) -I$(top_srcdir)/src

DEP_FILES=\
	./$(DEPDIR)/par1.Po \
	./$(DEPDIR)/par2.Po \
	./$(DEPDIR)/par3.Po \
	./$(DEPDIR)/qsort.Po \
	./$(DEPDIR)/chan1.Po \
	./$(DEPDIR)/chan2.Po \
	./$(DEPDIR)/chan3.Po \
	./$(DEPDIR)/chan4.Po \
	./$(DEPDIR)/chan5.Po \
	./$(DEPDIR)/chan6.Po \
	./$(DEPDIR)/chan7.Po \
	./$(DEPDIR)/spawn1.Po \
	./$(DEPDIR)/spawn2.Po \
	./$(DEPDIR)/spawn3.Po \
	./$(DEPDIR)/spawn4.Po

include ./$(DEPDIR)/par1.Po
include ./$(DEPDIR)/par2.Po
include ./$(DEPDIR)/par3.Po
include ./$(DEPDIR)/qsort.Po
include ./$(DEPDIR)/chan2.Po
include ./$(DEPDIR)/chan3.Po
include ./$(DEPDIR)/chan4.Po
include ./$(DEPDIR)/chan5.Po
include ./$(DEPDIR)/chan6.Po
include ./$(DEPDIR)/chan7.Po
include ./$(DEPDIR)/spawn2.Po
include ./$(DEPDIR)/spawn3.Po
include ./$(DEPDIR)/spawn4.Po

LDADD = -L$(top_srcdir)/src -lplasma -L$(top_srcdir)/qt -lqt

CXXLD = $(CXX)
LINK = $(CXXLD) $(AM_CXXFLAGS) $(CXXFLAGS) $(AM_LDFLAGS) $(LDFLAGS) -o $@

include $(top_srcdir)/tests/Makefile.rules
