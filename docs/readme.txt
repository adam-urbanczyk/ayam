[readme.txt; 22. Jan 2012]

This directory contains the sources (SGML and pictures) for the main
Ayam documentation (the Ayam reference manual) and a Tcl-script to
create a reference card.

In order to build your own version of the documentation you need the
following prerequisites:
o make
o tclsh in /usr/bin/ (otherwise edit the Makefile)
o sgml-tools 1.0.9 or linuxdoc-tools

if you want to build a PostScript version you, additionally, need
o LaTeX
o dvips

if you want to build a PDF version you, additionally, need
o dvipdf (not recommended) or pdflatex (recommended)
o needspace.sty, shorttoc.sty, footmisc.sty

Note: In order for HTML, PostScript, and PDF versions of the
documentation to be created the sub directory "pics" must
contain the pictures in GIF, EPS, _and_ PNG format respectively.

To actually build the documentation (HTML and nice PDF) simply enter:
>make

To build just the HTML version:
>make html

To re-create the style of footnotes of the official documentation
find and change in your linuxdoc html "mapping" file:

<footnote>      +       "<BLOCKQUOTE>"
</footnote>             "</BLOCKQUOTE>"         +

to

<footnote>      +       "<SMALL TITLE=\""
</footnote>             "\">*</SMALL>"            +

or

<footnote>      +       "<SUB TITLE=\""
</footnote>             "\">*</SUB>"            +

See also the provided file "html-mapping".

To build the PostScript version:
>make ps

This will not work unless you create EPS files for all pictures first!

To build a plain text version
(beware, output misses a lot of formatting information):
>make txt

To build the PDF version:
>make pdf

If you want a footnote rule as in the distributed PDF, find
and adapt linuxdoc-sgml.sty accordingly (comment out the
"\let\footnoterule\relax"
line with a %).

In addition, to improve the in-document hyperlinks, find and change in
your linuxdoc latex2e "mapping" file:

<ref>			"\\ref{[ID]} {([NAME])}"

to 

<ref>			"\\hyperref\[[ID]\]{\\ref*{[ID]} [NAME] (page \\pageref*{[ID]})}"

See also the provided file "latex2e-mapping".

==============================================================================

To (re)create the reference card, you need the following prerequisites:
o LaTeX

How do you create the reference card?
o Start Ayam.
o Enter into the Ayam console:
  >source ../docs/refcard.tcl
  (substitute ../docs with the real path of "refcard.tcl")
  >printHotKeys myrefcard.tex
o Now enter into a normal shell:
  >pdflatex myrefcard.tex

-- 
http://www.ayam3d.org/   Reconstruct the World!
