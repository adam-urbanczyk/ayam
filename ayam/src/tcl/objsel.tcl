# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2001 by Randolf Schultz
# (randolf.schultz@gmail.com) and others.
#
# All rights reserved.
#
# See the file License for details.

# objsel.tcl - scripts for object selection on viewport

array set rArray {
    lb ""
    yscroll ""
    xscroll ""
    selection ""
    oldLevel ""
    oldSelection ""
    result ""
    cycleList ""
    cycleIndex 0
}

#reconsiderObjSel:
# This function helps the user to solve ambiguity when he/she clicks an object
# on a viewport. Sometimes several objects are good candidates to be selected
# (this can happen when many objects are located very close together) but only
# one have to be added to the selection.
# By default the first candidate is added to the selection. But the user may
# want another object among the candidates to be retained. This function allows
# the user to reconsider (hence the name) the selected candidate.
proc reconsiderObjSel { Selection } {
    global rArray ay ayprefs

    set w .reconsider

    if { [winfo exists $w] } {
	update
	return
    }

    winAutoFocusOff

    # Save current values
    set rArray(selection) $Selection
    set rArray(oldLevel) $ay(CurrentLevel)
    set rArray(oldSelection) ""
    getSel rArray(oldSelection)

    # Create the window
    toplevel $w -class Ayam
    wm title $w "Ambiguous Pick"
    wm iconname $w "Ayam"
    if { $ay(ws) == "Aqua" } {
	winMakeFloat $w
    } else {
	if { $ay(currentView) != "" } {
	    wm transient $w [winfo toplevel $ay(currentView)]
	} else {
	    wm transient $w .
	}
    }

    # if we get shuffled under, silently go away after 0.5s
    # XXXX add warning message for the user, if this happens?
    if { $ayprefs(AutoCloseDialogs) == 1 } {
	bind $w <Visibility> {
	    if { "%s" == "VisibilityFullyObscured" } {
		bind %W <Visibility> {}
		after 500 {catch {.reconsider.f2.bca invoke}}
	    }
	}
    }

    # Create a frame where to place the listbox and the scrollbar
    set f [frame $w.f1]
    pack $f -in $w -side top -fill both -expand yes

    # Create the listbox
    listbox $f.lo -height 8 -selectmode browse -activestyle none \
	-exportselection 0 \
	-yscrollcommand {global rArray; $rArray(yscroll) set} \
	-xscrollcommand {global rArray; $rArray(xscroll) set}

    set entry ""
    set maxlen 0
    foreach i $Selection {

	# Computes the path of each object of the selection
	set object [split $i :]
	set level ""
	set path ""
	foreach i $object {
	    lappend level $i
	    if { $i != "root" } {
		getNameFromNode name [join $level ":"]
		lappend path $name
	    }
	}

	lappend entry [join $path "/"]
	set len [string length $path]
	if { $len > $maxlen } { set maxlen $len }
    }

    $f.lo configure -width ${maxlen}
    $f.lo delete 0 end
    eval [subst "$f.lo insert end $entry"]

    # Create the vertical scrollbar
    scrollbar $f.sv -command {global rArray; $rArray(lb) yview} -takefocus 0

    # Create the horizontal scrollbar
    scrollbar $f.sh -command {global rArray; $rArray(lb) xview} \
    	-takefocus 0 -orient h

    # Uses a grid to manage widgets (listbox, horizontal & vertical scrollbars)
    grid $f.lo $f.sv -sticky news
    grid $f.sh -sticky ew
    grid columnconfig $f 0 -weight 1
    grid rowconfig $f 0 -weight 1

    # Save the listbox and scrollbar ids
    set rArray(yscroll) $f.sv
    set rArray(xscroll) $f.sh
    set rArray(lb) $f.lo

    # Create a frame where to place buttons "Ok" and "Cancel"
    set f [frame $w.f2]

    # Button Ok
    button $f.bok -text "Ok" -width 5 -command {
	global rArray ay

	# Store the selected item into rArray(result)
	set item [$rArray(lb) curselection]
	set rArray(result) [lindex $rArray(selection) $item]

	# Restore the current level so that the tree knows that the
	# level has eventually changed
	set ay(CurrentLevel) $rArray(oldLevel)

	if { $ay(lb) == 0 } {
	    tree_handleSelection
	}

	focus $ay(currentView)
	destroy .reconsider
    }

    # Button Cancel
    button $f.bca -text "Cancel" -width 5 -command {
	global rArray ay

	set selection ""
	set rArray(result) ""
	# Restore previous state
	selOb

	if { $rArray(oldLevel) != "root" } {
	    foreach i $rArray(oldSelection) {
		lappend selection [expr $i + 1]
	    }
	    set rArray(oldSelection) $selection
	}
	append rArray(oldLevel) ":"
	append rArray(oldLevel) [lindex $rArray(oldSelection) 0]
	goLevObjSel $rArray(oldLevel)
	selOb $rArray(oldSelection)

	if { $ay(lb) == 1 } {
	    olb_select
	} else {
	    tree_handleSelection
	}
	focus $ay(currentView)
	destroy .reconsider
    }
    pack $f.bok $f.bca -in $f -side left -fill x -expand yes
    pack $f -in $w -side bottom -fill x

    bind $rArray(lb) <<ListboxSelect>> {
	selOb
	set i [$rArray(lb) curselection]
	if { $i != "" } {
	    # Get the node selected by the user
	    set node [lindex $rArray(selection) $i]
	    # Go to the corresponding level
	    goLevObjSel $node

	    # Get the selected item
	    set object [split $node :]
	    set item [lindex $object end]

	    # Put the item in the selection then update the views
	    selOb $item
	}
	after idle {rV}
    }
    bind $rArray(lb) <Double-1> {
	after 200 {.reconsider.f2.bok invoke}
    }
    bind $rArray(lb) <Home>\
	"%W see 0 ;%W selection clear 0 end; %W selection set 0;\
         %W activate 0; event generate %W <<ListboxSelect>>; break"
    bind $rArray(lb) <End>\
	"%W see end; %W selection clear 0 end; %W selection set end;\
         %W activate end; event generate %W <<ListboxSelect>>; break"

    # Get the default node (i.e. the first in the list)
    set node [lindex $Selection 0]
    # Go to the corresponding level
    goLevObjSel $node

    # Get the selected item
    set object [split $node :]
    set item [lindex $object end]

    # Put the first item in the selection then update the views
    selOb
    selOb $item
    $w.f1.lo selection set 0
    $w.f1.lo activate 0
    $w.f1.lo see 0
    after idle {rV}

    # Esc-key && close via window decoration == Cancel button
    bind $w <Escape> "$f.bca invoke"
    wm protocol $w WM_DELETE_WINDOW "$f.bca invoke"

    bind $w <Key-Return> "$f.bok invoke"
    catch { bind $w <Key-KP_Enter> "$f.bok invoke" }

    focus $w.f1.lo

    winToMouse $w

    tkwait window $w

    winAutoFocusOn

    update

 return;
}
# reconsiderObjSel

