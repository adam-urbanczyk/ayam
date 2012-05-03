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

addParam $w SfCurveAttrData M
addParam $w SfCurveAttrData N1
addParam $w SfCurveAttrData N2
addParam $w SfCurveAttrData N3
addParam $w SfCurveAttrData TMin
addParam $w SfCurveAttrData TMax
addParam $w SfCurveAttrData Sections

addParam $w SfCurveAttrData Order
#addMenu $w SfCurveAttrData Knot-Type [list NURB Chordal Centripetal]

addParam $w SfCurveAttrData Tolerance
addMenu $w SfCurveAttrData DisplayMode $ay(ncdisplaymodes)


# add menu entry to the "Create/Custom Object" sub-menu
mmenu_addcustom SfCurve "crtOb SfCurve;uS;sL;rV"

# tell the rest of Ayam (or other custom objects), that we are loaded
lappend ay(co) SfCurve
