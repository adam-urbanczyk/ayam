# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2001 by Randolf Schultz
# (rschultz@informatik.uni-rostock.de) and others.
#
# All rights reserved.
#
# See the file License for details.

# material.tcl - material objects Tcl code

set Material_props { RiAttributes Tags Surface Displacement Interior Exterior }

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
}

# create RiAttributes-UI
set w [frame $ay(pca).$RiAttributes(w)]

addString $w RiAttrData Materialname
addInfo $w RiAttrData RefCount
addInfo $w RiAttrData Registered
addColor $w RiAttrData Color
addColor $w RiAttrData Opacity

addText $w e3 "Shading:"
addParam $w RiAttrData ShadingRate
addMenu $w RiAttrData Interpolation [list Constant Smooth]

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


proc material_createp { } {
    global ay ay_error

    set w .createMw
    catch {destroy $w}
    toplevel $w
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

    button $f.bca -text "Cancel" -width 5 -command "destroy $w"

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
#
#
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
    set mat $matPropData(Name)

    if { $mat == "" } {
	# there is no material currently associated with this
	# object; therefore, we create a new material
	global RiAttrData
	set ay_error 0
	material_createp

	# if a new material has been created by the user
	# we link it to the object and select it again for editing
	if { $ay_error > 0 } { return; }
	getProp
	set newmaterial $RiAttrData(Materialname)
	selOb $sel
	set matPropData(Name) $newmaterial
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
	    global ay mat RiAttrData matlevel matobject

	    getProp

	    if { $RiAttrData(Materialname) == $mat } {
		set matlevel $ay(CurrentLevel)
		getSel matobject
		return -1;
	    }
	    return 1;
	}
	# forAll

	if { $matobject != "" } {

	    if { $matlevel != "root" } {
		regsub -all ":" $matlevel " " level
		set first 1
		set node "root"
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
		selOb [expr $matobject + 1]
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
#  material_edit