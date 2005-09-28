
noinst_DATA = Plasma_Intro.pdf

Plasma_Intro.pdf : Plasma_Intro.sxw

EXTRA_DIST = Plasma_Intro.sxw Plasma_MTV_2005.lyx

CLEANFILES = *.aux *.bib *.blg *.log *.out *.toc *.pdf

W2LFLAGS = -config $(srcdir)/article.xml

%.tex: %.sxw
	$(W2L) $(W2LFLAGS) $<

%.pdf: %.rst
	tfile=`echo "$<" | sed -e s/\.rst/\.tex/` && \
	$(TRIP) $(TFLAGS)  -w latex "$<" > $$tfile
	tfile=`echo "$<" | sed -e s/\.rst/\.tex/` &&\
	pdflatex $$tfile

%.bbl: %.tex
	$(PDFLATEX) $*

%.pdf: %.bbl
	$(BIBTEX) $*
	$(PDFLATEX) $*
	$(PDFLATEX) $*
