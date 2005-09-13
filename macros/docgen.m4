##
## Configures various documentation generation tools.  We expect 
## everything to be in the user's path.  The tools we look for are:
## skribe, pdflatex, bibtex, and trip
##
## Output:
##
## SKRIBE  : Path of the Skribe program.
## PDFLATEX: Path of the pdflatex program.
## BIBTEX  : Path of the bibtex program.
## TRIP    : Path of the restructured-text processing tool (trip). 
## W2L     : Path of the OpenOffice Writer -> Latex converter
##
## LATEXOK : AM_CONDITIONAL set if Latex can be used.
## RSTOK   : AM_CONDITIONAL set if trip can be used.
## SKRIBEOK: AM_CONDITIONAL set if skribe can be used.
## W2LOK   : AM_CONDITIONAL set if w2l can be used.
##
AC_DEFUN([AM_DOCS],
[

AC_PATH_PROG(SKRIBE,skribe)
AC_PATH_PROG(BIBTEX,bibtex,)
AC_PATH_PROG(PDFLATEX,pdflatex)
AC_PATH_PROG(TRIP,trip)
AC_CHECK_PROG([W2L],[w2l],[w2l])

if [[ x$PDFLATEX != x ]] ; then
## Make sure that our latex is good- it has to have the most
## recent version of hyperref.
AC_MSG_CHECKING([that latex is good])
latextest=latex-test.tex
rm -f $latextest
cat >$latextest <<_LATEXEND
\documentclass{book}
\usepackage{color}
\usepackage[[setpagesize=false]]{hyperref}
\usepackage{epsfig}


%% colors
\definecolor{c1033}{rgb}{0.0,0.81176470588235,0.0}
\definecolor{c1034}{rgb}{1.0,0.0,0.0}
\definecolor{c1035}{rgb}{0.67843137254902,0.26274509803922,0.52549019607843}
\definecolor{c1036}{rgb}{0.098039215686275,0.098039215686275,0.68627450980392}
\definecolor{c1037}{rgb}{0.41176470588235,0.34901960784314,0.81176470588235}
\definecolor{c1038}{rgb}{1.0,0.65098039215686,0.0}


\newdimen\oldframetabcolsep
\newdimen\oldcolortabcolsep

\title{The Plasma Language}
\begin{document}
\date{}
\maketitle
\noindent Sample text.\par


%% First-chapter
\chapter{First chapter}
\label{First-chapter}
\noindent Chapter text.\par

\end{document}
_LATEXEND

if $PDFLATEX -interaction=nonstopmode $latextest > /dev/null ; then
  AC_MSG_RESULT([ok])
else
  AC_WARN([no.  You need to get the most recent version of the hyperref package.])
  PDFLATEX=
fi
rm -f latex-test.*
fi

AM_CONDITIONAL(LATEXOK, test x$PDFLATEX != x -a x$BIBTEX != x)
AM_CONDITIONAL(SKRIBEOK, test x$SKRIBE != x)
AM_CONDITIONAL(RSTOK, test x$TRIP != x)
AM_CONDITIONAL(W2LOK, test x$W2L != x)

AC_SUBST(PDFLATEX)
AC_SUBST(BIBTEX)
AC_SUBST(SKRIBE)
AC_SUBST(TRIP)
AC_SUBST(W2L)

])
