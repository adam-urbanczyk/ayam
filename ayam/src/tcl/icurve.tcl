# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2001 by Randolf Schultz
# (rschultz@informatik.uni-rostock.de) and others.
#
# All rights reserved.
#
# See the file License for details.

# icurve.tcl - interpolating curves objects Tcl code

set ICurve_props { Transformations Attributes Tags ICurveAttr }


array set ICurveAttr {
arr   ICurveAttrData
sproc ""
gproc ""
w     fIcurveAttr

}

array set ICurveAttrData {
Mode 1
DisplayMode 0
}
# create IcurveAttr-UI
set w [frame $ay(pca).$ICurveAttr(w)]

addParam $w ICurveAttrData Length
addCheck $w ICurveAttrData Closed
addParam $w ICurveAttrData Order
addMenu $w ICurveAttrData Mode [list "C2-Cubic" "Global-4D" ]
addParam $w ICurveAttrData IParam
addParam $w ICurveAttrData Tolerance
addMenu $w ICurveAttrData DisplayMode $ay(ncdisplaymodes)

addText $w ICurveAttrData "Created NURBS Curve:"
addInfo $w ICurveAttrData NCInfo
