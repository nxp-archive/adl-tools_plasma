
SUBDIRS = md

lib_LTLIBRARIES = libqt.la libstp.la

libqt_la_SOURCES = \
	qt.c \
	qtmdc.c \
	qtmds.S

libstp_la_SOURCES = \
	stp.c

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
	./config clean
