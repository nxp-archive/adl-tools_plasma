lib_LTLIBRARIES = libplasma.la

libplasma_la_CPPFLAGS= -I$(top_srcdir)/qt -I$(top_srcdir)/gc

libplasma_la_SOURCES = \
	Interface.C \
	Init.C \
	Cluster.C \
	Proc.C \
	System.C \
	Queue.C \
	ProcQ.C \
	Thread.C \
	Random.C \
	LcgRand.C \
	KissRand.C \
	MtRand.C \
	Energy.C \
	ChanSupport.C

pkginclude_HEADERS = \
	plasma-interface.h \
	plasma.h \
	Interface.h \
	ChanSupport.h \
	Random.h \
	LcgRand.h \
	KissRand.h \
	MtRand.h \
	Quantity.h \
	Energy.h

noinst_HEADERS = \
	Machine.h  \
	Cluster.h  \
	Proc.h \
	System.h \
	Thread.h \
	Queue.h \
	ThreadQ.h \
	ProcQ.h
