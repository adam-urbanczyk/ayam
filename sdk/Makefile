all: html pdf3

html:
	rm -rf html
	mkdir html
	cp sdk.sgml html
	cd html;sgml2html -s 1 sdk.sgml;rm sdk.sgml;cd ..
	./fixhtml.tcsh

txt:
	sgml2txt -f sdk.sgml

ps:
	/bin/sh -c "sed -f sedsrc sdk.sgml > sdkl.sgml"
	sgml2latex -o ps sdkl.sgml
	mv sdkl.ps sdk.ps
	rm -f sdkl.sgml

pdf:
	/bin/sh -c "sed -f sedsrc sdk.sgml > sdkl.sgml"
	sgml2latex -o dvi sdkl.sgml
	dvi2pdf sdkl.dvi
	mv sdkl.pdf sdk.pdf
	rm -f sdkl.sgml sdkl.dvi

pdf2:
	/bin/sh -c "sed -f sedsrc sdk.sgml > sdkl.sgml"
	sgml2latex -o tex sdkl.sgml
	/bin/sh -c "sed -f sedsrc2 sdkl.tex > sdk.tex"
	/bin/sh -c "sed -f sedsrc3 sdk.tex > sdkl.tex"
	mv sdkl.tex sdk.tex
	pdflatex sdk.tex
	rm -f sdkl.sgml


pdf3: pdf2
	pdflatex sdk.tex
	pdflatex sdk.tex
	rm sdk.toc sdk.aux sdk.tex sdk.log sdk.out

clean:
	rm -rf html sdk.tex sdk.dvi sdk.txt sdk.pdf sdk.ps
	rm -f sdkl.*

.PHONY: html
