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
	par4 \
	par5 \
	par6 \
	par7 \
	par8 \
	par9 \
	qsort1 \
	qsort2 \
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
	chan12 \
	chan13 \
	chan14 \
	chan15 \
	chan16 \
	chan17 \
	spawn1 \
	spawn2 \
	spawn3 \
	spawn4 \
	mutex1 \
	pri1 \
	pri2 \
	pri3 \
	pri4 \
	pri5 \
	pri6 \
	pri7 \
	pri8 \
	proc1 \
	proc2 \
	proc3 \
	proc4 \
	proc5 \
	proc6 \
	proc7 \
	proc8 \
	proc9 \
	proc10 \
	time1 \
	time2 \
	time3 \
	time4 \
	time5 \
	rand1 \
	rand2 \
	clock1 \
	clock2 \
	clock3 \
	gc1

par1_SOURCES = par1.pa
par1_DEPENDENCIES = $(DEPENDENCIES)

par2_SOURCES = par2.pa
par2_DEPENDENCIES = $(DEPENDENCIES)

par3_SOURCES = par3.pa
par3_DEPENDENCIES = $(DEPENDENCIES)

par4_SOURCES = par4.pa
par4_DEPENDENCIES = $(DEPENDENCIES)

par5_SOURCES = par5.pa
par5_DEPENDENCIES = $(DEPENDENCIES)

par6_SOURCES = par6.pa
par6_DEPENDENCIES = $(DEPENDENCIES)

par7_SOURCES = par7.pa
par7_DEPENDENCIES = $(DEPENDENCIES)

par8_SOURCES = par8.pa
par8_DEPENDENCIES = $(DEPENDENCIES)

par9_SOURCES = par9.pa
par9_DEPENDENCIES = $(DEPENDENCIES)

qsort1_SOURCES = qsort1.pa
qsort1_DEPENDENCIES = $(DEPENDENCIES)

qsort2_SOURCES = qsort2.pa
qsort2_DEPENDENCIES = $(DEPENDENCIES)

chan1_SOURCES = chan1.C
chan1_DEPENDENCIES = $(DEPENDENCIES)
chan1_LDADD = $(CXX_LDADD)

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

chan12_SOURCES = chan12.pa
chan12_DEPENDENCIES = $(DEPENDENCIES)

chan13_SOURCES = chan13.pa
chan13_DEPENDENCIES = $(DEPENDENCIES)

chan14_SOURCES = chan14.pa
chan14_DEPENDENCIES = $(DEPENDENCIES)

chan15_SOURCES = chan15.pa
chan15_DEPENDENCIES = $(DEPENDENCIES)

chan16_SOURCES = chan16.pa
chan16_DEPENDENCIES = $(DEPENDENCIES)

chan17_SOURCES = chan17.pa
chan17_DEPENDENCIES = $(DEPENDENCIES)

spawn1_SOURCES = spawn1.C
spawn1_DEPENDENCIES = $(DEPENDENCIES)
spawn1_LDADD = $(CXX_LDADD)

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

pri5_SOURCES = pri5.pa
pri5_DEPENDENCIES = $(DEPENDENCIES)

pri6_SOURCES = pri6.pa
pri6_DEPENDENCIES = $(DEPENDENCIES)

pri7_SOURCES = pri7.pa
pri7_DEPENDENCIES = $(DEPENDENCIES)

pri8_SOURCES = pri8.pa
pri8_DEPENDENCIES = $(DEPENDENCIES)

proc1_SOURCES = proc1.C
proc1_DEPENDENCIES = $(DEPENDENCIES)
proc1_LDADD = $(CXX_LDADD)

proc2_SOURCES = proc2.pa
proc2_DEPENDENCIES = $(DEPENDENCIES)

proc3_SOURCES = proc3.pa
proc3_DEPENDENCIES = $(DEPENDENCIES)

proc4_SOURCES = proc4.pa
proc4_DEPENDENCIES = $(DEPENDENCIES)

proc5_SOURCES = proc5.pa
proc5_DEPENDENCIES = $(DEPENDENCIES)

proc6_SOURCES = proc6.pa
proc6_DEPENDENCIES = $(DEPENDENCIES)

proc7_SOURCES = proc7.pa
proc7_DEPENDENCIES = $(DEPENDENCIES)

proc8_SOURCES = proc8.pa
proc8_DEPENDENCIES = $(DEPENDENCIES)

proc9_SOURCES = proc9.pa
proc9_DEPENDENCIES = $(DEPENDENCIES)

proc10_SOURCES = proc10.pa
proc10_DEPENDENCIES = $(DEPENDENCIES)

time1_SOURCES = time1.pa
time1_DEPENDENCIES = $(DEPENDENCIES)

time2_SOURCES = time2.pa
time2_DEPENDENCIES = $(DEPENDENCIES)

time3_SOURCES = time3.pa
time3_DEPENDENCIES = $(DEPENDENCIES)

time4_SOURCES = time4.pa
time4_DEPENDENCIES = $(DEPENDENCIES)

time5_SOURCES = time5.pa
time5_DEPENDENCIES = $(DEPENDENCIES)

rand1_SOURCES = rand1.C
rand1_DEPENDENCIES = $(DEPENDENCIES)
rand1_LDADD = $(CXX_LDADD)

rand2_SOURCES = rand2.C
rand2_DEPENDENCIES = $(DEPENDENCIES)
rand2_LDADD = $(CXX_LDADD)

clock1_SOURCES = clock1.pa
clock1_DEPENDENCIES = $(DEPENDENCIES)

clock2_SOURCES = clock2.pa
clock2_DEPENDENCIES = $(DEPENDENCIES)

clock3_SOURCES = clock3.pa
clock3_DEPENDENCIES = $(DEPENDENCIES)

gc1_SOURCES = gc1.pa
gc1_DEPENDENCIES = $(DEPENDENCIES)

AM_CXXFLAGS = $(CXXFLAGS) -I$(top_srcdir)/src -I$(top_srcdir)/gc/include

DEP_FILES=\
	./$(DEPDIR)/par1.Po \
	./$(DEPDIR)/par2.Po \
	./$(DEPDIR)/par3.Po \
	./$(DEPDIR)/par4.Po \
	./$(DEPDIR)/par5.Po \
	./$(DEPDIR)/par6.Po \
	./$(DEPDIR)/par7.Po \
	./$(DEPDIR)/par8.Po \
	./$(DEPDIR)/par9.Po \
	./$(DEPDIR)/qsort1.Po \
	./$(DEPDIR)/qsort2.Po \
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
	./$(DEPDIR)/chan12.Po \
	./$(DEPDIR)/chan13.Po \
	./$(DEPDIR)/chan14.Po \
	./$(DEPDIR)/chan15.Po \
	./$(DEPDIR)/chan16.Po \
	./$(DEPDIR)/chan17.Po \
	./$(DEPDIR)/spawn1.Po \
	./$(DEPDIR)/spawn2.Po \
	./$(DEPDIR)/spawn3.Po \
	./$(DEPDIR)/spawn4.Po \
	./$(DEPDIR)/mutex1.Po \
	./$(DEPDIR)/pri1.Po \
	./$(DEPDIR)/pri2.Po \
	./$(DEPDIR)/pri3.Po \
	./$(DEPDIR)/pri4.Po \
	./$(DEPDIR)/pri5.Po \
	./$(DEPDIR)/pri6.Po \
	./$(DEPDIR)/pri7.Po \
	./$(DEPDIR)/pri8.Po \
	./$(DEPDIR)/proc1.Po \
	./$(DEPDIR)/proc2.Po \
	./$(DEPDIR)/proc3.Po \
	./$(DEPDIR)/proc4.Po \
	./$(DEPDIR)/proc5.Po \
	./$(DEPDIR)/proc6.Po \
	./$(DEPDIR)/proc7.Po \
	./$(DEPDIR)/proc8.Po \
	./$(DEPDIR)/proc9.Po \
	./$(DEPDIR)/proc10.Po \
	./$(DEPDIR)/time1.Po \
	./$(DEPDIR)/time2.Po \
	./$(DEPDIR)/time3.Po \
	./$(DEPDIR)/time4.Po \
	./$(DEPDIR)/time5.Po \
	./$(DEPDIR)/rand1.Po \
	./$(DEPDIR)/rand2.Po \
	./$(DEPDIR)/clock1.Po \
	./$(DEPDIR)/clock2.Po \
	./$(DEPDIR)/clock3.Po \
	./$(DEPDIR)/gc1.Po

include ./$(DEPDIR)/par1.Po
include ./$(DEPDIR)/par2.Po
include ./$(DEPDIR)/par3.Po
include ./$(DEPDIR)/par4.Po
include ./$(DEPDIR)/par5.Po
include ./$(DEPDIR)/par6.Po
include ./$(DEPDIR)/par7.Po
include ./$(DEPDIR)/par8.Po
include ./$(DEPDIR)/par9.Po
include ./$(DEPDIR)/qsort1.Po
include ./$(DEPDIR)/qsort2.Po
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
include ./$(DEPDIR)/chan12.Po
include ./$(DEPDIR)/chan13.Po
include ./$(DEPDIR)/chan14.Po
include ./$(DEPDIR)/chan15.Po
include ./$(DEPDIR)/chan16.Po
include ./$(DEPDIR)/chan17.Po
include ./$(DEPDIR)/spawn2.Po
include ./$(DEPDIR)/spawn3.Po
include ./$(DEPDIR)/spawn4.Po
include ./$(DEPDIR)/mutex1.Po
include ./$(DEPDIR)/pri1.Po
include ./$(DEPDIR)/pri2.Po
include ./$(DEPDIR)/pri3.Po
include ./$(DEPDIR)/pri4.Po
include ./$(DEPDIR)/pri5.Po
include ./$(DEPDIR)/pri6.Po
include ./$(DEPDIR)/pri7.Po
include ./$(DEPDIR)/pri8.Po
include ./$(DEPDIR)/proc2.Po
include ./$(DEPDIR)/proc3.Po
include ./$(DEPDIR)/proc4.Po
include ./$(DEPDIR)/proc5.Po
include ./$(DEPDIR)/proc6.Po
include ./$(DEPDIR)/proc7.Po
include ./$(DEPDIR)/proc8.Po
include ./$(DEPDIR)/proc9.Po
include ./$(DEPDIR)/proc10.Po
include ./$(DEPDIR)/time1.Po
include ./$(DEPDIR)/time2.Po
include ./$(DEPDIR)/time3.Po
include ./$(DEPDIR)/time4.Po
include ./$(DEPDIR)/time5.Po
include ./$(DEPDIR)/clock1.Po
include ./$(DEPDIR)/clock2.Po
include ./$(DEPDIR)/clock3.Po
include ./$(DEPDIR)/gc1.Po

include $(top_srcdir)/tests/Makefile.rules
