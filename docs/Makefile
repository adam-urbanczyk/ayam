# Emacs, this is -*- Mode: makefile -*-
#
# Makefile [17. Feb 2012]
# Makefile for the Ayam reference manual

all: html pdf


html:
	rm -rf html
	mkdir html
	cd html; ln -sf ../pics pics;cd ..
	cp ayam.sgml html
	cd html;sgml2html -D out=html -s 1 ayam.sgml;rm ayam.sgml;cd ..
	-cd html;../corrwh.pl;cd ..
	tclsh insover.tcl

html-draft: html
	tclsh insdraft.tcl

txt:
	sgml2txt -D out=txt -f ayam.sgml

ps:
	sgml2latex -o ps ayaml.sgml
	mv ayaml.ps ayam.ps
	rm -f ayaml.sgml

pdf:
	-rm -f ayam.tex ayam.toc ayam.log ayam.aux
	sgml2latex -p a4 -D out=latex -o tex ayam.sgml
	/usr/bin/tclsh fixpdftex.tcl ayam.tex ayaml.tex
	cp ayaml.tex ayam.tex
	pdflatex ayam.tex > /dev/null
	pdflatex ayam.tex > /dev/null
	pdflatex ayam.tex
	rm ayaml.tex ayam.tex ayam.toc ayam.log ayam.aux ayam.out

pdf2:
	-rm -f ayam.tex ayam.toc ayam.log ayam.aux
	sgml2latex -p a4 -D out=latex -o tex ayam.sgml
	/usr/bin/tclsh fixpdftex.tcl ayam.tex ayaml.tex
	cp ayaml.tex ayam.tex
	pdflatex ayam.tex
	pdflatex ayam.tex
	pdflatex ayam.tex
#	rm ayaml.tex ayam.tex ayam.toc ayam.log ayam.aux ayam.out

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

tags:
	etags --language=none --regex=@emacs.tags ayam.sgml

.PHONY: html