# Emacs, this is -*- Mode: makefile -*-
#
# Makefile [16. Jul 2002]
# Makefile for the first Ayam tutorial

all: htm pdf3

# do not try to be smart and rename this target to html
# as then the date-stamp comparing mechanism of make stumbles
# over the existing "html" directory!
htm:
	rm -rf html
	mkdir html
	cd html; ln -sf ../pics pics;cd ..
	cp tutorial1.sgml html
	cd html;sgml2html -s 1 tutorial1.sgml;rm tutorial1.sgml;cd ..
	./fixhtml.tcsh

txt:
	sgml2txt -f tutorial1.sgml

ps:
	/bin/sh -c "sed -f sedsrc tutorial1.sgml > tutorial1l.sgml"
	sgml2latex -o ps tutorial1l.sgml
	mv tutorial1l.ps tutorial1.ps
	rm -f tutorial1l.sgml

pdf:
	/bin/sh -c "sed -f sedsrc tutorial1.sgml > tutorial1l.sgml"
	sgml2latex -o dvi tutorial1l.sgml
	dvi2pdf tutorial1l.dvi
	mv tutorial1l.pdf tutorial1.pdf
	rm -f tutorial1l.sgml tutorial1l.dvi

pdf2:
	/bin/sh -c "sed -f sedsrc tutorial1.sgml > tutorial1l.sgml"
	sgml2latex -o tex tutorial1l.sgml
	/bin/sh -c "sed -f sedsrc2 tutorial1l.tex > tutorial1.tex"
	/bin/sh -c "sed -f sedsrc3 tutorial1.tex > tutorial1l.tex"
	mv tutorial1l.tex tutorial1.tex
	pdflatex tutorial1.tex
	rm -f tutorial1l.sgml


pdf3: pdf2
	pdflatex tutorial1.tex
	pdflatex tutorial1.tex
	rm tutorial1.tex tutorial1.log tutorial1.aux

clean:
	rm -f *.html *.txt html/*.html tutorial1l.tex tutorial1.tex

