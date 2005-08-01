# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2001 by Randolf Schultz
# (rschultz@informatik.uni-rostock.de) and others.
#
# All rights reserved.
#
# See the file License for details.

# npatch.tcl - NURBS patchs objects Tcl code

set NPatch_props { Transformations Attributes Material Tags NPatchAttr }


array set NPatchAttr {
arr   NPatchAttrData
sproc ""
gproc ""
w     fNPatchAttr

}

array set NPatchAttrData {
Knot-Type_U 1
Knot-Type_V 1
DisplayMode 1
}
# create NPatchAttr-UI
set w [frame $ay(pca).$NPatchAttr(w)]

addParam $w NPatchAttrData Width
addParam $w NPatchAttrData Height
addParam $w NPatchAttrData Order_U
addParam $w NPatchAttrData Order_V

addMenu $w NPatchAttrData Knot-Type_U [list Bezier B-Spline NURB Custom]
addString $w NPatchAttrData Knots_U
addMenu $w NPatchAttrData Knot-Type_V [list Bezier B-Spline NURB Custom]
addString $w NPatchAttrData Knots_V

addInfo $w NPatchAttrData IsRat

addParam $w NPatchAttrData Tolerance
global ay
set l $ay(npdisplaymodes)
set l [linsert $l 0 Global]
addMenu $w NPatchAttrData DisplayMode $l

