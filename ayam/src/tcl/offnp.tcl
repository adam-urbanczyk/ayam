# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2007 by Randolf Schultz
# (randolf.schultz@gmail.com) and others.
#
# All rights reserved.
#
# See the file License for details.

# offnp.tcl - offset surfaces objects Tcl code

set OffsetNP 1

proc init_OffsetNP { } {
global ay OffsetNP_props OffsetNPAttr OffsetNPAttrData

set OffsetNP_props { Attributes Material Tags OffsetNPAttr }

array set OffsetNPAttr {
arr   OffsetNPAttrData
sproc ""
gproc ""
w     fOffsetNPAttr
}

array set OffsetNPAttrData {
Mode 1
DisplayMode 0
NPInfoBall "N/A"
}
# create UI
set w [frame $ay(pca).$OffsetNPAttr(w)]

addMenu $w OffsetNPAttrData Mode [list "Normal" "Section" ]

addParam $w OffsetNPAttrData Offset

addParam $w OffsetNPAttrData Tolerance
addMenu $w OffsetNPAttrData DisplayMode $ay(npdisplaymodes)

addText $w OffsetNPAttrData "Created NURBS Patch:"
addInfo $w OffsetNPAttrData NPInfo

return;
}
# init_OffsetNP
