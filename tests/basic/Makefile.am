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
	chan8 \
	chan9 \
	chan10 \
	chan11 \
	spawn1 \
	spawn2 \
	spawn3 \
	spawn4 \
	mutex1 \
	pri1 \
	pri2 \
	pri3 \
	pri4 \
	proc1 \
	proc2 \
	proc3 \
	time1 \
	time2 \
	time3 \
	time4

DEPENDENCIES = $(top_srcdir)/src/libplasma.a

par1_SOURCES = par1.pa
par1_DEPENDENCIES = $(DEPENDENCIES)

par2_SOURCES = par2.pa
par2_DEPENDENCIES = $(DEPENDENCIES)

par3_SOURCES = par3.pa
par3_DEPENDENCIES = $(DEPENDENCIES)

qsort_SOURCES = qsort.pa
qsort_DEPENDENCIES = $(DEPENDENCIES)

chan1_SOURCES = chan1.C
chan1_DEPENDENCIES = $(DEPENDENCIES)

chan2_SOURCES = chan2.pa
chan2_DEPENDENCIES = $(DEPENDENCIES)

chan3_SOURCES = chan3.pa
chan3_DEPENDENCIES = $(DEPENDENCIES)

chan4_SOURCES = chan4.pa
chan4_DEPENDENCIES = $(DEPENDENCIES)

chan5_SOURCES = chan5.pa
chan5_DEPENDENCIES = $(DEPENDENCIES)

chan6_SOURCES = chan6.pa
chan6_DEPENDENCIES = $(DEPENDENCIES)

chan7_SOURCES = chan7.pa
chan7_DEPENDENCIES = $(DEPENDENCIES)

chan8_SOURCES = chan8.pa
chan8_DEPENDENCIES = $(DEPENDENCIES)

chan9_SOURCES = chan9.pa
chan9_DEPENDENCIES = $(DEPENDENCIES)

chan10_SOURCES = chan10.pa
chan10_DEPENDENCIES = $(DEPENDENCIES)

chan11_SOURCES = chan11.pa
chan11_DEPENDENCIES = $(DEPENDENCIES)

spawn1_SOURCES = spawn1.C
spawn1_DEPENDENCIES = $(DEPENDENCIES)

spawn2_SOURCES = spawn2.pa
spawn2_DEPENDENCIES = $(DEPENDENCIES)

spawn3_SOURCES = spawn3.pa
spawn3_DEPENDENCIES = $(DEPENDENCIES)

spawn4_SOURCES = spawn4.pa
spawn4_DEPENDENCIES = $(DEPENDENCIES)

mutex1_SOURCES = mutex1.pa
mutex1_DEPENDENCIES = $(DEPENDENCIES)

pri1_SOURCES = pri1.pa
pri1_DEPENDENCIES = $(DEPENDENCIES)

pri2_SOURCES = pri2.pa
pri2_DEPENDENCIES = $(DEPENDENCIES)

pri3_SOURCES = pri3.pa
pri3_DEPENDENCIES = $(DEPENDENCIES)

pri4_SOURCES = pri4.pa
pri4_DEPENDENCIES = $(DEPENDENCIES)

proc1_SOURCES = proc1.C
proc1_DEPENDENCIES = $(DEPENDENCIES)

proc2_SOURCES = proc2.pa
proc2_DEPENDENCIES = $(DEPENDENCIES)

proc3_SOURCES = proc3.pa
proc3_DEPENDENCIES = $(DEPENDENCIES)

time1_SOURCES = time1.pa
time1_DEPENDENCIES = $(DEPENDENCIES)

time2_SOURCES = time2.pa
time2_DEPENDENCIES = $(DEPENDENCIES)

time3_SOURCES = time3.pa
time3_DEPENDENCIES = $(DEPENDENCIES)

time4_SOURCES = time4.pa
time4_DEPENDENCIES = $(DEPENDENCIES)

AM_CXXFLAGS = $(CXXFLAGS) -I$(top_srcdir)/src -I$(top_srcdir)/gc/include

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
	./$(DEPDIR)/chan8.Po \
	./$(DEPDIR)/chan9.Po \
	./$(DEPDIR)/chan10.Po \
	./$(DEPDIR)/chan11.Po \
	./$(DEPDIR)/spawn1.Po \
	./$(DEPDIR)/spawn2.Po \
	./$(DEPDIR)/spawn3.Po \
	./$(DEPDIR)/spawn4.Po \
	./$(DEPDIR)/mutex1.Po \
	./$(DEPDIR)/pri1.Po \
	./$(DEPDIR)/pri2.Po \
	./$(DEPDIR)/pri3.Po \
	./$(DEPDIR)/pri4.Po \
	./$(DEPDIR)/proc1.Po \
	./$(DEPDIR)/proc2.Po \
	./$(DEPDIR)/proc3.Po \
	./$(DEPDIR)/time1.Po \
	./$(DEPDIR)/time2.Po \
	./$(DEPDIR)/time3.Po \
	./$(DEPDIR)/time4.Po

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
include ./$(DEPDIR)/chan8.Po
include ./$(DEPDIR)/chan9.Po
include ./$(DEPDIR)/chan10.Po
include ./$(DEPDIR)/chan11.Po
include ./$(DEPDIR)/spawn2.Po
include ./$(DEPDIR)/spawn3.Po
include ./$(DEPDIR)/spawn4.Po
include ./$(DEPDIR)/mutex1.Po
include ./$(DEPDIR)/pri1.Po
include ./$(DEPDIR)/pri2.Po
include ./$(DEPDIR)/pri3.Po
include ./$(DEPDIR)/pri4.Po
include ./$(DEPDIR)/proc2.Po
include ./$(DEPDIR)/proc3.Po
include ./$(DEPDIR)/time1.Po
include ./$(DEPDIR)/time2.Po
include ./$(DEPDIR)/time3.Po
include ./$(DEPDIR)/time4.Po

LDADD = -L$(top_srcdir)/src -lplasma -L$(top_srcdir)/qt -lqt -L$(top_srcdir)/gc/.libs -Wl,-R$(top_srcdir)/gc/.libs -lgc -ldl

CXXLD = $(CXX)
LINK = $(CXXLD) $(AM_CXXFLAGS) $(CXXFLAGS) $(AM_LDFLAGS) $(LDFLAGS) -o $@

include $(top_srcdir)/tests/Makefile.rules
