
pkglib_PROGRAMS = Plasma.so

Plasma_so_SOURCES = \
	plasma.mc \
	VarWalker.mc

noinst_HEADERS = \
	VarWalker.h

CLEANFILES= *.ii *-init.cc *.so

CXXLINK = $(CXX) $(AM_CXXFLAGS) $(CXXFLAGS) $(AM_LDFLAGS) $(LDFLAGS)

.mc.o:
	$(OCC) -m$@ $(OCCFLAGS) $< -c -- -Wall -g

Plasma.so: $(Plasma_so_OBJECTS)
	$(CXXLINK) -shared -o $@ $(Plasma_so_OBJECTS) $(Plasma_so_LDFLAGS) $(Plasma_so_LDADD)
