
EXTRA_DIST = $(skribe_src) pipe2.pa ex1.pa

webdir = $(WEBHOME)

web_DATA = index.html 

# Only build a pdf if latex is good.
if LATEXOK
  web_DATA += index.pdf
endif

skribe_src = $(shell ls *.skb)

TESTDIR = $(top_srcdir)/tests

DistBase = distribution
DistDir = $(webdir)/$(DistBase)
DistUrl = $(WEBURL)/$(DistBase)

DISTFILE = $(PACKAGE)-$(VERSION).tar.gz

SKRIBEVARS = --eval "(define *test-dir* \"$(TESTDIR)\")" --eval "(define *web-url* \"$(WEBURL)\")" --eval "(define *dist-url* \"$(DistBase)\")"

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
    ## Remove from the web directory any existing distributions.
	rm -f $(DESTDIR)/$(DistDir)/$(PACKAGE)*.tar.gz
	$(mkinstalldirs) $(DESTDIR)$(DistDir)
    ## Final step:  Make a distribution and copy it to the web directory.
	(cd $(top_srcdir) && $(MAKE) dist && cp $(DISTFILE) $(DESTDIR)$(DistDir) )


index.html: $(skribe_src) source-code.html

source-code.html: pipe2.pa

index.pdf: $(skribe_src)

%.html : %.skb ;
	$(SKRIBE) $(SKRIBEVARS) $< -o $@

%.tex : %.skb ;
	$(SKRIBE) $(SKRIBEVARS) $< -o $@

%.pdf : %.tex ;
	$(PDFLATEX) $<
