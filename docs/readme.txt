[readme.txt; 16. Dec 2002]

This directory contains the sources (SGML and pictures) for the main
Ayam documentation (the Ayam reference manual) and a Tcl-script to
create a reference card.

In order to build your own version of the documentation you need the
following prerequisites:
o make
o tcsh in /bin (otherwise edit "fixhtml.tcsh")
o sed
o sgml-tools1.0.9
if you want to build a PostScript you, additionally ,need
o LaTeX
o dvips
if you want to build a PDF you, additionally, need
o dvipdf (not recommended) or pdflatex (recommended)

If you are on Debian 3.0 (Woody) you, additionally, need:
o tclsh in /usr/bin/ (otherwise edit "Makefile/pdf4:")

Note: In order for HTML, PostScript, and PDF versions of the
documentation to be created the sub directory "pics" must
contain the pictures in GIF, EPS, and PNG format respectively.

To actually build the documentation (HTML and nice PDF) simply enter:
>make

To build just the HTML version:
>make htm

To build the PostScript version:
>make ps

To build a plain text version
(beware, output misses a lot of formatting information):
>make txt

To build the real good looking PDF version:
>make pdf3

If you are on Debian 3.0 (Woody) the pdf3-target will not work,
use pdf5 instead!

To build a simple and not good looking PDF version made with dvipdf:
>make pdf

==============================================================================

To (re)create the reference card, you need the following prerequisites:
o LaTeX
o dvips

How do you create the reference card?
o Start Ayam.
o Enter into the Ayam console:
  >source ../docs/refcard.tcl
  (substitute ../docs with the real path of "refcard.tcl")
  >printHotKeys myrefcard.tex
o Now enter into a normal shell:
  >latex myrefcard.tex
  >dvips myrefcard.tex -o myrefcard.ps


-- 
http://www.ayam3d.org/   Reconstruct the World!
