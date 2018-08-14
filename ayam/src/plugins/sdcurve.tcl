# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2017 by Randolf Schultz
# (randolf.schultz@gmail.com) and others.
#
# All rights reserved.
#
# See the file License for details.

# sdcurve.tcl - Sdcurve objects Tcl code

global ay SDCurve_props SDCurveAttr SDCurveAttrData

set SDCurve_props { Transformations Attributes Tags SDCurveAttr }

array set SDCurveAttr {
arr   SDCurveAttrData
sproc ""
gproc ""
w     fSDCurveAttr
}

array set SDCurveAttrData {
Type 0
SLength "n/a"
}

# create SDCurveAttr-UI
set w [frame $ay(pca).$SDCurveAttr(w)]

set ay(bok) $ay(appb)

addVSpace $w s1 2

addCheck $w SDCurveAttrData Closed
addParam $w SDCurveAttrData Length
set l [list Chaikin Cubic]
addMenu $w SDCurveAttrData Type $l
addParam $w SDCurveAttrData Level
addText $w SDCurveAttrData "Subdivision:"
addInfo $w SDCurveAttrData SLength

# add menu entry to the "Create/Custom Object" sub-menu
mmenu_addcustom SDCurve "crtOb SDCurve;uS;sL;rV"

# tell the rest of Ayam (or other custom objects), that we are loaded
lappend ay(co) SDCurve

# EOF
