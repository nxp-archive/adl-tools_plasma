lib_LTLIBRARIES = libqt.la libstp.la

libqt_la_SOURCES = \
	qt.c \
	qtmdc.c \
	qtmds.S

libstp_la_SOURCES = \
	stp.c

noinst_HEADERS = \
	qt.h \
	qtmd.h

#libqt_la_LIBADD = qtmds.o

#qtmds.o: qtmds.s

# Remove the configuration information on a distclean.
distclean: distclean-am
	./config clean
