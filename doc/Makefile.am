
EXTRA_DIST = $(skribe_src)

webdir = $(WEBHOME)

skribe_src = $(shell ls *.skb)

web_DATA = \
	index.html

TESTDIR = $(top_srcdir)/tests

SKRIBEVARS = --eval "(define *test-dir* \"$(TESTDIR)\")"

CLEANFILES = *.sui

docclean:
	rm -f $(CLEANFILES)

index.html: $(skribe_src) source-code.html

%.html : %.skb ;
	$(SKRIBE) $(SKRIBEVARS) $< -o $@
