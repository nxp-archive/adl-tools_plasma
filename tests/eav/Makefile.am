#
# EAV build.
#

TESTS = regress

check_PROGRAMS = \
	eav

eav_SOURCES = eav.pa 
eav_DEPENDENCIES = $(DEPENDENCIES)

AM_CXXFLAGS = $(CXXFLAGS) -I$(top_srcdir)/src -I$(top_srcdir)/gc/include

DEP_FILES=\
	./$(DEPDIR)/eav.Po

include ./$(DEPDIR)/eav.Po

include $(top_srcdir)/tests/Makefile.rules
