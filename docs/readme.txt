[readme.txt; 14. Aug 2014]

This directory contains the sources (SGML and pictures) for the main
Ayam documentation (the Ayam reference manual) and a Tcl-script to
create a reference card.

In order to build the documentation the following prerequisites are needed:
o make
o tclsh in /usr/bin/ (otherwise edit the Makefile)
o sgml-tools 1.0.9 or linuxdoc-tools

Additional prerequisites for the PostScript version are:
o LaTeX
o dvips

Additional prerequisites for the PDF version are:
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
My html mapping resides in "/usr/share/linuxdoc-tools/mappings/html".

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
My latex2e mapping resides in "/usr/share/linuxdoc-tools/mappings/latex2e".

Also employ the isoent and ISOtech.2tex files.
My isoent resides in "/usr/share/sgml/dtd".
My ISOtech.2tex resides in "/usr/share/entity-map/0.1.0/".

==============================================================================

To (re)create the reference card, the following prerequisites are needed:
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
