# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2009 by Randolf Schultz
# (randolf.schultz@gmail.com) and others.
#
# All rights reserved.
#
# See the file License for details.

# concatnp.tcl - concatnp objects Tcl code

set ConcatNP 1

proc init_ConcatNP { } {
global ay ConcatNP_props ConcatNPAttr ConcatNPAttrData

set ConcatNP_props { Transformations Attributes Tags Caps Bevels ConcatNPAttr }

array set ConcatNPAttr {
arr   ConcatNPAttrData
sproc ""
gproc ""
w     fConcatNPAttr
}

array set ConcatNPAttrData {
Type 0
FillGaps 0
Revert 0
Length 0
Order 0
Knot-Type 1
FTLength 1.0
NPInfoBall "N/A"
BoundaryNames { "U0" "U1" "V0" "V1" }
}

# create ConcatNPAttr-UI
set w [frame $ay(pca).$ConcatNPAttr(w)]
addVSpace $w s1 2
addMenu $w ConcatNPAttrData Type [list Open Closed Periodic]
addParam $w ConcatNPAttrData Order
addCheck $w ConcatNPAttrData Revert
addCheck $w ConcatNPAttrData FillGaps
addParam $w ConcatNPAttrData FTLength
addMenu $w ConcatNPAttrData Knot-Type\
    [list B-Spline NURB Chordal Centripetal Custom]
addString $w ConcatNPAttrData UVSelect
addCheck $w ConcatNPAttrData Compatible

addText $w  e0 "Resulting Patch:"
addInfo $w ConcatNPAttrData NPInfo

return;
}
# init_ConcatNP
