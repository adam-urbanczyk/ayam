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
o dvips
if you want to build a PDF you additionally need
o dvipdf (not recommended) or pdflatex (recommended)

To actually build the documentation (HTML and PDF) simply enter:
>make

To build just the HTML version:
>make htm

To build the PostScript version:
>make ps

To build a plain text version
(beware, output misses a lot of formatting information):
>make txt

To build a simple and not good looking PDF version made with dvipdf:
>make pdf

To build the real good looking PDF version:
>make pdf3


-- 
http://www.ayam3d.org/   Reconstruct the World!