#cleanObjSel:
# This function is called when picked objects have to be added to the selection
# (i.e. when the user pressed <Ctrl> during the picking). Its purpose is
# to remove the following objects from the list of picked objects:
# - objects that are already selected
# - objects that are from a different level than the selected ones
proc cleanObjSel { newSelection oldSelection } {
    global ay
    set cleanSelection ""
    set report 0
    if { $ay(lb) == 1 } {
	# For each item picked by the user :
	foreach node $newSelection {
	    set hierarchy [split $node :]
	    # Is the item in the current level ?
	    if { [join [lrange $hierarchy 0 end-1] :] == $ay(CurrentLevel) } {
		set item [lindex $hierarchy end]
		# Because of the '..' in the list we have to increment
		# the item entry, except for the first level where 'root'
		# is actually entry 0.
		if { $ay(CurrentLevel) != "root" } {
		    set item [expr $item + 1]
		}
		# Is the item already stored in the current selection ?
		if { [lsearch -exact $oldSelection $item] == -1} {
		    lappend cleanSelection $node
		} else {
		    set report 1
		}
	    }
	}
    } else {
	# For each item picked by the user :
	foreach node $newSelection {
	    # Is the item already stored in the current selection ?
	    if { [lsearch -exact $oldSelection $node] == -1 } {
		# Is the item in the current level ?
		if { $ay(SelectedLevel) == [$ay(tree) parent $node] } {
		    lappend cleanSelection $node
		} else {
		    set report 1
		}
	    }
	}
    }

    if { $oldSelection != "" && $report } {
	ayError 1 "objSel" "Can not select from different levels!"
    }

 return $cleanSelection
}
#cleanObjSel

