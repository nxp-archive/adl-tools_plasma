
EXTRA_DIST = $(skribe_src)

webdir = $(WEBHOME)

web_DATA = index.html

skribe_src = $(shell ls *.skb)

TESTDIR = $(top_srcdir)/tests

SKRIBEVARS = --eval "(define *test-dir* \"$(TESTDIR)\")"

DOCCLEANFILES = *.sui *.html

docclean:
	rm -f $(DOCCLEANFILES)

install: all-am
	$(mkinstalldirs) $(DESTDIR)$(webdir)
	@list=$$(ls *.html); for p in $$list; do \
	  if test -f "$$p"; then d=; else d="$(srcdir)/"; fi; \
	  f="`echo $$p | sed -e 's|^.*/||'`"; \
	  echo " $(install_sh_DATA) $$d$$p $(DESTDIR)$(webdir)/$$f"; \
	  $(install_sh_DATA) $$d$$p $(DESTDIR)$(webdir)/$$f; \
	done

index.html: $(skribe_src) source-code.html

%.html : %.skb ;
	$(SKRIBE) $(SKRIBEVARS) $< -o $@
