#
# ECU build.
#

TESTS = regress

check_PROGRAMS = \
	ecu

ecu_SOURCES = \
	ecu.pa \
	configuration.pa \
	engine.pa \
	car.pa \
	utilities.pa \
	angle.pa

ecu_DEPENDENCIES = $(DEPENDENCIES)

DEP_FILES=\
	./$(DEPDIR)/ecu.Po \
	./$(DEPDIR)/configuration.Po \
	./$(DEPDIR)/engine.Po \
	./$(DEPDIR)/car.Po \
	./$(DEPDIR)/utilities.Po \
	./$(DEPDIR)/angle.Po

include ./$(DEPDIR)/ecu.Po
include ./$(DEPDIR)/configuration.Po
include ./$(DEPDIR)/engine.Po
include ./$(DEPDIR)/car.Po
include ./$(DEPDIR)/utilities.Po
include ./$(DEPDIR)/angle.Po

include $(top_srcdir)/tests/Makefile.rules

CLEANFILES+=LogFile