#goLevObjSel:
# Goes to the level which name (in tree format) is given by 'node'
proc goLevObjSel { node } {

    # Because of the '..' in the list we have to increment each entry number
    # in the hierarchy, except for the first level where 'root' is actually
    # entry 0.
    set tmp [split $node :]
    set hierarchy [lindex $tmp 1]
    foreach i [lrange $tmp 2 end] {
	lappend hierarchy [expr $i + 1]
    }

    # Goes down into the list hierarchy until we reach the desired level
    goTop
    set end [expr [llength $hierarchy] -2]
    foreach i [lrange $hierarchy 0 $end] {
        goDown $i
    }

    return [lindex $hierarchy end]
}
#goLevObjSel

#listBoxObjSel:
# Updates the List Box according to the selected item
proc listBoxObjSel { Selection } {
    global ay

    set lb $ay(olb)
    # Goes to the level of 'Selection' then stores the item' id
    set item [goLevObjSel $Selection]

    # Updates the list box to reflect the level change
    $lb delete 0 end
    getLevel curlevel
    eval [subst "$lb insert end $curlevel"]

    return $item
}

proc treeObjSel { nodes } {
    global ay

set tr $ay(tree)
    tree_openTree $tr [$tr parent [lindex $nodes end]]

    eval [subst "$tr selection set $nodes"]
    tree_handleSelection
    update
    plb_update
    update idletasks

    if { $ay(need_redraw) == 1 } {
	rV
    }

    $tr see [lindex $nodes end]

 return;
}


#singleObjSel:
# Replace the current selection by one picked object.
proc singleObjSel { node } {
    global ay rArray ayprefs

    if { $node != "" } {
	set newSelection [split $node " "]
	set singleSelection [lindex $newSelection 0]

	# If the user has picked several objects then reconsider...
	if {[llength $newSelection] > 1} {
	    if { ($ayprefs(PickCycle) == 0) ||
		 ([llength $newSelection] > $ayprefs(PickCycleMax)) } {
		# ask
		set rArray(result) ""
		reconsiderObjSel $newSelection
		if { $rArray(result) != "" } {
		    set singleSelection $rArray(result)
		} else {
		    return;
		}
	    } else {
		# cycle
		if { $rArray(cycleList) == $newSelection } {
		    if { $rArray(cycleIndex) >= [llength $rArray(cycleList)] } {
			set rArray(cycleIndex) 0
		    }
		  set singleSelection [lindex $newSelection $rArray(cycleIndex)]
		}
		set rArray(cycleList) $newSelection
		incr rArray(cycleIndex)
	    }
	}

	if {$ay(lb) == 1} {
	    set lb $ay(olb)
	    $lb selection clear 0 end
	    set item [listBoxObjSel $singleSelection]
	    $lb selection set $item
	    $lb see $item
	    olb_select
	} else {
	    tree_openTree $ay(tree) $singleSelection
	    tree_selectItem $ay(tree) $singleSelection
	    $ay(tree) see $singleSelection
	}
    }

 return;
}
#singleObjSel

