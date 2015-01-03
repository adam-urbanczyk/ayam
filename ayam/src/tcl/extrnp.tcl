# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2004 by Randolf Schultz
# (randolf.schultz@gmail.com) and others.
#
# All rights reserved.
#
# See the file License for details.

# extrnp.tcl - ExtrNP objects Tcl code

set ExtrNP 1

proc init_ExtrNP { } {
global ay ExtrNP_props ExtrNPAttr ExtrNPAttrData

set ExtrNP_props { Attributes Material Tags Bevels Caps ExtrNPAttr }

array set ExtrNPAttr {
arr   ExtrNPAttrData
sproc ""
gproc ""
w     fExtrNPAttr
}

array set ExtrNPAttrData {
DisplayMode 1
UMin 0.0
UMin 1.0
VMin 0.0
VMin 1.0
BoundaryNames { "U0" "U1" "V0" "V1" }
BevelsChanged 0
CapsChanged 0
}

# create ExtrNPAttr-UI
set w [frame $ay(pca).$ExtrNPAttr(w)]
addVSpace $w s1 2
addParam $w ExtrNPAttrData UMin
addParam $w ExtrNPAttrData UMax
addParam $w ExtrNPAttrData VMin
addParam $w ExtrNPAttrData VMax

addCheck $w ExtrNPAttrData Relative

addParam $w ExtrNPAttrData PatchNum

addParam $w ExtrNPAttrData Tolerance
addMenu $w ExtrNPAttrData DisplayMode $ay(npdisplaymodes)

addText $w ExtrNPAttrData "Extracted NURBS Patch:"
addInfo $w ExtrNPAttrData NPInfo

return;
}
# init_ExtrNP
