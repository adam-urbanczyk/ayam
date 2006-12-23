# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2004 by Randolf Schultz
# (rschultz@informatik.uni-rostock.de) and others.
#
# All rights reserved.
#
# See the file License for details.

# ncircle.tcl - Ncircle objects Tcl code

set NCircle_props { Transformations Attributes Material Tags NCircleAttr }


array set NCircleAttr {
arr   NCircleAttrData
sproc ""
gproc ""
w     fNCircleAttr

}

array set NCircleAttrData {
DisplayMode 1
}

# create NCircleAttr-UI
set w [frame $ay(pca).$NCircleAttr(w)]

addParam $w NCircleAttrData Radius
addParam $w NCircleAttrData TMin
addParam $w NCircleAttrData TMax

addParam $w NCircleAttrData Tolerance
addMenu $w NCircleAttrData DisplayMode $ay(ncdisplaymodes)

addText $w NCircleAttrData "Created NURBS Curve:"
addInfo $w NCircleAttrData NCInfo
