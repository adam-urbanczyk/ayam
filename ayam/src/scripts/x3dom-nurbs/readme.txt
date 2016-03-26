[readme.txt - 26. Mar 2016]

This directory contains a NURBS tessellator for x3dom.

The tessellator is based on idea and example code from
A. J. Chung and A. J. Field (https://sourceforge.net/projects/emvise/).

The implementation spans four files:

o x3dom-nurbs-nodes.js - interface of the tessellator to x3dom
o x3dom-nurbs-pool.js - worker managament (current pool size is 3)
o x3dom-nurbs-worker.js - a worker
o x3dom-nurbs-tess.js - the tessellator

In order to use the tessellator just add the following to your XHTML
after inclusion of x3dom.js:
 <script type="text/javascript" src="x3dom-nurbs-pool.js"/>
 <script type="text/javascript" src="x3dom-nurbs-nodes.js"/>
.

As the tessellator is fully automatic, no further adjustments are needed.

For full documentation, see the main Ayam documentation.
