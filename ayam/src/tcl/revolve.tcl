# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2001 by Randolf Schultz
# (randolf.schultz@gmail.com) and others.
#
# All rights reserved.
#
# See the file License for details.

# revolve.tcl - Revolve objects Tcl code

set Revolve 1

proc init_Revolve { } {
global ay Revolve_props RevolveAttr RevolveAttrData

set Revolve_props { Transformations Attributes Material Tags Bevels Caps RevolveAttr }

array set RevolveAttr {
arr   RevolveAttrData
sproc ""
gproc ""
w     fRevolveAttr
}

array set RevolveAttrData {
DisplayMode 1
NPInfoBall "n/a"
BoundaryNames { "Upper" "Lower" "Start" "End" }
BoundaryIDs { 2 3 0 1 }
}

# create RevolveAttr-UI
set w [frame $ay(pca).$RevolveAttr(w)]
addVSpace $w s1 2
addParam $w RevolveAttrData ThetaMax
addParam $w RevolveAttrData Sections
addParam $w RevolveAttrData Order

addParam $w RevolveAttrData Tolerance
addMenu $w RevolveAttrData DisplayMode $ay(npdisplaymodes)

addText $w RevolveAttrData "Created NURBS Patch:"
addInfo $w RevolveAttrData NPInfo

return;
}
# init_Revolve
