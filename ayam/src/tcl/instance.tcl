# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2001 by Randolf Schultz
# (rschultz@informatik.uni-rostock.de) and others.
#
# All rights reserved.
#
# See the file License for details.

# instance.tcl - instance objects Tcl code

set Instance_props { Transformations }

# instance_edit:
#  this should be really named master_edit, as it finds the
#  master of an instance object and selects it for editing
proc instance_edit { } {
    global ay ay_error sel type matlevel matobject

    set sel ""
    getSel sel
    if { $sel == "" } {
	ayError 2 editMaster "No object selected!"
	return;
    }

    # we only work with one object
    #selOb $sel
    update
    set type ""
    getType type
    if { $type != "Instance" } {
	ayError 2 editMaster "Object is not an instance!"
	return;
    }


    if { $ay(lb) == 1 } {
	set ay(CurrentLevel) "root"
    }

    # get master object
    set mo ""

    getMaster mo

    if { $mo != "" } {

	goTop

	set len [llength $mo]
	set node "root"
	if { $len > 1 } {
	    set mo1 [lrange $mo 0 [expr $len - 2]]
	    set first 1
	    foreach l $mo1 {
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
	    # foreach
	}
	# if
	set o [lindex $mo end]
	if { $ay(lb) == 0 } {
	    if { $len > 1 } {
		selOb [expr $o + 1]
	    } else {
		selOb $o
	    }
	    $ay(tree) selection set ${node}:$o
	    $ay(tree) see ${node}:$o
	    tree_paintLevel ${node}
	    update
	} else {
	    uS
	    selOb [expr $o + 1]
	    $ay(olb) selection set [expr $o + 1]
	}
	# if
	plb_update
    } else {
	ayError 2 editMaster "Could not find master object!"
    }
    # if

 return;
}
#  instance_edit