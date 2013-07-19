# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2009 by Randolf Schultz
# (randolf.schultz@gmail.com) and others.
#
# All rights reserved.
#
# See the file License for details.

# sweep.tcl - sweep objects Tcl code

set Sweep 1

proc init_Sweep { } {
global ay Sweep_props SweepAttr SweepAttrData

set Sweep_props { Transformations Attributes Material Tags Bevels Caps SweepAttr }

array set SweepAttr {
arr   SweepAttrData
sproc ""
gproc ""
w     fSweepAttr
}

array set SweepAttrData {
Type 0
DisplayMode 1
NPInfoBall "n/a"
BoundaryNames { "Start" "End" "Left" "Right" }
BoundaryIDs { 2 3 0 1 }
StartCap 0
EndCap 0
LeftCap 0
RightCap 0
}

set w [frame $ay(pca).$SweepAttr(w)]
addVSpace $w s1 2
addMenu $w SweepAttrData Type [list Open Closed Periodic]
addCheck $w SweepAttrData Rotate
addCheck $w SweepAttrData Interpolate
addParam $w SweepAttrData Sections

addParam $w SweepAttrData Tolerance
addMenu $w SweepAttrData DisplayMode $ay(npdisplaymodes)

addText $w SweepAttrData "Created NURBS Patch:"
addInfo $w SweepAttrData NPInfo

return;
}
# init_Sweep


# sweep_rotcross:
#  helper for Sweep creation; rotates the cross section to YZ plane
proc sweep_rotcross { } {
    global transfPropData
    goDown -1
    selOb 0
    getTrafo
    if { $transfPropData(Rotate_Y) == 0.0 } {
	rotOb 0 90 0
	notifyOb
	rV
    }
    goUp
    sL
 return;
}
# sweep_rotcross
