# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2001 by Randolf Schultz
# (randolf.schultz@gmail.com) and others.
#
# All rights reserved.
#
# See the file License for details.

# icurve.tcl - interpolating curves objects Tcl code

set ICurve 1

proc init_ICurve { } {
global ay ICurve_props ICurveAttr ICurveAttrData

set ICurve_props { Transformations Attributes Tags ICurveAttr }

array set ICurveAttr {
arr   ICurveAttrData
sproc ""
gproc ""
w     fICurveAttr

}

array set ICurveAttrData {
Type 0
DisplayMode 0
ParamType 0
NCInfoBall "N/A"
}
# create IcurveAttr-UI
set w [frame $ay(pca).$ICurveAttr(w)]

addMenu $w ICurveAttrData Type [list "Open" "Closed" ]
addParam $w ICurveAttrData Length
addParam $w ICurveAttrData Order
addMenu $w ICurveAttrData ParamType [list "Chordal" "Centripetal" "Uniform" ]
addCheck $w ICurveAttrData Derivatives
addParam $w ICurveAttrData SDLen
addParam $w ICurveAttrData EDLen

addParam $w ICurveAttrData Tolerance
addMenu $w ICurveAttrData DisplayMode $ay(ncdisplaymodes)

addText $w ICurveAttrData "Created NURBS Curve:"
addInfo $w ICurveAttrData NCInfo

}
# init_ICurve
