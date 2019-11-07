# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2012 by Randolf Schultz
# (randolf.schultz@gmail.com) and others.
#
# All rights reserved.
#
# See the file License for details.

# sfcurve.tcl - SfCurve objects Tcl code

global ay SfCurve_props SfCurveAttr SfCurveAttrData

set SfCurve_props { Transformations Attributes Material Tags SfCurveAttr }

array set SfCurveAttr {
    arr   SfCurveAttrData
    sproc ""
    gproc ""
    w     fSfCurveAttr
}

array set SfCurveAttrData {
    Knot-Type 1
    DisplayMode 0
}

# create SfCurveAttr-UI
set w [frame $ay(pca).$SfCurveAttr(w)]
set a $SfCurveAttr(arr)
set ay(bok) $ay(appb)

addParam $w $a M
addParam $w $a N1
addParam $w $a N2
addParam $w $a N3
addParam $w $a TMin
addParam $w $a TMax
addParam $w $a Sections

addParam $w $a Order
#addMenu $w $a Knot-Type [list NURB Chordal Centripetal]

addParam $w $a Tolerance
addMenu $w $a DisplayMode $ay(ncdisplaymodes)


# add menu entry to the "Create/Custom Object" sub-menu
mmenu_addcustom SfCurve "crtOb SfCurve;uCR;sL;rV"

# tell the rest of Ayam (or other custom objects), that we are loaded
lappend ay(co) SfCurve

# EOF
