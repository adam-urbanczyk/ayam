# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2009 by Randolf Schultz
# (randolf.schultz@gmail.com) and others.
#
# All rights reserved.
#
# See the file License for details.

# pamesh.tcl - PatchMesh objects Tcl code

proc init_PatchMesh { } {
    global ay PatchMesh_props PatchMeshAttr PatchMeshAttrData

    set PatchMesh_props [list Transformations Attributes Material Tags Bevels\
			     Caps PatchMeshAttr]

    array set PatchMeshAttr {
	arr   PatchMeshAttrData
	sproc ""
	gproc pamesh_getAttr
	w     fPatchMeshAttr
    }

    array set PatchMeshAttrData {
	Type 0
	BType_U 0
	BType_V 0
	Step_U 0
	Step_V 0
	DisplayMode 1
	BoundaryNames { "U0" "U1" "V0" "V1" }
	BevelsChanged 0
	CapsChanged 0
    }

    for {set i 0} {$i < 16} {incr i} {
	set PatchMeshAttrData(Basis_U_${i}) 0.0
	set PatchMeshAttrData(Basis_V_${i}) 0.0
    }

 return;
}
# init_PatchMesh

# pamesh_updateWHL:
# helper to increase/decrease a width/height/length value in step amounts
# never decreases below 4
proc pamesh_updateWHL { w prop name btype op } {
    global $prop

    set f $w.f${name}

    set oldval [$f.e get]

    # need to derive the step from the type, as getProp is lazy
    set step 3
    switch $btype {
	1 { set step 1 }
	2 { set step 1 }
	3 { set step 2 }
	4 { set step 4 }
	5 {
	    if { $name == "Length" } {
		set step $prop(Step)
	    } else {
		if { $name == "Width" } {
		    set step $prop(Step_U)
		} else {
		    set step $prop(Step_V)
		}
	    }
	}
    }

    switch $op {
	"p" {
	    set newval [expr $oldval + $step]
	}
	"m" {
	    set newval [expr $oldval - $step]
	    if { $newval < 4 } {
		set newval 4
	    }
	}
    }

    set ${prop}(${name}) $newval

 return;
}
# pamesh_updateWHL

# pamesh_getAttr:
#  get Attributes from C context and build new PropertyGUI
#
proc pamesh_getAttr { } {
    global ay PatchMeshAttr PatchMeshAttrData

    set oldfocus [focus]

    # remove old, create new PatchMeshAttr-UI
    catch {destroy $ay(pca).$PatchMeshAttr(w)}
    set w [frame $ay(pca).$PatchMeshAttr(w)]
    set a $PatchMeshAttr(arr)

    set ay(bok) $ay(appb)
    addVSpace $w s1 2
    addMenu $w $a Type [list Bilinear Bicubic]
    addParam $w $a Width
    addParam $w $a Height
    addCheck $w $a Close_U
    addCheck $w $a Close_V

    addInfo $w $a IsRat

    if { $PatchMeshAttrData(Type) == 1 } {
	set l [list Bezier B-Spline CatmullRom Hermite Power Custom]
	addMenu $w $a BType_U $l
	addMenu $w $a BType_V $l

	if { $PatchMeshAttrData(BType_U) == 5 } {
	    addParam $w $a Step_U
	    addMatrix $w $a Basis_U
	}

	if { $PatchMeshAttrData(BType_V) == 5 } {
	    addParam $w $a Step_V
	    addMatrix $w $a Basis_V
	}
    }

    addParam $w $a Tolerance
    addMenu $w $a DisplayMode $ay(npdisplaymodes)

    # advanced bindings for Width/Height manipulation
    if { $PatchMeshAttrData(Type) == 1 } {
    bind $w.fWidth.b1 <Control-ButtonPress-1> "pamesh_updateWHL $w $a Width \$PatchMeshAttrData(BType_U) m;break"
    bind $w.fWidth.b2 <Control-ButtonPress-1> "pamesh_updateWHL $w $a Width \$PatchMeshAttrData(BType_U) p;break"
    bind $w.fHeight.b1 <Control-ButtonPress-1> "pamesh_updateWHL $w $a Height \$PatchMeshAttrData(BType_V) m;break"
    bind $w.fHeight.b2 <Control-ButtonPress-1> "pamesh_updateWHL $w $a Height \$PatchMeshAttrData(BType_V) p;break"
    }

    getProp

    # add UI to property canvas
    plb_setwin $w $oldfocus

 return;
}
# pamesh_getAttr

