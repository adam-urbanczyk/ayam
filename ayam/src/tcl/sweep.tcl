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


# sweep_rotcurve:
# helper for Sweep/Swing creation; rotates the selected curve to proper plane:
# if plane is 0: to YZ-plane,
# if plane is 1: to XZ-plane,
# if plane is 2: to XY-plane,
proc sweep_rotcurve { plane } {
    global ayprefs
    while { 1 } {
	if { $ayprefs(RotateCrossSection) != 0 } {
	    getType type
	    set stride 3
	    if { ($type == "NCurve") } {
		incr stride
	    }
	    getPnt -trafo -all pnts
	    set j $plane
	    set isInPlane true
	    while { $j < [llength $pnts] } {
		set xyz [lindex $pnts $j]
		if { [expr {abs($xyz) > 10e-6}] } {
		    set isInPlane false
		    break
		}
		incr j $stride
	    }
	    if { $ayprefs(RotateCrossSection) == 2 && $isInPlane } {
		break;
	    }
	    if { $ayprefs(RotateCrossSection) == 2 && !$isInPlane } {
		#"Correct Curve?"
		set t [ms info_rc1]
		#"Rotate curve to correct plane?"
		set m [ms info_rc2]
		if { $ayprefs(FixDialogTitles) == 1 } {
		    set m "$t\n\n$m"
		}
		set answer [tk_messageBox -title $t -type yesno -icon question\
				-message $m]
		if { $answer == "no" } {
		    break;
		}
	    }

	    undo save ToPlane

	    if { ($type == "NCurve") || ($type == "ICurve") ||
		 ($type == "ACurve") } {
		switch $plane { 0 { toYZC } 1 { toXZC } 2 { toXYC } }
		resetRotate; normTrafos
	    } else {
		if { 1 } {
		    if { [hasTrafo -r] } {
			break;
		    }
		    switch $plane {
			0 { rotOb 0 90 0 }
			1 { rotOb 90 0 0 }
		    }
		} else {
		    global transfPropData
		    getTrafo
		    set cn [getNormal -trafo]
		    switch $plane {
			0 { set tn { 1 0 0 } }
			1 { set tn { 0 1 0 } }
			2 { set tn { 0 0 1 } }
		    }
		    # compute new rotation attributes from cn and tn
		}

		notifyOb
	    }
	    rV
	}
	break;
    }
  return;
}
# sweep_rotcurve


# sweep_rotcross:
# helper for Sweep creation
proc sweep_rotcross { {goup 1} } {
    goDown -1
    selOb 0
    sweep_rotcurve 0
    if { $goup } {
	goUp; sL
    }
 return;
}
# sweep_rotcross
