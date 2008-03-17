# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2008 by Randolf Schultz
# (randolf.schultz@gmail.com) and others.
#
# All rights reserved.
#
# See the file License for details.

# acurve.tcl - approximating curves objects Tcl code

set ACurve_props { Transformations Attributes Tags ACurveAttr }


array set ACurveAttr {
arr   ACurveAttrData
sproc ""
gproc ""
w     fACurveAttr

}

array set ACurveAttrData {
Mode 1
DisplayMode 0
NCInfoBall "N/A"
}
# create ACurveAttr-UI
set w [frame $ay(pca).$ACurveAttr(w)]

addParam $w ACurveAttrData Length
addParam $w ACurveAttrData ALength
addCheck $w ACurveAttrData Closed
addParam $w ACurveAttrData Order

addParam $w ACurveAttrData Tolerance
addMenu $w ACurveAttrData DisplayMode $ay(ncdisplaymodes)

addText $w ACurveAttrData "Created NURBS Curve:"
addInfo $w ACurveAttrData NCInfo
