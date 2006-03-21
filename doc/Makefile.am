
EXTRA_DIST = $(skribe_src) pipe2.pa ex1.pa

webdir = $(InfoHome)

web_DATA = index.html $(wildcard *.html)

# Only build a pdf if latex is good.
if LATEXOK
  web_DATA += index.pdf
endif

skribe_src = $(shell ls *.skb)

TESTDIR = $(top_srcdir)/tests

DistBase = distribution

DISTFILE = $(PACKAGE)-$(VERSION).tar.gz

FtpCompass = $(top_srcdir)/scripts/ftp-compass --param1=$(CompassParam1)

SKRIBEVARS = --eval "(define *test-dir* \"$(TESTDIR)\")" --eval "(define *web-url* \"$(WEBURL)\")" --eval "(define *dist-url* \"$(DISTURL)\")"

DOCCLEANFILES = *.sui *.html *.pdf *.dvi *.tex *.log *.aux *.out

INFO = info.xml

install:
	@echo "***"
	@echo "*** Documentation is not installed by default."
	@echo "*** Use the 'docinstall' target to install it."
	@echo "***"

docclean:
	rm -f $(DOCCLEANFILES)

# This installs just the documentation.
web-data-install:
	if [ $(CompassExists) ] ; then \
		$(FtpCompass) --folder=$(CompassPlasmaDir) $(web_DATA) ; \
	fi
	if [ ! -z $(webdir) -a -e $(webdir) ] ; then \
		echo " $(install_sh_DATA) $(INFO) $(webdir)/$(INFO)"; \
		$(install_sh_DATA) $(INFO) $(webdir)/$(INFO); \
	fi

# Create a distribution, then install it and all documentation.
docinstall: web-data-install all-am
	if [ $(CompassExists) ] ; then \
		(cd $(top_srcdir) && $(MAKE) dist ) && $(FtpCompass) --folder=$(CompassDistDir) $(top_srcdir)/$(DISTFILE) ; \
	fi


index.html: $(skribe_src) source-code.html

source-code.html: pipe2.pa

index.pdf: $(skribe_src)

%.html : %.skb ;
	$(SKRIBE) $(SKRIBEVARS) $< -o $@

%.tex : %.skb ;
	$(SKRIBE) $(SKRIBEVARS) $< -o $@

%.pdf : %.tex ;
	$(PDFLATEX) $<
