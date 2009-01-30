

SUBDIRS = macros scripts qt gc libltdl opencxx src parser tests doc

AUTOMAKE_OPTIONS = foreign

# This ensures that aclocal will be run correctly if we touch
# configure.in, then type "make".  What happens is that configure
# captures the state of the variable and substitutes it into the
# final Makefile.
ACLOCAL_AMFLAGS = -I ./macros -I ./libltdl

# Install documentation (not done by 'all' target)
docinstall:
	cd doc && $(MAKE) docinstall

# I'll eventually get this to work, but for now we just ignore it.
distcleancheck:
	@:
