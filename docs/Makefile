# Emacs, this is -*- Mode: makefile -*-
#
# Makefile [18. Nov 2001]
# Makefile for the Ayam reference manual

all: htm pdf3

# do not try to be smart and rename this target to html
# as then the date-stamp comparing mechanism of make stumbles
# over the existing "html" directory!
htm:
	rm -rf html
	mkdir html
	cd html; ln -sf ../pics pics;cd ..
	cp ayam.sgml html
	cd html;sgml2html -s 1 ayam.sgml;rm ayam.sgml;cd ..
	./fixhtml.tcsh

txt:
	sgml2txt -f ayam.sgml

ps:
	/bin/sh -c "sed -f sedsrc ayam.sgml > ayaml.sgml"
	sgml2latex -o ps ayaml.sgml
	mv ayaml.ps ayam.ps
	rm -f ayaml.sgml

pdf:
	/bin/sh -c "sed -f sedsrc ayam.sgml > ayaml.sgml"
	sgml2latex -o dvi ayaml.sgml
	dvi2pdf ayaml.dvi
	mv ayaml.pdf ayam.pdf
	rm -f ayaml.sgml ayaml.dvi

pdf2:
	/bin/sh -c "sed -f sedsrc ayam.sgml > ayaml.sgml"
	sgml2latex -o tex ayaml.sgml
	/bin/sh -c "sed -f sedsrc2 ayaml.tex > ayam.tex"
	/bin/sh -c "sed -f sedsrc3 ayam.tex > ayaml.tex"
	mv ayaml.tex ayam.tex
	pdflatex ayam.tex
	rm -f ayaml.sgml


pdf3: pdf2
	pdflatex ayam.tex
	pdflatex ayam.tex
	rm ayam.tex ayam.toc ayam.log ayam.aux

clean:
	rm -rf html ayam.tex ayam.dvi ayam.txt ayam.pdf ayam.ps
	rm -rf ayaml.sgml ayaml.tex ayaml.dvi ayaml.pdf ayaml.ps
