
pkglib_PROGRAMS = Plasma.so

Plasma_so_SOURCES = \
	plasma.mc

CLEANFILES= *.ii *-init.cc *.so

Plasma.so: plasma.mc
	$(OCC) -m$@ $(OCCFLAGS) $< -- -Wall -g
