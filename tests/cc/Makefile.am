check_PROGRAMS = mytest

mytest_SOURCES = \
	String.C \
	main.pa \
	CLexer.pa \
	clex.ll

mytest_DEPENDENCIES = $(DEPENDENCIES)

DEP_FILES=\
	./$(DEPDIR)/main.Po \
	./$(DEPDIR)/String.Po \
	./$(DEPDIR)/CLexer.Po \
	./$(DEPDIR)/clex.Po

include ./$(DEPDIR)/main.Po
include ./$(DEPDIR)/CLexer.Po

BUILT_SOURCES = clex.cc

mytest_CPPFLAGS = -I$(top_srcdir)/gc/include

mytest_LDFLAGS = -L$(top_srcdir)/gc -Wl,-R$(top_srcdir)/gc/.libs

mytest_LDADD =  -lgc -ldl

clex_lflag = -Cfe

%.cc : %.ll ;
	$(LEX) $($(patsubst %.cc,%_lflag,$(@))) -o$@ $(@:.cc=.ll)

include $(top_srcdir)/tests/Makefile.rules

# For some stupid reason, Automake thinks that this is a C++ project, so
# I have to fool it here.
CXXLINK = $(PLASMA) -o $@

CLEANFILES += clex.cc