#multipleObjSel:
# Replace the current selection by the multiple objects picked
proc multipleObjSel { node } {
    global ay

    if { $node != "" } {
	set newSelection [split $node " "]

	# go to the level of the first selected object
	set firstSelection [lindex $newSelection 0]
	set hierarchy [split $firstSelection :]
	set ay(CurrentLevel) [join [lrange $hierarchy 0 end-1] :]
	set ay(SelectedLevel) $ay(CurrentLevel)
	goLevObjSel $firstSelection

	# Remove items that do not belong to the (new) current level
	set cleanSelection [cleanObjSel $newSelection ""]

	if { $cleanSelection != "" } {
	    if { $ay(lb) == 1 } {
	        set lb $ay(olb)
		# Empty the current selection
		selOb

		# Unselect all items of the listbox and replace them by
		# the new selection
		$lb selection clear 0 end
		foreach node $cleanSelection {
		    set item [lindex [split $node :] end]
		    # Because of the '..' in the list we have to increment
		    # the item entry, except for the first level where 'root'
		    # is actually entry 0.
		    if { $ay(CurrentLevel) != "root" } {
			set item [expr $item + 1]
		    }
		    $lb selection set $item
		}

		# Scroll the listbox so that selected items are visible
		$lb see $item
		olb_select
	    } else {
		$ay(tree) selection clear
		treeObjSel $cleanSelection
	    }
	}
    }

 return;
}
#multipleObjSel

#addObjSel:
# Add one picked object to the current selection
proc addObjSel { node } {
    global ay rArray

    if { $node != "" } {
	set newSelection [split $node " "]
	if { $ay(lb) == 1} {
	    set oldSelection [$ay(olb) curselection]
	} else {
	    set oldSelection [$ay(tree) selection get]
	}

	# Remove already selected items and items that do not belong to the
	# current level
	set cleanSelection [cleanObjSel $newSelection $oldSelection]

	set Selection [lindex $ay(LastSelection) 0]

	# If the user has picked items that are already selected or that
	# do not belong to the current level then $ay(Selection) is empty
	# (it has been emptied by cleanObjSel) else we can process the user's
	# picking
	if { $cleanSelection != "" } {

	    #If there was no previously selected items then we fall back
	    # in a regular "single object selection"
	    if { $oldSelection == "" } {
		singleObjSel $node
		return
	    }

	    # If the user has picked several objects then reconsider...
	    if {[llength $cleanSelection] > 1} {
		set oldSel ""
		# Save the current selection
		getSel oldSel

		# Reconsider the ambiguous picking
		reconsiderObjSel $cleanSelection
		if { $rArray(result) != "" } {
		    set cleanSelection $rArray(result)
		} else {
		    return
		}

		# Restore the previous selection
		selOb
		selOb $oldSel
	    }

	    if { $ay(lb) == 1 } {
		set item [lindex [split $cleanSelection :] end]
		# Because of the '..' in the list we have to increment
		# the item entry, except for the first level where 'root'
		# is actually entry 0.
		if { $ay(CurrentLevel) != "root" } {
		    set item [expr $item + 1]
		}
		$ay(olb) selection set $item
		# Scroll the listbox so that the selected item is visible
		$ay(olb) see $item
		olb_select
	    } else {
		tree_toggleSelection $ay(tree) $cleanSelection
		# Scroll the tree so that the selected item is visible
		$ay(tree) see $cleanSelection
	    }
	}
    }

 return;
}
#addObjSel

#addMultipleObjSel:
# Add several picked objects to the selection
proc addMultipleObjSel { node } {
    global ay

    if { $node != "" } {
	set newSelection [split $node " "]

	if { $ay(lb) == 1} {
	    set oldSelection [$ay(olb) curselection]
	} else {
	    set oldSelection [$ay(tree) selection get]
	}

	set cleanSelection [cleanObjSel $newSelection $oldSelection]

	if { $cleanSelection != "" } {

	    #If there was no previously selected items then we fall back
	    # in a regular "multiple object selection"
	    if { $oldSelection == "" } {
		multipleObjSel $node
		return
	    }

	    if { $ay(lb) == 1 } {
		foreach node $cleanSelection {
		    set item [lindex [split $node :] end]
		    # Because of the '..' in the list we have to increment
		    # the item entry, except for the first level where 'root'
		    # is actually entry 0.
		    if { $ay(CurrentLevel) != "root" } {
			set item [expr $item + 1]
		    }
		    $ay(olb) selection set $item
		}
		# Scroll the listbox so that selected items are visible
		$ay(olb) see $item
		olb_select
	    } else {
		treeObjSel $cleanSelection
	    }
	}
    }

 return;
}
#addMultipleObjSel
