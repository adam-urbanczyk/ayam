# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2007 by Randolf Schultz
# (randolf.schultz@gmail.com) and others.
#
# All rights reserved.
#
# See the file License for details.

# offnc.tcl - offset curves objects Tcl code

set OffsetNC 1

proc init_OffsetNC { } {
global ay OffsetNC_props OffsetNCAttr OffsetNCAttrData

set OffsetNC_props { Attributes Tags OffsetNCAttr }

array set OffsetNCAttr {
arr   OffsetNCAttrData
sproc ""
gproc ""
w     fOffsetNCAttr
}

array set OffsetNCAttrData {
Mode 1
DisplayMode 0
NCInfoBall "N/A"
}
# create UI
set w [frame $ay(pca).$OffsetNCAttr(w)]
addVSpace $w s1 2
addMenu $w OffsetNCAttrData Mode [list "Point" "Section" "Hybrid" "3DPVN"]
addCheck $w OffsetNCAttrData Revert
addParam $w OffsetNCAttrData Offset

addParam $w OffsetNCAttrData Tolerance
addMenu $w OffsetNCAttrData DisplayMode $ay(ncdisplaymodes)

addText $w OffsetNCAttrData "Created NURBS Curve:"
addInfo $w OffsetNCAttrData NCInfo

return;
}
# init_OffsetNC
