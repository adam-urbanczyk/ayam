# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2001 by Randolf Schultz
# (rschultz@informatik.uni-rostock.de) and others.
#
# All rights reserved.
#
# See the file License for details.

# control.tcl - new control structures

# forAll_tree:
#
#
proc forAll_tree { recursive command } {
    global ay i
    
    set tree $ay(tree)

    set selection [$tree selection get]

    if { $selection == "" } {
	set sel [$tree nodes $ay(CurrentLevel)]
	$tree selection set $sel
	treeSelect $sel
	set selection $sel
    }

    $tree selection clear
    treeSelect ""
    set i 0
    foreach sel $selection {

	incr i

	$tree selection clear
	$tree selection set $sel
	treeSelect $sel
	plb_update

	# go down?
	if { $recursive } {
	    if { [hasChild] == 1} {
		set index [$tree index $sel]
		goDown $index
		$tree selection clear
		treeSelect ""
		set oldclevel $ay(CurrentLevel)
		set ay(CurrentLevel) $sel
		set oldi $i
		update
		catch {forAll_tree 1 $command} retCode
		if { $retCode == -1 } {
		    return -1;
		}
		set i $oldi
		goUp
		set ay(CurrentLevel) $oldclevel
		$tree selection clear
		$tree selection set $sel
		treeSelect $sel
		plb_update
	    }
	}

	catch {eval $command} retCode
	if { $retCode == -1 } {
	    return -1;
	}
    }

 return;
}
# forAll_tree


# forAll_lb
#
#
proc forAll_lb { recursive command } {
    global ay i

    set lb $ay(olb)

    set selection [$lb curselection]

    if { $selection == "" } {
	$lb selection set 1 end
	set selection [$lb curselection]
    }
    $lb selection clear 0 end
    selOb
    set i 0
    foreach sel $selection {

	incr i
	$lb selection clear 0 end
	$lb selection set $sel
	selOb $sel
	plb_update
	
	# go down?
	if { $recursive } {
	    if { [hasChild] == 1} {
		set templevel $ay(CurrentLevel)
		append ay(CurrentLevel) ":$sel"
		goDown $sel
		uS
		$lb selection clear 0 end
		selOb
		set oldi $i
		update
		catch {forAll_lb 1 $command} retCode
		if { $retCode == -1 } {
		    return -1;
		}
		set i $oldi
		goUp
		uS
		$lb selection set $sel
		selOb $sel
		plb_update
		set ay(CurrentLevel) $templevel
	    }
	}

	$lb selection clear 0 end
	$lb selection set $sel
	selOb $sel
	plb_update
	catch {eval $command} retCode
	if { $retCode == -1 } {
	    return -1;
	}
    }

    uS
    plb_update

 return;
}
# forAll_lb


# forAll:
#
#
proc forAll { recursive command } {
    global ay

    if { $ay(lb) == 0 } {
	set tree $ay(tree)
	# save current state of tree widget
	set clevel $ay(CurrentLevel)
	set slevel $ay(SelectedLevel)
	set sel [$tree selection get]

	# now call commands
	catch {forAll_tree $recursive $command}

	# recover state of tree widget
	$tree selection set $sel
	treeSelect $sel
	plb_update
	set ay(CurrentLevel) $clevel
	set ay(SelectedLevel) $slevel
	set ay(DropActive) 0
	rV
    } else {
	forAll_lb $recursive $command
    }

 return;
}
# forAll


# forAllT_tree:
#
#
proc forAllT_tree { type recursive command } {
    global ay i
    
    set tree $ay(tree)

    set selection [$tree selection get]

    if { $selection == "" } {
	set sel [$tree nodes $ay(CurrentLevel)]
	$tree selection set $sel
	treeSelect $sel
	set selection $sel
    }

    $tree selection clear
    treeSelect ""
    set i 0
    foreach sel $selection {

	incr i

	$tree selection clear
	$tree selection set $sel
	treeSelect $sel
	plb_update

	# go down?
	if { $recursive } {
	    if { [hasChild] == 1} {
		set index [$tree index $sel]
		goDown $index
		$tree selection clear
		treeSelect ""
		set oldclevel $ay(CurrentLevel)
		set ay(CurrentLevel) $sel
		set oldi $i
		update
		catch { forAllT_tree $type $recursive $command } retCode
		if { $retCode == -1 } {
		    return -1;
		}
		set i $oldi
		goUp
		set ay(CurrentLevel) $oldclevel
		$tree selection clear
		$tree selection set $sel
		treeSelect $sel
		plb_update
	    }
	}
	global otype
        set otype ""
        getType otype
        if { [string tolower $type] == [string tolower $otype] } {
	    catch { eval $command } retCode
	    if { $retCode == -1 } {
		    return -1;
	    }
        }

    }

 return;
}
# forAllT_tree


