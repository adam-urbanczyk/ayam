# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2001 by Randolf Schultz
# (rschultz@informatik.uni-rostock.de) and others.
#
# All rights reserved.
#
# See the file License for details.

# material.tcl - material objects Tcl code

set Material_props { RiAttributes Tags Surface Displacement Interior\
	Exterior MaterialAttr }

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
    global ay ay_error

    set w .createMw
    catch {destroy $w}
    toplevel $w -class ayam
    wm title $w "Create Material"
    wm iconname $w "Ayam"
    wm transient $w .

    
    set f [frame $w.f1]
    pack $f -in $w -side top -fill x

    label $f.l -text "Name:"
    entry $f.e -width 20
    pack $f.l $f.e -side left -fill x

    set f [frame $w.f2]
    button $f.bok -text "Ok" -width 5 -command { 
	global ay ayprefs ay_error
	set ay_error 0

	regsub -all " " [.createMw.f1.e get] "_" temp
	crtOb Material $temp

	if { $ay_error == 0 } {
	    focus .
	    grab release .createMw
	    destroy .createMw
	    uCR; sL; rV;
	}
    }

    button $f.bca -text "Cancel" -width 5 -command "focus .;destroy $w"

    pack $f.bok $f.bca -in $f -side left -fill x -expand yes
    pack $f -in $w -side bottom -fill x

    set ay_error 1

    winCenter $w
    grab $w
    focus $w.f1.e
    tkwait window $w

 return;
}
# material_createp


# material_edit:
#  creates or edits material object of currently selected
#  normal object (bound to main menu entry Edit/Material)
proc material_edit { } {
    global ay ay_error matPropData sel matlevel matobject

    set sel ""
    getSel sel
    if { $sel == "" } {
	ayError 2 editMaterial "No object selected!"
	return;
    }

    if { $ay(lb) == 1 } { 

	if { [$ay(olb) get 0] == ".." } {
	    set sel [expr [lindex $sel 0] + 1]
	} else {
	    set sel [lindex $sel 0]
	}

    } else {

	if { $ay(CurrentLevel) != "root" } {
	    set sel [expr [lindex $sel 0] + 1]
	} else {
	    set sel [lindex $sel 0]
	}
    }

    # we only work with one object
    selOb $sel

    getType type
    global ${type}_props
    eval set props \$${type}_props

    if { [lsearch -exact $props Material]  == -1 } {
	ayError 2 editMaterial "Object may not be associated with a material!"
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

	# if a new material has been created by the user
	# we link it to the object and select it again for editing
	if { $ay_error > 0 } { return; }
	getProp
	set newmaterial $MaterialAttrData(Materialname)
	selOb $sel
	set matPropData(Materialname) $newmaterial
	setMat
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

	forAllT material 1 {
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
		if { $matlevel != "root" } {
		    selOb [expr $matobject + 1]
		} else {
		    selOb [expr $matobject]
		}
		$ay(tree) selection set ${matlevel}:$matobject
		$ay(tree) see ${matlevel}:$matobject
		tree_paintLevel ${matlevel}
		update
	    } else {
		uS
		selOb [expr $matobject + 1]
		$ay(olb) selection set [expr $matobject + 1]
	    }
	    # if
	    plb_update
	
	} else {
	    ayError 2 matEdit "Could not find material ${mat}!"
	}
	# if
    }
    # if

 return;
}
# material_edit

# MaterialAttr Property:

proc setMaterialAttrp { } {
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


# material_highlight:
#  highlights tree nodes of objects of same material (colouring them red)
#  needs a selected material or normal object from which the material
#  name to search for is derived
proc material_highlight { } {
    global ay MaterialAttrData matPropData otype mat sel oldslevel

    # check, whether tree view is open
    if { $ay(lb) != 0 } {
	ayError 2 highlightMaterial\
	    "This command only works for the tree view!"
	return;
    }

    # check/save original selection
    set sel ""
    getSel sel
    if { $sel == "" } {
	ayError 2 highlightMaterial "No object selected!"
	return;
    }

    # get type of selected object
    getType otype
    if { $otype == "Material" } {
	set MaterialAttrData(Materialname) ""
	getProp
	# search only for materials that are registered and have
	# objects in the scene made of this material
	if { $MaterialAttrData(Registered) != "Yes." } {
	  ayError 2 highlightMaterial\
	    "Material is not registered!"
	  return;
	}
	if { $MaterialAttrData(RefCount) == 0 } {
	  ayError 2 highlightMaterial\
	    "No objects of this Material exist!"
	  return;
	}
	set mat $MaterialAttrData(Materialname)
    } else {
	set matPropData(Materialname) ""
	getMat
	set mat $matPropData(Materialname)
    }
    # if

    if { $mat == "" } { return; }

    ayError 4 highlightMaterial\
	    "Searching for objects of material \\\"$mat\\\" ..."
    update

    # save old selection state
    set oldclevel $ay(CurrentLevel)
    set oldslevel $ay(SelectedLevel)
    set oldselection [$ay(tree) selection get]
    # go to top level and clear selection
    set ay(CurrentLevel) "root"
    set ay(SelectedLevel) "root"
    goTop
    $ay(tree) selection clear
    selOb

    global matsfound
    set matsfound 0
    # now find and highlight normal objects of material <mat>
    forAll 1 {
	global ay mat matPropData i matsfound oldslevel
	set matPropData(Materialname) ""
	getMat
	update
	set ni [ expr $i - 1 ]
	if { $matPropData(Materialname) == $mat } {
	    incr matsfound
	    tree_openTree $ay(tree) $ay(CurrentLevel)
	    update
	    $ay(tree) itemconfigure ${ay(CurrentLevel)}:$ni -fill red
	} else {
	    if { $ay(CurrentLevel) != $oldslevel } {
		$ay(tree) itemconfigure ${ay(CurrentLevel)}:$ni -fill darkgrey
	    } else {
		$ay(tree) itemconfigure ${ay(CurrentLevel)}:$ni -fill black
	    }
	}
	# if
    }
    # forAll

    ayError 4 highlightMaterial "Done. Found $matsfound objects."
    unset matsfound

    # re-establish old selection
    set ay(CurrentLevel) $oldclevel
    set ay(SelectedLevel) $oldslevel
    $ay(tree) selection set $oldselection
    tree_handleSelection
    plb_update

 return;
}
# material_highlight
