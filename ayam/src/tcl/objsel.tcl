# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2001 by Randolf Schultz
# (rschultz@informatik.uni-rostock.de) and others.
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
}

#reconsider:
# This function helps the user to solve ambiguity when he/she clicks an object
# on a viewport. Sometimes several objects are good candidates to be selected
# (this can happen when many objects are located very close together) but only
# one have to be added to the selection.
# By default the first candidate is added to the selection. But the user may
# want another object among the candidates to be retained. This function allows
# the user to reconsider (hence the name) the selected candidate.
proc reconsider { Selection } {
    global rArray ay
    
    set w .reconsider

    if { [winfo exists $w] } {
	update
	return
    }
    
    # Save current values
    set rArray(selection) $Selection
    set rArray(oldLevel) $ay(CurrentLevel)
    set rArray(oldSelection) ""
    getSel rArray(oldSelection)
    
    # Create the window
    toplevel $w
    wm title $w "Ambiguous Pick"
    wm iconname $w "Ayam"
    wm transient $w .
    winCenter $w
    
    # Create a frame where to place the listbox and the scrollbar
    set f [frame $w.f1]
    pack $f -in $w -side top -fill both -expand yes
    
    # Create the listbox
    listbox $f.lo -height 5 -selectmode single -exportselection 0 \
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
    eval [ subst "$f.lo insert end $entry"]
    $f.lo selection set 0
    
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
    
    bind $rArray(lb) <ButtonRelease-1> {
	# Get the node selected by the user
	set node [lindex $rArray(selection) [$rArray(lb) curselection]]
	# Go to the corresponding level
	goLevObjSel $node
	
	# Convert 'tree format' node name into 'listbox format' node name
	set object [split $node :]
	set item [lindex $object end]
	if { [llength $object] > 2 } {
	    set item [expr $item + 1]
	}
	
	# Put the item in the selection then update the views
	selOb
	selOb $item
	rV
    }
    
    # Get the default node (i.e. the first in the list)
    set node [lindex $Selection 0]
    # Go to the corresponding level
    goLevObjSel $node
	
    # Convert 'tree format' node name into 'listbox format' node name
    set object [split $node :]
    set item [lindex $object end]
    if { [llength $object] > 2 } {
	set item [expr $item + 1]
    }
	
    # Put the item in the selection then update the views
    selOb
    selOb $item
    rV

    focus $f.bok
    tkwait window $w
    update
    return;
}
#reconsider

#cleanObjSel:
# This function is called when picked objects have to be added to the selection
# (i.e. when the user pressed Shift during the picking). Its purpose is
# to remove the following objects from the list of picked objects:
# - objects that are already selected
# - objects that are from a different level than the selected ones
proc cleanObjSel { Selection } {
    global ay
    set cleanedSelect ""
    
    if { $ay(lb) == 1 } {
	# For each item picked by the user :
	foreach i $ay(LastSelection) {
	    set hierarchy [split $i :]
	# Is the item in the current level ?
	    if { [join [lrange $hierarchy 0 end-1] :] == $ay(CurrentLevel)} {
		set item [lindex $hierarchy end]
		# Because of the '..' in the list we have to increment
		# the item entry, except for the first level where 'root'
		# is actually entry 0.
		if { $ay(CurrentLevel) != "root" } {
		    set item [expr $item + 1]
		}
		# Is the item already stored in the current selection ?
		if { [lsearch $Selection $item] == -1} {
		    lappend cleanedSelect $i
		}
	    }
	} 
    } else {
    
	# For each item picked by the user :
	foreach i $ay(LastSelection) {
	# Is the item already stored in the current selection ?
	    if { [lsearch $Selection $i] == -1} {
	# Is the item in the current level ?
		if { $ay(SelectedLevel) == [$ay(tree) parent $i] } {
		    lappend cleanedSelect $i
		}
	    }
	}
    }
    
    set ay(LastSelection) $cleanedSelect
}
#cleanObjSel

#goLevObjSel:
# Goes to the level which name (in tree format) is given by 'level'
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
    set end [ expr [llength $hierarchy] -2 ]
    foreach i [lrange $hierarchy 0 $end] {
        goDown $i
    }
    
    return [lindex $hierarchy end]
}


#listBoxObjSel:
# Updates the List Box (if activated) accordingly to the selected items
proc listBoxObjSel { Selection } {
    global ay curlevel curtypes

    set lb $ay(olb)
    # Goes to the level of 'Selection' then stores the item' id
    set item [goLevObjSel $Selection]

    # Updates the list box to reflect the level change
    $lb delete 0 end
    getLevel curlevel curtypes
    eval [ subst "$lb insert end $curlevel"]

    return $item
}

