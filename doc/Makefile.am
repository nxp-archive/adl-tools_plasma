#
# For now, building of documentation always occurs within the original source
# directory, even if the project is configured in a separate build directory.
#

EXTRA_DIST = $(skribe_src) pipe2.pa ex1.pa

webdir = $(InfoHome)

web_DATA = $(srcdir)/index.html $(wildcard $(srcdir)/*.html)

# Only build a pdf if latex is good.
if LATEXOK
  web_DATA += index.pdf
endif

skribe_src = $(wildcard $(srcdir)/*.skb)

TESTDIR = $(top_srcdir)/tests

DistBase = distribution

DISTFILE = $(PACKAGE)-$(VERSION).tar.gz

FtpCompass = $(top_srcdir)/scripts/ftp-compass --param1=$(CompassParam1)

SKRIBEVARS = -I $(srcdir) --eval "(define *test-dir* \"$(TESTDIR)\")" --eval "(define *web-url* \"$(WEBURL)\")" --eval "(define *dist-url* \"$(DISTURL)\")"

DOCCLEANFILES = $(srcdir)/*.sui $(srcdir)/*.html $(srcdir)/*.pdf $(srcdir)/*.dvi \
				$(srcdir)/*.tex $(srcdir)/*.log $(srcdir)/*.aux $(srcdir)/*.out

INFO = $(srcdir)/info.xml

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
		(cd $(top_builddir) && $(MAKE) dist ) && $(FtpCompass) --folder=$(CompassDistDir) $(top_builddir)/$(DISTFILE) ; \
	fi


$(srcdir)/index.html: $(skribe_src) $(srcdir)/source-code.html

$(srcdir)/source-code.html: $(srcdir)/pipe2.pa

$(srcdir)/index.pdf: $(skribe_src)

%.html : %.skb ;
	$(SKRIBE) $(SKRIBEVARS) $< -o $@

%.tex : %.skb ;
	$(SKRIBE) $(SKRIBEVARS) $< -o $@

%.pdf : %.tex ;
	$(PDFLATEX) $<
