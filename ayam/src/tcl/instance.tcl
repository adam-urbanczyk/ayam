# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2001 by Randolf Schultz
# (randolf.schultz@gmail.com) and others.
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
    # => trim selection
    if { [llength $sel] > 1 } {
	selOb [lindex $sel 1]
    }

    set type ""
    getType type
    if { $type != "Instance" } {
	ayError 2 editMaster "Object is not an instance!"
	# restore original selection
	selOb $sel
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
	set mo [string range $mo 5 end]
	set mo [string map {: " "} $mo]
	set len [llength $mo]
	set node "root"
	if { $len > 1 } {
	    set mo1 [lrange $mo 0 [expr $len - 2]]
	    set first 1
	    foreach l $mo1 {
		if { $first == 1 } {
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
	    # TreeView is active
	    selOb $o
	    $ay(tree) selection set ${node}:$o
	    $ay(tree) see ${node}:$o
	    tree_paintLevel $node
	    set ay(CurrentLevel) $node
	    set ay(SelectedLevel) $node
	    update
	} else {
	    # ListBox is active
	    uS
	    selOb $o
	    if { $len > 1 } {
		$ay(olb) selection set [expr $o + 1]
	    } else {
		$ay(olb) selection set $o
	    }
	}
	# if
	plb_update
    } else {
	ayError 2 editMaster "Could not find master object!"
	# restore original selection
	selOb $sel
    }
    # if

 return;
}
#  instance_edit


# crtInstances:
#  helper to create instances from all selected objects
#
proc crtInstances { } {
    set selection ""
    getSel selection
    if { $selection == "" } {
	ayError 2 "crtInstances" "Please select an object!"
    } else {
	forAll -r 0 { crtOb Instance; }
    }
 return;
}
# crtInstances