#singleObjSel:
# Replace the current selection by one picked object.
proc singleObjSel { node } {
    global ay rArray
    
    if { $node != "" } {
	set ay(LastSelection) [split $node " "]
	set Selection [lindex $ay(LastSelection) 0]

	# If the user has picked several objects then reconsider...
	if {[llength $ay(LastSelection)] > 1} {
	    reconsider $ay(LastSelection)
	    if { $rArray(result) != "" } {
		set Selection $rArray(result)
	    } else {
		return
	    }
	}
	
	# Determine whether the listbox is enabled or not
	if {$ay(lb) == 1} {
	    set lb $ay(olb)
	    $lb selection clear 0 end
	    set item [listBoxObjSel $Selection]; # update listbox
	    $lb selection set $item
	    $lb see $item
	    olb_select
	} else {
	    tree_openTree $ay(tree) $Selection
	    tree_selectItem 1 $ay(tree) $Selection
	    $ay(tree) see $Selection
	}
    }
}
#singleObjSel

#multipleObjSel:
# Replace the current selection by the multiple objects picked
proc multipleObjSel { node } {
    global ay

    if { $node != "" } {
	set ay(LastSelection) [split $node " "]
	# Remove items that do not belong to the current level
	cleanObjSel ""
	
	# If the user has picked items that are already selected or that
	# do not belong to the current level then $ay(Selection) is empty
	# (it has been emptied by cleanObjSel) else we can process the user's
	# picking
	if { $ay(LastSelection) != "" } {
	    # Determine whether the listbox is enabled or not
	    if { $ay(lb) == 1 } {
	        set lb $ay(olb)
		# Empty the current selection
		selOb
		
		# Unselect all items of the listbox and replace them by
		# the new selection
		$lb selection clear 0 end
		foreach i $ay(LastSelection) {
		    set item [lindex [split $i :] end]
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
		# Unselect all items of the listbox and replace them by
		# the new selection
		tree_selectItem 1 $ay(tree) [lindex $ay(LastSelection) 0]
		foreach i [lrange $ay(LastSelection) 1 end] {
		    tree_toggleSelection $ay(tree) $i
		}
		# Scroll the tree so that selected items are visible
		$ay(tree) see [lindex $ay(LastSelection) end]
	    }
	}
    }
}
#addMultipleObjSel

#addObjSel:
# Add one picked object to the current selection
proc addObjSel { node } {
    global ay rArray

    if { $node != "" } {
	set ay(LastSelection) [split $node " "]

	# Get the current selection
	if { $ay(lb) == 1} {
	    set Selected [$ay(olb) curselection]
	} else {
	    set Selected [$ay(tree) selection get]
	}

	# Remove already selected items and items that do not belong to the
	# current level
	cleanObjSel $Selected
	
	set Selection [lindex $ay(LastSelection) 0]
	
	# If the user has picked items that are already selected or that
	# do not belong to the current level then $ay(Selection) is empty
	# (it has been emptied by cleanObjSel) else we can process the user's
	# picking
	if { $ay(LastSelection) != "" } {

	    #If there was no previously selected items then we fall back
	    # in a regular "single object selection"
	    if { $Selected == "" } {
		singleObjSel $node
		return
	    }

	    # If the user has picked several objects then reconsider...
	    if {[llength $ay(LastSelection)] > 1} {
		set oldSelection ""
		# Save the current selection
		getSel oldSelection
		
		# Reconsider the ambiguous picking
		reconsider $ay(LastSelection)
		if { $rArray(result) != "" } {
		    set Selection $rArray(result)
		} else {
		    return
		}
		
		# Restore the previous selection
		selOb
		selOb $oldSelection
	    }
	
	    # Determine whether the listbox is enabled or not
	    if { $ay(lb) == 1 } {
		set item [lindex [split $Selection :] end]
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
		tree_toggleSelection $ay(tree) $Selection
		# Scroll the tree so that the selected item is visible
		$ay(tree) see $Selection
	    }
	} else {
	    ayError 1 "addObjSel" "Can not select from different levels!"
	}
    }
}
#addObjSel

#addMultipleObjSel:
# Add several picked objects to the selection
proc addMultipleObjSel { node } {
    global ay

    if { $node != "" } {
	set ay(LastSelection) [split $node " "]

	# Get the current selection
	if { $ay(lb) == 1} {
	    set Selected [$ay(olb) curselection]
	} else {
	    set Selected [$ay(tree) selection get]
	}
	
	# Remove already selected items and items that do not belong to the
	# current level
	cleanObjSel $Selected
	
	# If the user has picked items that are already selected or that
	# do not belong to the current level then $ay(Selection) is empty
	# (it has been emptied by cleanObjSel) else we can process the user's
	# picking
	if { $ay(LastSelection) != "" } {

	    #If there was no previously selected items then we fall back
	    # in a regular "multiple object selection"
	    if { $Selected == "" } {
		multipleObjSel $node
		return
	    }

	    # Determine whether the listbox is enabled or not
	    if { $ay(lb) == 1 } {
		foreach i $ay(LastSelection) {
		    set item [lindex [split $i :] end]
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
		foreach i $ay(LastSelection) {
		    tree_toggleSelection $ay(tree) $i
		}
		# Scroll the tree so that selected items are visible
		$ay(tree) see [lindex $ay(LastSelection) end]
	    }
	} else {
	    ayError 1 "addMultipleObjSel" "Can not select from different levels!"
	}
    }
}
#addMultipleObjSel
