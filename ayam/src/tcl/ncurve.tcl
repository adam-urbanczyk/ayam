# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2001 by Randolf Schultz
# (rschultz@informatik.uni-rostock.de) and others.
#
# All rights reserved.
#
# See the file License for details.

# ncurve.tcl - NURBS curves objects Tcl code

set NCurve_props { Transformations Attributes Tags NCurveAttr }


array set NCurveAttr {
arr   NCurveAttrData
sproc ""
gproc ""
w     fNCurveAttr

}

array set NCurveAttrData {
Knot-Type 1
DisplayMode 0
}
# create NCurveAttr-UI
set w [frame $ay(pca).$NCurveAttr(w)]

addParam $w NCurveAttrData Length
addParam $w NCurveAttrData Order
addCheck $w NCurveAttrData Closed
addMenu $w NCurveAttrData Knot-Type [list Bezier B-Spline NURB Custom]
addString $w NCurveAttrData Knots
addParam $w NCurveAttrData Tolerance
addMenu $w NCurveAttrData DisplayMode [list Global Curve CurveAndHull ControlHull]

