
SUBDIRS = md

# For some reason, this doesn't build on i686 platforms.
#noinst_PROGRAMS = meas

lib_LTLIBRARIES = libqt.la libstp.la

libqt_la_SOURCES = \
	qt.c \
	qtmdc.c \
	qtmds.S

libstp_la_SOURCES = \
	stp.c

meas_SOURCES = \
	meas.c \
	qtmdb.S \
	qtmdb_c.c

meas_LDADD = \
	libstp.la \
	libqt.la

noinst_HEADERS = \
	qt.h \
	qtmd.h \
	stp.h

EXTRA_DIST = 	CHANGES \
		INSTALL \
		README \
		README.MISC \
		README.PORT \
		b.h \
		config \
		copyright.h \
		meas.c


# Remove the configuration information on a distclean.
distclean: distclean-am
	(cd $(srcdir) && ./config clean)

LINK = $(LIBTOOL) --mode=link $(CXX) -o $@
