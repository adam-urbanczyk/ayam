# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2001 by Randolf Schultz
# (randolf.schultz@gmail.com) and others.
#
# All rights reserved.
#
# See the file License for details.

# npatch.tcl - NURBS patchs objects Tcl code

set NPatch_props { Transformations Attributes Material Tags Caps Bevels
    NPatchAttr }

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
Knots_U ""
Knots_U-Modified 0
Knots_V ""
Knots_V-Modified 0
BoundaryNames { "U0" "U1" "V0" "V1" }
}

# create NPatchAttr-UI
set w [frame $ay(pca).$NPatchAttr(w)]
addVSpace $w s1 2
addParam $w NPatchAttrData Width
addParam $w NPatchAttrData Height
addParam $w NPatchAttrData Order_U
addParam $w NPatchAttrData Order_V

addMenu $w NPatchAttrData Knot-Type_U [list Bezier B-Spline NURB Custom\
					 Chordal Centripetal]
addString $w NPatchAttrData Knots_U
addMenu $w NPatchAttrData Knot-Type_V [list Bezier B-Spline NURB Custom\
					 Chordal Centripetal]
addString $w NPatchAttrData Knots_V

addCheck $w NPatchAttrData CreateMP

addInfo $w NPatchAttrData IsRat

addParam $w NPatchAttrData Tolerance
addMenu $w NPatchAttrData DisplayMode $ay(npdisplaymodes)

trace variable NPatchAttrData(Knots_U) w markPropModified
trace variable NPatchAttrData(Knots_V) w markPropModified

