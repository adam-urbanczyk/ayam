# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2001 by Randolf Schultz
# (randolf.schultz@gmail.com) and others.
#
# All rights reserved.
#
# See the file License for details.

# skin.tcl - Skin objects Tcl code

set Skin_props { Transformations Attributes Material Tags Bevels Caps SkinAttr }

array set SkinAttr {
arr   SkinAttrData
sproc ""
gproc ""
w     fSkinAttr
}

array set SkinAttrData {
Interpolate 0
DisplayMode 1
Knot-Type_U 1
NPInfoBall "n/a"
BoundaryNames { "Start" "End" "Left" "Right" }
BoundaryIDs { 2 3 0 1 }
}

set w [frame $ay(pca).$SkinAttr(w)]
addVSpace $w s1 2
addCheck $w SkinAttrData Interpolate
addParam $w SkinAttrData Order_U
addMenu $w SkinAttrData Knot-Type_U [list Bezier B-Spline NURB Custom]

addParam $w SkinAttrData Tolerance
addMenu $w SkinAttrData DisplayMode $ay(npdisplaymodes)

addText $w SkinAttrData "Created NURBS Patch:"
addInfo $w SkinAttrData NPInfo
