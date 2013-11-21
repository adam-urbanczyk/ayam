# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2001 by Randolf Schultz
# (randolf.schultz@gmail.com) and others.
#
# All rights reserved.
#
# See the file License for details.

# material.tcl - material objects Tcl code

set Material_props { RiAttributes Tags Surface Displacement Interior Exterior MaterialAttr }

# RiAttributes Property:

proc setRiAttrp { } {
    setProp
    uCL cl "0 1"
 return;
}

array set RiAttributes {
arr   RiAttrData
sproc setRiAttrp
gproc ""
w     fRiAttr
}

array set RiAttrData {
    Interpolation 0
    BoundCoord 0
    CastShadows 0
    Sides 0
}

# create RiAttributes-UI
set w [frame $ay(pca).$RiAttributes(w)]
addVSpace $w s1 2
addColor $w RiAttrData Color
addColor $w RiAttrData Opacity

addText $w e3 "Shading:"
addParam $w RiAttrData ShadingRate
addMenu $w RiAttrData Interpolation [list Constant Smooth]
addMenu $w RiAttrData Sides [list DontSet 1 2]

addText $w e4 "Displacement:"
addMenu $w RiAttrData BoundCoord [list Object Shader Camera]
addParam $w RiAttrData BoundVal

addText $w e5 "BMRT-Specific:"
addCheck $w RiAttrData TrueDisp
addMenu $w RiAttrData CastShadows [list Os None Opaque Shade]

addText $w e6 "Visibility:"
addCheck $w RiAttrData Camera
addCheck $w RiAttrData Reflection
addCheck $w RiAttrData Shadow


# material_createp:
#  helper procedure, invoked on "create material",
#  asks for a new material name, checks it and
#  finally creates the new material object
proc material_createp { } {
    global ay ayprefs ay_error

    winAutoFocusOff

    set ay(createMFocus) [focus]

    set w .createMw
    set t "Create Material"
    winDialog $w $t

    set f [frame $w.f1]
    pack $f -in $w -side top -fill x

    set ay(bca) $w.f2.bca
    set ay(bok) $w.f2.bok
    if { $ayprefs(FixDialogTitles) == 1 } {
	addText $w.f1 e0 "Create Material:"
    }
    addString $w.f1 aydd Name

    set f [frame $w.f2]
    button $f.bok -text "Ok" -width 5 -command {
	global ay aydd ayprefs ay_error

	# replace all whitespace by underscores
	regsub -all "\[\[:space:\]\[:cntrl:\]\]" $aydd(Name) "_" temp

	set ay_error 0

	crtOb Material $temp

	if { $ay_error == 0 } {
	    grab release .createMw
	    destroy .createMw
	    uCR; sL;
	    restoreFocus $ay(createMFocus)
	}
    }

    button $f.bca -text "Cancel" -width 5 -command "\
      grab release .createMw;\
      restoreFocus $ay(createMFocus);\
      destroy $w"

    pack $f.bok $f.bca -in $f -side left -fill x -expand yes
    pack $f -in $w -side bottom -fill x

    # Esc-key && close via window decoration == Cancel button
    bind $w <Escape> "$f.bca invoke"
    wm protocol $w WM_DELETE_WINDOW "$f.bca invoke"

    winRestoreOrCenter $w $t
    grab $w
    focus $w.f1.fName.e
    tkwait window $w

    winAutoFocusOn

 return;
}
# material_createp


# material_edit:
#  creates or edits material object of currently selected
#  normal object (bound to main menu entry Edit/Material, shortcut <Ctrl-m>)
proc material_edit { } {
    global ay ay_error matPropData sel matlevel matobject

    set sel ""
    getSel sel
    if { $sel == "" } {
	ayError 2 editMaterial "No object selected!"
	return;
    }

    set oldsel $sel

    # we only work with one object
    # => trim selection
    if { [llength $sel] > 1 } {
	selOb [lindex $sel 1]
    }

    getType type
    global ${type}_props
    if { ! [info exists ${type}_props] } {
	init_${type}
    }
    eval set props \$${type}_props

    if { [lsearch -exact $props Material]  == -1 } {
	ayError 2 editMaterial "Object may not be associated with a material!"
	# restore original selection
	selOb $sel
	return;
    }

    # get name of current material
    getMat
    global mat
    set mat $matPropData(Materialname)

    if { $mat == "" } {
	# there is no material currently associated with this
	# object; therefore, we create a new material
	global MaterialAttrData
	set ay_error 0
	material_createp

	if { $ay_error > 0 } {
	    # restore original selection
	    selOb $sel
	    return;
	}

	# if a new material has been created by the user
	# we link it to all the originally selected object(s)
	getProp
	set newmaterial $MaterialAttrData(Materialname)
	foreach obj $oldsel {
	    selOb $obj
	    getMat
	    set matPropData(Materialname) $newmaterial
	    setMat
	}
	# select the newly created material again for editing
	sL;rV

    } else {
	# search material object
	goTop
	set ay(CurrentLevel) "root"
	selOb
	uCL cs

	set matobject ""
	set matlevel ""
	if { $ay(lb) == 1 } {
	    set ay(CurrentLevel) "root"
	}

	forAll -type material {
	    global ay mat MaterialAttrData matlevel matobject

	    getProp

	    if { $MaterialAttrData(Materialname) == $mat } {
		set matlevel $ay(CurrentLevel)
		getSel matobject
		return -1;
	    }
	    return 1;
	}
	# forAll

	if { $matobject != "" } {
	    set node "root"
	    if { $matlevel != "root" } {
		regsub -all ":" $matlevel " " level
		set first 1
		foreach l $level {
		    if { $l != "root" } {
			if { $first == 1 || $ay(lb) == 1 } {
			    set first 0
			    goDown $l
			} else {
			    goDown [expr $l + 1]
			}
			append node ":${l}"
			if { $ay(lb) == 0 } {
			    $ay(tree) itemconfigure $node -open 1
			}
			# if
		    }
		    # if
		}
		# foreach
	    }
	    # if

	    if { $ay(lb) == 0 } {
		# TreeView is active
		selOb $matobject
		$ay(tree) selection set ${matlevel}:$matobject
		$ay(tree) see ${matlevel}:$matobject
		tree_paintLevel $matlevel
		set ay(CurrentLevel) $matlevel
		set ay(SelectedLevel) $matlevel
		update
	    } else {
		# ListBox is active
		uS
		selOb $matobject
		$ay(olb) selection set [expr $matobject + 1]
	    }
	    # if
	    plb_update

	} else {
	    ayError 2 matEdit "Could not find material ${mat}!"
	    # restore original selection
	    selOb $sel
	}
	# if
    }
    # if

 return;
}
# material_edit

# MaterialAttr Property:

proc setMaterialAttrp { } {
    global MaterialAttrData

    # replace all whitespace/control characters by underscores
    regsub -all "\[\[:space:\]\[:cntrl:\]\]" $MaterialAttrData(Materialname)\
	"_" MaterialAttrData(Materialname)

    setProp
    uCL cl "0 1"
 return;
}

array set MaterialAttr {
    arr   MaterialAttrData
    sproc setMaterialAttrp
    gproc ""
    w     fMaterialAttr
}

array set MaterialAttrData {
    dummy 0
}

# create MaterialAttr-UI
set w [frame $ay(pca).$MaterialAttr(w)]

addString $w MaterialAttrData Materialname
addInfo $w MaterialAttrData RefCount
addInfo $w MaterialAttrData Registered
