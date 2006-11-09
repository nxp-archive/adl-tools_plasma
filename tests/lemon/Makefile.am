noinst_PROGRAMS = lemon

lemon_SOURCES = lemon.c

LEMPAR = lempar.c

EXTRA_DIST = $(LEMPAR)

all: all-am
	if [ ! -e $(LEMPAR) ] ; then \
	  cp $(srcdir)/$(LEMPAR) . ;\
	fi
