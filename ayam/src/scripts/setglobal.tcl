# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2006 by Randolf Schultz
# (rschultz@informatik.uni-rostock.de) and others.
#
# All rights reserved.
#
# See the file License for details.

# setglobal.tcl - example script for Ayam to set some global variables

global ay

# set default length of NURBS curves
set ay(nclen) 5

# set default width of NURBS patches
set ay(npwidth) 5

# set default height of NURBS patches
set ay(npheight) 5

# set default length of closed BSplines (circles)
set ay(cbspnum) 10

return;