# forAllT_lb:
#
#
proc forAllT_lb { type recursive command } {
    global ay i

    set lb $ay(olb)

    set selection [$lb curselection]

    if { $selection == "" } {
	$lb selection set 1 end
	set selection [$lb curselection]
    }
    $lb selection clear 0 end
    selOb
    set i 0
    foreach sel $selection {

	incr i
	$lb selection clear 0 end
	$lb selection set $sel
	selOb $sel
	plb_update
	
	# go down?
	if { $recursive } {
	    if { [hasChild] == 1} {
		set templevel $ay(CurrentLevel)
		append ay(CurrentLevel) ":$sel"

		goDown $sel
		uS
		$lb selection clear 0 end
		selOb
		set oldi $i
		update
		catch {forAllT_lb $type $recursive $command} retCode
		if { $retCode == -1 } {
		    return -1;
		}
		set i $oldi
		goUp
		uS
		$lb selection set $sel
		selOb $sel
		plb_update
		set ay(CurrentLevel) $templevel
	    }
	}

	$lb selection clear 0 end
	$lb selection set $sel
	selOb $sel
	plb_update

	global otype
        set otype ""
        getType otype
        if { [string tolower $type] == [string tolower $otype] } {
            catch {eval $command} retCode
	    if { $retCode == -1 } {
		return -1;
	    }
        }
    }

    uS
    plb_update

 return;
}
# forAllT_lb


# forAllT:
#
#
proc forAllT { type recursive command } {
    global ay

    if { $ay(lb) == 0 } {
	set tree $ay(tree)
	# save current state of tree widget
	set clevel $ay(CurrentLevel)
	set slevel $ay(SelectedLevel)
	set sel [$tree selection get]

	# now call commands
	catch {forAllT_tree $type $recursive $command}

	# recover state of tree widget
	$tree selection set $sel
	treeSelect $sel
	plb_update
	set ay(CurrentLevel) $clevel
	set ay(SelectedLevel) $slevel
	set ay(DropActive) 0
	rV
    } else {
	forAllT_lb $type $recursive $command
    }

 return;
}
# forAllT

# allow cancelling of user Tcl-scripts in the console

set cancelled 0

bind .fl.con.console <Control-KeyPress-C> {set cancelled 1}
bind .fl.con.console <Control-KeyRelease-C> {set cancelled 0}

#
#
#
proc cancelled {} {
    global cancelled
    # give the user a chance to press Ctrl+C
    update
    if { $cancelled == 1 } {
	error "Cancelled!"
    }
}
# cancelled

rename while _while
proc while {condition body} {
    uplevel [list _while $condition "cancelled;$body"]
}

rename for _for
proc for {start test next body} {
    uplevel [list _for $start $test $next "cancelled;$body"]
}

#bind .fl.con.console <FocusIn> "renameWhileFor"
#bind .fl.con.console <FocusOut> "unrenameWhileFor"

#
#
#
proc renameWhileFor { } {
    uplevel \#0 {
	rename while _while
	proc while {condition body} {
	    uplevel [list _while $condition "cancelled;$body"]
	}

	rename for _for
	proc for {start test next body} {
	    uplevel [list _for $start $test $next "cancelled;$body"]
	}

	#rename foreach _foreach
	#proc foreach {vname args} {
	#    puts "vname: $vname"
	#    set elems [lrange $args 0 end-1]
	#    puts "elems: $elems"
	#    set body [lindex $args end]
	#    puts "body: $body"
	#    uplevel [list _foreach $vname $elems "cancelled;$body"]
	#}
    }
    #uplevel
 return;
}
# renameWhileFor


#
#
#
proc unrenameWhileFor { } {
    uplevel \#0 {
	#rename foreach {}
	rename for {}
	rename while {}
	#rename _foreach foreach 
	rename _for for
	rename _while while
    }
    #uplevel
 return;
}
# unrenameWhileFor

