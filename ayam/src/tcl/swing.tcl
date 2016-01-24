# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2001 by Randolf Schultz
# (randolf.schultz@gmail.com) and others.
#
# All rights reserved.
#
# See the file License for details.

# swing.tcl - Swing objects Tcl code

set Swing 1

proc init_Swing { } {
global ay Swing_props SwingAttr SwingAttrData

set Swing_props { Transformations Attributes Material Tags Bevels Caps SwingAttr }

array set SwingAttr {
arr   SwingAttrData
sproc ""
gproc ""
w     fSwingAttr
}

array set SwingAttrData {
DisplayMode 1
NPInfoBall "n/a"
BoundaryNames { "Start" "End" "Upper" "Lower" }
BoundaryIDs { 2 3 0 1 }
}

# create SwingAttr-UI
set w [frame $ay(pca).$SwingAttr(w)]

addParam $w SwingAttrData Tolerance
addMenu $w SwingAttrData DisplayMode $ay(npdisplaymodes)

addText $w SwingAttrData "Created NURBS Patch:"
addInfo $w SwingAttrData NPInfo

return;
}
# init_Swing


# swing_rotcross:
#  helper for Swing creation; rotates the cross section to YZ plane
#  and the trajectory to the XZ plane
proc swing_rotcross { } {
    global transfPropData
    goDown -1
    selOb 0 1
    undo save ToPlane
    selOb 0
    getType type
    if { ($type == "NCurve") || ($type == "ICurve") || ($type == "ACurve") } {
	toYZC; resetTrafo; normTrafos
    } else {
	getTrafo
	if { $transfPropData(Rotate_Y) == 0.0 } {
	    rotOb 0 90 0
	}
	notifyOb
    }
    selOb 1
    getType type
    if { ($type == "NCurve") || ($type == "ICurve") || ($type == "ACurve") } {
	toXZC; resetTrafo; normTrafos
    } else {
	getTrafo
	if { $transfPropData(Rotate_X) == 0.0 } {
	    rotOb 90 0 0
	}
	notifyOb
    }
    rV; goUp; sL
 return;
}
# swing_rotcross
