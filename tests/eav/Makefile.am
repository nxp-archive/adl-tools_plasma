#
# EAV build.
#

TESTS = regress

check_PROGRAMS = \
	eav

EXTRA_DIST = regress

eav_SOURCES = eav.pa 
eav_DEPENDENCIES = $(DEPENDENCIES)

DEP_FILES=\
	./$(DEPDIR)/eav.Po

include ./$(DEPDIR)/eav.Po

include $(top_srcdir)/tests/Makefile.rules
