
noinst_DATA = Plasma_Intro.pdf Plasma_MTV_2006.pdf

Plasma_Intro.pdf : Plasma_Intro.sxw
Plasma_MTV_2006.pdf : Plasma_MTV_2006.sxw

EXTRA_DIST = $(wildcard *.sxw)

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