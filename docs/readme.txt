[readme.txt 16. Nov 2001]

This directory contains the sources (SGML and pictures) for the main
Ayam documentation, the Ayam reference manual.

In order to build your own version of the documentation you need the
following prerequisites:
o make
o tcsh in /bin (otherwise edit "fixhtml.tcsh")
o sed
o sgml-tools1.0.9
if you want to build a PostScript you additionally need
o LaTeX
if you want to build a PDF you additionally need
o pdflatex

To actually build the documentation (HTML and PDF) simply enter:
>make


