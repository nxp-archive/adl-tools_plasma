

SUBDIRS = macros scripts qt gc libltdl opencxx src parser tests $(DOCDIR)

AUTOMAKE_OPTIONS = foreign

# This ensures that aclocal will be run correctly if we touch
# configure.in, then type "make".  What happens is that configure
# captures the state of the variable and substitutes it into the
# final Makefile.
ACLOCAL_AMFLAGS = -I ./macros -I ./libltdl
