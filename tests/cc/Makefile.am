
TESTS = regress

check_PROGRAMS = minicc

minicc_SOURCES = \
	String.C \
	Node.C \
	main.pa \
	CParser.pa \
	cparse.yy \
	CLexer.pa \
	clex.ll

minicc_DEPENDENCIES = $(DEPENDENCIES)

DEP_FILES=\
	./$(DEPDIR)/main.Po \
	./$(DEPDIR)/String.Po \
	./$(DEPDIR)/CLexer.Po \
	./$(DEPDIR)/CParser.Po \
	./$(DEPDIR)/cparse.Po \
	./$(DEPDIR)/clex.Po \
	./$(DEPDIR)/Node.Po

include ./$(DEPDIR)/main.Po
include ./$(DEPDIR)/CLexer.Po
include ./$(DEPDIR)/CParser.Po

BUILT_SOURCES = clex.cc

LEMON = ../lemon/lemon

AM_CXXFLAGS = -I$(top_srcdir)/gc/include

minicc_LDFLAGS = -L$(top_srcdir)/gc -Wl,-R$(top_srcdir)/gc/.libs

minicc_LDADD =  -lgc -ldl

clex_lflag = -Cfe

%.cc : %.ll ;
	$(LEX) $($(patsubst %.cc,%_lflag,$(@))) -o$@ $(@:.cc=.ll)

%.cc : %.yy ;
	$(LEMON) $<

cparse.h: cparse.cc
CLexer.o: cparse.h
clex.o: cparse.h

include $(top_srcdir)/tests/Makefile.rules

# For some stupid reason, Automake thinks that this is a C++ project, so
# I have to fool it here.
CXXLINK = $(PLASMA) -o $@

CLEANFILES += clex.cc cparse.cc cparse.h
