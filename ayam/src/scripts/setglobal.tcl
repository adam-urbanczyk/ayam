# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2006 by Randolf Schultz
# (randolf.schultz@gmail.com) and others.
#
# All rights reserved.
#
# See the file License for details.

# setglobal.tcl - example script for Ayam to set some global variables

# the following lines demonstrate how to modify the default values
# in the object create dialogs

#
# NURB Curves:
#
# create centered NURBS curves with 5 control points
set ::ay(nclen) 5
append ::ay(ncadda) "-ce 1"

# want them a bit larger? uncomment:
#append ::ay(ncadda) " -dx 0.5"

# hate those axis aligned lines? try:
#append ::ay(ncadda) " -dy 0.25"


#
# Interpolating Curves:
#
# create centered interpolating curves with 5 control points
set ::ay(iclen) 5
append ::ay(icadda) " -ce 1"

#
# Approximating Curves:
#
# create centered approximating curves with 5 control points
set ::ay(aclen) 5
append ::ay(acadda) " -ce 1"


#
# NURBS Surfaces:
#
# create centered NURBS patches with 5 by 5 control points
set ::ay(npwidth) 5
set ::ay(npheight) 5
append ::ay(npadda) " -ce 1"

# want them a bit larger? uncomment:
#append ::ay(npadda) " -udx 0.5 -vdy 0.5"


# set default length of closed BSplines (circles)
set ay(cbspnum) 10

return;

# EOF
