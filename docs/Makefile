# Emacs, this is -*- Mode: makefile -*-
#
# Makefile [18. Mar 2009]
# Makefile for the Ayam reference manual

# on Debian3.0 (Woody) the pdf3 target does not work anymore,
# use pdf5 instead

all: html pdf5


html:
	rm -rf html
	mkdir html
	cd html; ln -sf ../pics pics;cd ..
	cp ayam.sgml html
	cd html;sgml2html -D out=html -s 1 ayam.sgml;rm ayam.sgml;cd ..
	/usr/bin/tclsh fixhtml.tcl
	-cd html;../corrwh.pl;cd ..

txt:
	sgml2txt -D out=txt -f ayam.sgml

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
	pdflatex ayam.tex > ga
	rm -f ayaml.sgml


pdf3: pdf2
	pdflatex ayam.tex > ga
	pdflatex ayam.tex > ga
	rm ayam.tex ayam.toc ayam.log ayam.aux


pdf4:
	-rm -f ayam.tex ayam.toc ayam.log ayam.aux ga
	/bin/sh -c "sed -f sedsrc ayam.sgml > ayaml.sgml"
	sgml2latex -p a4 -D out=latex -o tex ayaml.sgml
	/bin/sh -c "sed -f sedsrc3 ayaml.tex > ayam.tex"
	/bin/sh -c "sed -f sedsrc4 ayam.tex > ayaml.tex"
	/usr/bin/tclsh fixpdftex.tcl ayaml.tex ayam.tex
	pdflatex ayam.tex > ga
	rm -f ayaml.* ga

pdf5: pdf4
	pdflatex ayam.tex > ga
	pdflatex ayam.tex 
	rm ayam.tex ayam.toc ayam.log ayam.aux ayam.out ga

archive: html
	mkdir -p docs/pics
	cp pics/*.gif docs/pics
	cp html/*.html docs
	tar czf aydocs.tgz docs


clean:
	rm -rf html ayam.tex ayam.dvi ayam.txt ayam.pdf ayam.ps
	rm -rf ayam.aux ayam.toc ayam.log ayam.out
	rm -rf ayaml.*
	rm -rf docs

.PHONY: html