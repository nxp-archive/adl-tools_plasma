
noinst_DATA = Plasma_Intro.pdf

Plasma_Intro.pdf : Plasma_Intro.sxw

CLEANFILES = *.aux *.bib *.blg *.log *.out *.toc *.pdf

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