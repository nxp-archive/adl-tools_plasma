
EXTRA_DIST = $(skribe_src) pipe2.pa

WEBURL=http://www.somerset.sps.mot.com/projects/tools/plasma/

webdir = $(WEBHOME)

web_DATA = index.html 

# Only build a pdf if latex is good.
if LATEXOK
  web_DATA += index.pdf
endif

skribe_src = $(shell ls *.skb)

TESTDIR = $(top_srcdir)/tests

SKRIBEVARS = --eval "(define *test-dir* \"$(TESTDIR)\")" --eval "(define *web-url* \"$(WEBURL)\")"

DOCCLEANFILES = *.sui *.html *.pdf *.dvi *.tex *.log *.aux *.out

INFO = info.xml

install:
	@echo "***"
	@echo "*** Documentation is not installed by default."
	@echo "*** Use the 'docinstall' target to install it."
	@echo "***"

docclean:
	rm -f $(DOCCLEANFILES)

docinstall: all-am
	$(mkinstalldirs) $(DESTDIR)$(webdir)
	@list=$$(ls *.html *.pdf); for p in $$list; do \
	  if test -f "$$p"; then d=; else d="$(srcdir)/"; fi; \
	  f="`echo $$p | sed -e 's|^.*/||'`"; \
	  echo " $(install_sh_DATA) $$d$$p $(DESTDIR)$(webdir)/$$f"; \
	  $(install_sh_DATA) $$d$$p $(DESTDIR)$(webdir)/$$f; \
	done
	echo " $(install_sh_DATA) $(INFO) $(DESTDIR)$(webdir)/$(INFO)";
	$(install_sh_DATA) $(INFO) $(DESTDIR)$(webdir)/$(INFO);

index.html: $(skribe_src) source-code.html

source-code.html: pipe2.pa

index.pdf: $(skribe_src)

%.html : %.skb ;
	$(SKRIBE) $(SKRIBEVARS) $< -o $@

%.tex : %.skb ;
	$(SKRIBE) $(SKRIBEVARS) $< -o $@

%.pdf : %.tex ;
	$(PDFLATEX) $<