#selAdd:
# extend selection in tree or listbox
proc selAdd { ud } {
    global ay
    if { $ay(lb) == 0 } {
	set tree $ay(tree)
	set cl $ay(CurrentLevel)
	set sel [$tree selection get]
	if { [llength $sel] == 0 } { return; }
	if { $ud == 0 } {
	    # Up -> extend to previous
	    set node [lindex $sel 0]
	    set i [string last ":" $node]
	    set j [string range $node [expr $i+1] end]
	    if { $j > 0 } {
		incr j -1
		set newnode [string range $node 0 $i]
		append newnode $j
		set sel [linsert $sel 0 $newnode]
		set newsel 1
	    } else {
		set newsel 0
	    }
	} else {
	    # Down -> extend to next
	    set node [lindex $sel end]
	    set i [string last ":" $node]
	    set j [string range $node [expr $i+1] end]
	    set lastn [lindex [$tree nodes $cl] end]
	    set i [string last ":" $lastn]
	    set last [string range $lastn [expr $i+1] end]

	    if { $j < $last } {
		incr j
		set newnode [string range $node 0 $i]
		append newnode $j
		set sel [lappend sel $newnode]
		set newsel 1
	    } else {
		set newsel 0
	    }
	}
	if { $newsel == 1 } {
	    eval [subst "$tree selection set $sel"]
	    $tree see $sel
	    eval [subst "treeSelect $sel"]
	    plb_update
	    if { $ay(need_redraw) == 1 } {
		rV
	    }
	}
    } else {
	#if { [focus -displayof .] == $ay(olb) } { return; }
	set lb $ay(olb)
	set sel ""
	set sel [$lb curselection]
	if { [llength $sel] == 0 } { return; }
	if { $ud == 0 } {
	    # Up -> extend to previous
	    set first [lindex $sel 0]
	    if { $first > 0 } {
		set sel [linsert $sel 0 [expr $first-1]]
		$lb selection set  [lindex $sel 0] [lindex $sel end]
		$lb see [lindex $sel 0]
		eval [subst "selOb $sel"]
		plb_update
		if { $ay(need_redraw) == 1 } {
		    rV
		}
		# if
	    }
	} else {
	    # Down -> extend to next
	    set last [lindex $sel end]
	    if { $last < [$lb index end] } {
		lappend sel [expr $last+1]
		$lb selection set  [lindex $sel 0] [lindex $sel end]
		$lb see [lindex $sel end]
		eval [subst "selOb $sel"]
		plb_update
		if { $ay(need_redraw) == 1 } {
		    rV
		}
	    }
	}
    }
}
# selAdd

# select Next or Previous or First or Last object in tree or listbox
proc selNPFL { npfl } {
    global ay
    if { $ay(lb) == 0 } {
	set tree $ay(tree)
	set cl $ay(CurrentLevel)

	set sel ""
	set sel [$tree selection get]
	set lastn [lindex [$tree nodes $cl] end]
	set i [string last ":" $lastn]
	set last [string range $lastn [expr $i+1] end]

	if { $sel == "" } {
	    if { $npfl != 3 } {
		# select first
		set sel ${ay(CurrentLevel)}:0
	    } else {
		# select last
		set sel [lindex [$tree nodes $cl] end]
	    }
	} else {
	    set sel [lindex $sel 0]
	    set i [string last ":" $sel]
	    set cur [string range $sel [expr $i+1] end]
	    set sel [string range $sel 0 $i]
	    if { $npfl == 0 } {
		# select next
		set nxt [expr $cur+1]
		if { $nxt <= $last } {
		    append sel $nxt
		} else {
		    set sel ""
		}
	    }
	    if { $npfl == 1 } {
		# select previous
		set prev [expr $cur-1]
		if { $prev >= 0 } {
		    append sel $prev
		} else {
		    set sel ""
		}
	    }
	    if { $npfl == 2 } {
		# select first
		append sel 0
	    }
	    if { $npfl == 3 } {
		# select last
		append sel $last
	    }
	}

	if { $sel != "" } {
	    $tree selection set $sel
	    $tree see $sel
	    treeSelect $sel
	    plb_update
	    if { $ay(need_redraw) == 1 } {
		rV
	    }
	}

    } else {
	#if { [focus -displayof .] == $ay(olb) } { return; }
	set lb $ay(olb)
	set sel ""
	set sel [$lb curselection]
	set last [$lb index end]
	incr last -1

	if { $sel == "" } {
	    if { $npfl == 2 } {
		# select first
		set sel 0
	    } else {
		if { $npfl == 3 } {
		    # select last
		    set sel $last
		} else {
		    # select first
		    set sel 0
		}
	    }
	} else {

	    set sel [lindex $sel 0]

	    if { $npfl == 0 } {
		# select next
		set nxt [expr $sel+1]
		if { $nxt <= $last } {
		    set sel $nxt
		} else {
		    set sel ""
		}
	    }
	    if { $npfl == 1 } {
		# select previous
		set prev [expr $sel-1]
		if { $prev >= 0 } {
		    set sel $prev
		} else {
		    set sel ""
		}
	    }
	    if { $npfl == 2 } {
		# select first
		set sel 0
	    }
	    if { $npfl == 3 } {
		# select last
		set sel $last
	    }

	}
	# if

	if { $sel != "" } {
	    $lb selection clear 0 end
	    $lb selection set $sel
	    $lb see [lindex $sel 0]
	    eval [subst "selOb $sel"]
	    plb_update
	    if { $ay(need_redraw) == 1 } {
		rV
	    }
	    # if
	}
	# if
    }
    # if

 return;
}
# selNPFL


# 
proc resetFocus { } {
    global ay
    if { $ay(lb) == 0 } {
	focus $ay(tree)
    } else {
	focus $ay(olb)
    }
 return;
}
# resetFocus


#
proc mouseWatch { onoff ws } {
    if { $onoff } {
	foreach w $ws {
	    if { [winfo exists $w] } {
		$w configure -cursor "watch"
	    }
	}
	# foreach
    } else {
	foreach w $ws {
	    if { [winfo exists $w] } {
		$w configure -cursor {}
	    }
	}
	# foreach
    }
    # if
 return;
}
# mouseWatch
