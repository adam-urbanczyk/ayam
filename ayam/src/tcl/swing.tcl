# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2001 by Randolf Schultz
# (randolf.schultz@gmail.com) and others.
#
# All rights reserved.
#
# See the file License for details.

# swing.tcl - Swing objects Tcl code

proc init_Swing { } {
    global ay Swing_props SwingAttr SwingAttrData

    set Swing_props [list Transformations Attributes Material Tags Bevels Caps\
			 SwingAttr]

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
    set a $SwingAttr(arr)
    addParam $w $a Tolerance
    addMenu $w $a DisplayMode $ay(npdisplaymodes)
    addText $w $a "Created NURBS Patch:"
    addInfo $w $a NPInfo

 return;
}
# init_Swing


# swing_rotcross:
#  helper for Swing creation; rotates the cross section to YZ plane
#  and the trajectory to the XZ plane
proc swing_rotcross { } {
    global transfPropData
    goDown -1
    selOb 0
    sweep_rotcurve 0
    selOb 1
    sweep_rotcurve 1
    rV; goUp; sL
 return;
}
# swing_rotcross
