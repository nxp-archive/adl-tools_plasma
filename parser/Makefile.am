
OccBuildDir = $(top_builddir)/opencxx
GcBuildDir = $(top_builddir)/gc
GcSrcDir = $(top_srcdir)/gc

OCC = $(OccBuildDir)/occ
OCCFLAGS = -I$(top_srcdir) -I$(GcSrcDir)

bin_PROGRAMS = plasma-comp

plasma_comp_SOURCES = \
	Plasma.mc \
	VarWalker.mc \
	Wrapper.mc \
	Mutex.mc \
	TMutex.mc

plasma_comp_LDADD = \
	$(OccBuildDir)/libocc.la $(LIBLTDL)

plasma_comp_LINK = $(LIBTOOL) --mode=link $(CXX) -o $@

noinst_HEADERS = \
	Plasma.h \
	VarWalker.h \
	Wrapper.h \
	Mutex.h \
	LdLeaf.h

CLEANFILES= *.ii *-init.cc *.so

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
	if $(OCC) --static-mc=$@ $(OCCFLAGS) $< -c -d"-MT $@" -d-MD -d-MP -d"-MF$(DEPDIR)/$*.Tpo" -- $(CXXFLAGS) ;\
	then mv "$(DEPDIR)/$*.Tpo" "$(DEPDIR)/$*.Po" ; \
	else rm -f "$(DEPDIR)/$*.Tpo"; exit 1; \
	fi	
