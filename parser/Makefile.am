
pkglib_PROGRAMS = Plasma.so Wrapper.so Mutex.so TMutex.so

Plasma_so_SOURCES = \
	Plasma.mc \
	VarWalker.mc

Wrapper_so_SOURCES = \
	Wrapper.mc

Mutex_so_SOURCES = \
	Mutex.mc

TMutex_so_SOURCES = \
	TMutex.mc

noinst_HEADERS = \
	VarWalker.h \
	Wrapper.h \
	Mutex.h

CLEANFILES= *.ii *-init.cc *.so

CXXLINK = $(CXX) $(AM_CXXFLAGS) $(CXXFLAGS) $(AM_LDFLAGS) $(LDFLAGS)

am__depfiles_maybe = depfiles

DEP_FILES = \
	./$(DEPDIR)/Plasma.Po \
	./$(DEPDIR)/VarWalker.Po \
	./$(DEPDIR)/Wrapper.Po \
	./$(DEPDIR)/Mutex.Po \
	./$(DEPDIR)/TMutex.Po

include ./$(DEPDIR)/Plasma.Po
include ./$(DEPDIR)/VarWalker.Po
include ./$(DEPDIR)/Wrapper.Po
include ./$(DEPDIR)/Mutex.Po
include ./$(DEPDIR)/TMutex.Po

.mc.o:
	if $(OCC) -m$@ $(OCCFLAGS) $< -c -d"-MT $@" -d-MD -d-MP -d"-MF$(DEPDIR)/$*.Tpo" -- -Wall -g ;\
	then mv "$(DEPDIR)/$*.Tpo" "$(DEPDIR)/$*.Po" ; \
	else rm -f "$(DEPDIR)/$*.Tpo"; exit 1; \
	fi	

Plasma.so: $(Plasma_so_OBJECTS)
	$(CXXLINK) -shared -o $@ $(Plasma_so_OBJECTS) $(Plasma_so_LDFLAGS) $(Plasma_so_LDADD)

Wrapper.so: $(Wrapper_so_OBJECTS)
	$(CXXLINK) -shared -o $@ $(Wrapper_so_OBJECTS) $(Wrapper_so_LDFLAGS) $(Wrapper_so_LDADD)

Mutex.so: $(Mutex_so_OBJECTS)
	$(CXXLINK) -shared -o $@ $(Mutex_so_OBJECTS) $(Mutex_so_LDFLAGS) $(Mutex_so_LDADD)

TMutex.so: $(TMutex_so_OBJECTS)
	$(CXXLINK) -shared -o $@ $(TMutex_so_OBJECTS) $(TMutex_so_LDFLAGS) $(TMutex_so_LDADD)
