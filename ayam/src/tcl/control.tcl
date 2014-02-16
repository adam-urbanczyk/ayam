# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2005 by Randolf Schultz
# (randolf.schultz@gmail.com) and others.
#
# All rights reserved.
#
# See the file License for details.

# control.tcl - new control structures

# forAll_callcmd:
#  helper for forAll
#
proc forAll_callcmd { cmd type } {

    if { $type != "" } {
	set otype ""
	getType otype
	if { [string co -nocase $type $otype] } {
	    return 0;
	}
    }
    catch {eval $cmd} retCode
    if { $retCode == -1 } {
	return -1;
    }

 return 0;
}
# forAll_callcmd


# forAll_tree:
#  helper for forAll
#
proc forAll_tree { recursive type command } {
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

	if { $recursive == 2 } {
	    if { [forAll_callcmd $command $type] == -1 } {
		return -1;
	    }
	}

	# go down?
	if { $recursive } {
	    if { [hasChild] == 1 } {
		set index [$tree index $sel]
		goDown $index
		$tree selection clear
		treeSelect ""
		set oldclevel $ay(CurrentLevel)
		set ay(CurrentLevel) $sel
		set oldi $i
		update
		catch {forAll_tree $recursive $type $command} retCode
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

	if { $recursive != 2 } {
	    if { [forAll_callcmd $command $type] == -1 } {
		return -1;
	    }
	}
    }
    # foreach

 return 0;
}
# forAll_tree


# forAll_lb
#  helper for forAll
#
proc forAll_lb { recursive type command } {
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
	selOb -lb $sel
	plb_update

	if { $recursive == 2 } {
	    if { [forAll_callcmd $command $type] == -1 } {
		return -1;
	    }
	}

	# go down?
	if { $recursive } {
	    if { [hasChild] == 1 } {
		set templevel $ay(CurrentLevel)
		append ay(CurrentLevel) ":$sel"
		goDown $sel
		uS
		$lb selection clear 0 end
		selOb
		set oldi $i
		update
		catch {forAll_lb $recursive $type $command} retCode
		if { $retCode == -1 } {
		    return -1;
		}
		set i $oldi
		goUp
		uS
		$lb selection clear 0 end
		$lb selection set $sel
		selOb -lb $sel
		plb_update
		set ay(CurrentLevel) $templevel
	    }
	}

	if { $recursive != 2 } {
	    if { [forAll_callcmd $command $type] == -1 } {
		return -1;
	    }
	}
    }
    # foreach

    uS
    plb_update

 return 0;
}
# forAll_lb


# forAll:
#  (recursively) call command for all (selected) objects
#
proc forAll { args } {
    global ay

    if { [llength $args] < 1 } { return; }

    set command [lindex $args end]
    set recursive 1
    set type ""
    foreach {key val} $args {
	switch -- $key {
	    -r -
	    -recursive {
		set recursive $val
	    }
	    -t -
	    -type {
		set type $val
	    }
	}
    }

    if { $ay(lb) == 0 } {
	set tree $ay(tree)

	# save current state of tree widget
	set clevel $ay(CurrentLevel)
	set slevel $ay(SelectedLevel)
	set sel [$tree selection get]

	# now call commands
	catch {forAll_tree $recursive $type $command}

	# recover state of tree widget
	eval [subst "$tree selection set $sel"]
	eval [subst "treeSelect $sel"]
	plb_update
	set ay(CurrentLevel) $clevel
	set ay(SelectedLevel) $slevel
	set ay(DropActive) 0

	# redraw all views
	rV
    } else {
	forAll_lb $recursive $type $command
    }

 return;
}
# forAll


# allow cancelling of user Tcl-scripts in the console

set cancelled 0

bind .fl.con.console <Control-KeyPress-C> {set cancelled 1;break}
bind .fl.con.console <Control-KeyRelease-C> {set cancelled 0;break}

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
    if { $ay(sellock) == 1 } {
	bell; return;
    } else {
	set ay(sellock) 1
    }
    while { 1 } {
    if { $ay(lb) == 0 } {
	# Tree
	set tree $ay(tree)
	set cl $ay(CurrentLevel)
	set sel [$tree selection get]
	if { [llength $sel] == 0 } { break; }
	set newsel 0
	if { $ud == 0 } {
	    # -> extend to previous
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
	}
	# if
	if { $ud == 1 } {
	    # -> extend to next
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
	# if
	if { $ud == 3 } {
	    # -> extend to upper end
	    set node [lindex $sel 0]
	    set i [string last ":" $node]
	    set j [string range $node [expr $i+1] end]
	    if { $ay(CurrentLevel) == "root" } {
		set k 1
	    } else {
		set k 0
	    }
	    while { $j > $k } {
		set newnode [string range $node 0 $i]
		append newnode $k
		set sel [linsert $sel 0 $newnode]
		set newsel 1
		incr k
	    }
	    # while
	}
	# if
	if { $ud == 4 } {
	    # -> extend to lower end
	    set node [lindex $sel end]
	    set i [string last ":" $node]
	    set j [string range $node [expr $i+1] end]
	    set lastn [lindex [$tree nodes $cl] end]
	    set i [string last ":" $lastn]
	    set last [string range $lastn [expr $i+1] end]
	    set k [expr {$j + 1}]
	    while { $k <= $last } {
		set newnode [string range $node 0 $i]
		append newnode $k
		set sel [linsert $sel 0 $newnode]
		set newsel 1
		incr k
	    }
	    # while
	}
	# if

	if { $newsel == 1 } {
	    eval [subst "$tree selection set $sel"]
	    $tree see $sel
	    eval [subst "treeSelect $sel"]

	    # allow dnd of multiple selected objects
	    $tree bindText <ButtonPress-1> ""
	    $tree bindText <ButtonRelease-1> "tree_selectItem 1 $tree"

	    plb_update
	    if { $ay(need_redraw) == 1 } {
		rV
	    }
	}
	# if
    } else {
	# Listbox
	set lb $ay(olb)
	set sel [$lb curselection]
	if { [llength $sel] == 0 } { break; }
	if { $ud == 0 } {
	    # Up -> extend to previous
	    set first [lindex $sel 0]
	    if { $first > 0 } {
		set sel [linsert $sel 0 [expr $first-1]]
		$lb selection set [lindex $sel 0] [lindex $sel end]
		$lb see [lindex $sel 0]
		eval [subst "selOb -lb $sel"]
		plb_update
		if { $ay(need_redraw) == 1 } {
		    rV
		}
	    }
	    # if
	}
	# if
	if { $ud == 1 } {
	    # Down -> extend to next
	    set last [lindex $sel end]
	    if { $last < [$lb index end] } {
		lappend sel [expr $last+1]
		$lb selection set [lindex $sel 0] [lindex $sel end]
		$lb see [lindex $sel end]
		eval [subst "selOb -lb $sel"]
		plb_update
		if { $ay(need_redraw) == 1 } {
		    rV
		}
	    }
	    # if
	}
	# if
	if { $ud == 3 } {
	    # -> extend to first
	    set first [lindex $sel 0]
	    if { $first > 0 } {
		set k 1
		while { $k < $first } {
		    set sel [linsert $sel 0 $k]
		    incr k
		}
		$lb selection set 1 [lindex $sel end]
		$lb see 1
		eval [subst "selOb -lb $sel"]
		plb_update
		if { $ay(need_redraw) == 1 } {
		    rV
		}
	    }
	    # if
	}
	# if
	if { $ud == 4 } {
	    # -> extend to last
	    set last [lindex $sel end]
	    if { $last < [$lb index end] } {
		set k $last
		while { $k <= [$lb index end] } {
		    lappend sel $k
		    incr k
		}
		$lb selection set [lindex $sel 0] [lindex $sel end]
		$lb see [lindex $sel end]
		eval [subst "selOb -lb $sel"]
		plb_update
		if { $ay(need_redraw) == 1 } {
		    rV
		}
	    }
	}
	# if
    }
    # if tree or lb
    break;
  }
  # while 1
 set ay(sellock) 0
 return;
}
# selAdd


# select Next or Previous or First or Last object in tree or listbox
proc selNPFL { npfl } {
    global ay
    if { $ay(sellock) == 1 } {
	bell; return;
    } else {
	set ay(sellock) 1
    }
    if { $ay(lb) == 0 } {
	# Tree is active
	set tree $ay(tree)
	set cl $ay(CurrentLevel)
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
	# ListBox is active
	set lb $ay(olb)
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
	    eval [subst "selOb -lb $sel"]
	    plb_update
	    if { $ay(need_redraw) == 1 } {
		rV
	    }
	    # if
	}
	# if
    }
    # if

 set ay(sellock) 0

 return;
}
# selNPFL


#
proc resetFocus { } {
    global ay
    if { $ay(lb) == 0 } {
	catch {focus $ay(tree)}
    } else {
	catch {focus $ay(olb)}
    }
 return;
}
# resetFocus


#
proc restoreFocus { w } {
    if { $w == "" } {
	resetFocus;
    } else {
	if { [winfo exists $w] && [winfo ismapped $w] } {
	    catch {focus $w}
	} else {
	    resetFocus
	}
    }
 return;
}
# restoreFocus


#
proc mouseWatch { onoff ws } {
    if { $onoff } {
	foreach w $ws {
	    if { [winfo exists $w] } {
		$w configure -cursor "watch"
	    }
	}
    } else {
	foreach w $ws {
	    if { [winfo exists $w] } {
		$w configure -cursor {}
	    }
	}
    }
 return;
}
# mouseWatch


# addToProc:
#  add code in <addition> to the code of procedure <procedure>
#  replacing the last "return;"-statement in that procedure
#  (which has to be present in order to make this work)
proc addToProc { procedure addition } {
    # get old proc code
    set oldprocbody [info body $procedure]
    # find last "return;"-statement
    set index [string last "return;\n" $oldprocbody]
    # remove last "return;"-statement
    set newprocbody [string range $oldprocbody 0 [expr $index - 1]]
    # add new code
    append newprocbody "\n"
    append newprocbody $addition
    # add new trailing "return;"-statement
    append newprocbody "\nreturn;\n"
    # overwrite procedure
    if { [info args $procedure] != "" } {
	foreach arg [info args $procedure] {
	    if {[info default $procedure $arg def]} {
		lappend newargs [list $arg $def]
	    } else {
		lappend newargs $arg
	    }
	}
    } else {
	set newargs { }
    }
    proc $procedure $newargs $newprocbody
 return;
}
# addToProc


# pushSel:
#  push the current object selection (in tree or listbox) onto the
#  selection stack (for later re-establishing it using popSel below)
proc pushSel { } {
    global ay
    set ay(cursel) ""
    getSel ay(cursel)
    lappend ay(selstack) {$ay(cursel)}
    return;
}
# pushSel


# popSel:
#  pop the topmost selection from the selection stack and re-establish it
#  (without changing anything to the UI!)
proc popSel { } {
    global ay
    set ay(cursel) [lindex $ay(selstack) end]
    set ay(selstack) [lreplace $ay(selstack) end end]
    eval [subst "selOb $ay(cursel)"]
    return;
}
# popSel


# selMUD:
#  move the selected object(s) up or down in the current level
#
proc selMUD { up } {
    global ay
    if { $ay(selmudlock) == 1 } {
	bell; return;
    } else {
	set ay(selmudlock) 1
    }
    while { 1 } {
    if { $ay(lb) == 0 } {
	# tree
	set w $ay(tree)
	set sel ""
	set sel [$w selection get]
	if { $sel ne "" } {
	    set s [lindex $sel 0]
	    set i [string range $s [expr [string last ":" $s]+1] end]
	    if { $ay(CurrentLevel) == "root" && $i == 0} { break; }
	    if { $up == 1 } {
		upOb
		if { $ay(CurrentLevel) == "root" } {
		    set first 1
		    if { $i == 0 || $i == 1 } { break; }
		} else {
		    set first 0
		    if { $i == 0 } { break; }
		}
	    } else {
		downOb
		getLevel -l ll
		set s [lindex $sel end]
		set i [string range $s [expr [string last ":" $s]+1] end]
		if { $ay(CurrentLevel) == "root" } {
		    incr ll -1
		    if { $i == $ll } { break; }
		} else {
		    if { $i == [expr $ll - 1] } { break; }
		}
	    }
	    $w selection clear
	    uS 0
	    foreach s $sel {
		set i [string range $s [expr [string last ":" $s]+1] end]
		if { $up } {
		    if { $i > $first } {
			incr i -1
			set s [string range $s 0 [string last ":" $s]]
			append s $i
		    }
		} else {
		    if { $i < $ll } {
			incr i
			set s [string range $s 0 [string last ":" $s]]
			append s $i
		    }
		}
		$w selection add $s
	    }
	}
    } else {
	# listbox
	set w  $ay(olb)
	set sel ""
	set sel [$w curselection]
	if { $sel ne "" } {
	    set i [lindex $sel 0]
	    if { $i == 0} { break; }
	    if { $up == 1 } {
		if { $i == 1 } { break; }
		upOb
	    } else {
		set ll [expr [$w index end]-1]
		set i [lindex $sel end]
		if { $i == $ll } { break; }
		downOb
	    }
	    $w selection clear 0 end
	    uS 0
	    foreach i $sel {
		if { $up } {
		    incr i -1
		} else {
		    incr i
		}
		$w selection set $i
	    }
	}
    }
    break;
    }
    set ay(selmudlock) 0
 return;
}
# selMUD

# lreverse:
# returns the given list in reverse order
proc lreverse { in } {
    set out ""
    foreach elem $in {
	set out [linsert $out 0 $elem]
    }
 return $out;
}
# lreverse

# searchOb:
# find objects according to the given expression and execute the
# given action on them
proc searchOb { expression action {gui 0} } {
    global ay ObjectSearch

    if { $ObjectSearch(ClearHighlight) } {
	tree_paintTree root
	set ObjectSearch(nodes) ""
    }
    if { $ObjectSearch(ClearClipboard) } {
	clearClip
    }

    # compile expression
    if { ([string first "\$" $ObjectSearch(Expression)] == 0) ||
	 ([string first "(" $ObjectSearch(Expression)] == 0) } {
	# variable comparison
	set rem [string range $ObjectSearch(Expression)\
		     [string first "\$" $ObjectSearch(Expression)] end]
	set cx ""
	set vi 0
	set mappings {}
	while { [string first "\$" $rem] != -1 } {
	    if { [string first "\$name" $rem] == 0 } {
		# object name comparison
		append cx "getName name;"
	    } elseif { [string first "\$type" $rem] == 0 } {
		# object type comparison
		append cx "getType type;"
	    } elseif { [string first "\$mat" $rem] == 0 } {
		# object material comparison
		append cx "global matPropData;"
		append cx "set matPropData(Materialname) \"\";getMat;"
		append cx "set mat \$matPropData(Materialname);"
	    } else {
		# arbitrary variable, probably a property value comparison like
		# SphereAttr(Radius) == 1.0
		# => create an expression like
		# getProperty SphereAttr(Radius) val; expr $val == 1.0
		set index [string first ")" $rem]
		if { $index != -1 } {
		    set prop [string range $rem 1 $index]
		    # arrange to transform the property specifier
		    if { [lsearch -exact $mappings $prop] == -1 } {
			append cx "getProperty "
			append cx $prop
			append cx " val$vi;"
			lappend mappings $prop
			lappend mappings "val$vi"
			incr vi
		    }
		}
	    }

	    # arrange to process next $
	    set rem [string range $rem 1 end]
	    set index [string first "\$" $rem]
	    if { $index != -1 } {
		set rem [string range $rem $index end]
	    } else {
		break;
	    }
	}
	# while
	append cx "expr \{"
	if { [llength $mappings] > 0 } {
	    append cx [string map $mappings $ObjectSearch(Expression)]
	} else {
	    append cx $ObjectSearch(Expression)
	}
	append cx "\}"

	set ObjectSearch(cx) $cx
    } else {
	# expression is not a variable comparison
	if { [string first "Master" $ObjectSearch(Expression)] == 0 } {
	    set cx "set j \$i;incr j -1;"
	    append cx "expr \{ \"$ObjectSearch(master)\" == "
	    append cx "\"\$::ay(CurrentLevel):\$j\" \}"
	    set ObjectSearch(cx) $cx
	} elseif { [string first "Instances" $ObjectSearch(Expression)]
		   == 0 } {
	    set cx "getType type; if \{ \$type == \"Instance\" \} \{"
	    append cx "getMaster m; expr \{ \"\$m\" == "
	    append cx "\"$ObjectSearch(master)\" \} \}"
	    set ObjectSearch(cx) $cx
	} else {
	    # expression is a procedure call
	    set ObjectSearch(cx) $ObjectSearch(Expression)
	}
    }

    # save old selection state
    set ObjectSearch(oldclevel) $ay(CurrentLevel)
    set ObjectSearch(oldslevel) $ay(SelectedLevel)
    set ObjectSearch(oldselection) [$ay(tree) selection get]
    # go to top level?
    if { $ObjectSearch(Scope) == 0 } {
	# scope == "All"
	set ay(CurrentLevel) "root"
	set ay(SelectedLevel) "root"
	goTop
    }
    # clear selection (i.e. work on all objects regardless
    # of their current selection state)?
    if { $ObjectSearch(Scope) != 1 } {
	# scope != "Selection"
	$ay(tree) selection clear
	selOb
    }

    # now go find the objects
    set ObjectSearch(numfound) 0
    forAll 1 {
	global i ay ObjectSearch
	if { [eval $ObjectSearch(cx)] } {
	    # found an object
	    incr ObjectSearch(numfound)
	    # execute action
	    switch $ObjectSearch(Action) {
		"Highlight" {
		    tree_openTree $ay(tree) $ay(CurrentLevel)
		    set ti [ expr $i - 1 ]
		    $ay(tree) itemconfigure ${ay(CurrentLevel)}:$ti\
			-fill $ObjectSearch(HighlightColor)
		    $ay(tree) see ${ay(CurrentLevel)}:$ti
		    lappend ObjectSearch(nodes)\
			${ay(CurrentLevel)}:$ti
		}
		"Copy" {
		    copOb -add
		}
		"Collect" -
		"Delete" {
		    set ti [ expr $i - 1 ]
		    lappend ObjectSearch(nodes)\
			${ay(CurrentLevel)}:$ti
		}
		default {
		    eval $ObjectSearch(Action)
		}
	    }
	    # switch
	}
	# if
    }
    # forAll

    if { $gui != 0 } {
	ayError 4 searchOb "Done. Found $ObjectSearch(numfound) objects."
    }

    set l [llength $ObjectSearch(nodes)]
    if { $gui != 0 && $ObjectSearch(Action) == "Collect" } {
	ayError 4 searchOb "Nodes collected in ObjectSearch(nodes)."
    }
    if { $ObjectSearch(Action) == "Delete" && $l > 0 } {
	# arrange for nodes in a level to be in reverse order
	# so that we may delete nodes one by one without changing
	# the existing indices of the remaining nodes;
	# furthermore, should parents _and_ their (even indirect)
	# children be in the list of nodes, arrange for the children
	# to be removed from the list, as the corresponding objects
	# will be removed when deleting the respective parent anyway
	if { $l > 1 } {
	    set nodes [lreverse $ObjectSearch(nodes)]
	    set i 0
	    while { $i < $l } {
		set i1 [expr $i + 1]
		set n [lindex $nodes $i]
		set n1 [lindex $nodes $i1]
		if { [regexp -all {:} $n] < [regexp -all {:} $n1] } {
		    # n is higher in the hierarchy than n1,
		    # but is n indeed a (even indirect) parent of n1?
		    if { [string eq -length [string length $n] $n $n1] } {
			# yes, remove n1 (the child) from the list
			set nodes [lreplace $nodes $i1 $i1]
		    }
		}
		incr i
	    }
	} else {
	    set nodes $ObjectSearch(nodes)
	}
	foreach node $nodes {
	    treeSelect $node
	    catch {delOb}
	}
	# no use even trying to re-establish the old selection, as
	# the indices in ObjectSearch(oldselection) are probably
	# wrong now...
	uS
    } else {
	# re-establish old selection
	set ay(CurrentLevel) $ObjectSearch(oldclevel)
	set ay(SelectedLevel) $ObjectSearch(oldslevel)
	$ay(tree) selection set $ObjectSearch(oldselection)
	tree_handleSelection
	plb_update
    }

  return;
}
# searchOb


# searchObjects:
#  object search facility
#  finds objects matching user defined expressions
#  and highlights them in the object tree view or
#  executes user defined actions on them
proc objectsearch_open { } {
    global ay ayprefs ObjectSearch

    # check, whether tree view is open
    if { $ay(lb) != 0 } {
	ayError 2 searchObjects "Search only works for the tree view!"
	return;
    }

    winAutoFocusOff

    set oldFocus [focus]

    set w .searchw
    set t "Search Objects"
    winDialog $w $t

    set f [frame $w.f1]
    pack $f -in $w -side top -fill x

    set ay(bca) $w.fb.bca
    set ay(bok) $w.fb.bok

    if { $ayprefs(FixDialogTitles) == 1 } {
	addText $f ObjectSearch $t
    }

    # check/save original selection
    set sel ""
    getSel sel

    set name ""
    set type ""
    set mat ""
    set master ""

    if { $sel != "" } {
	# trim selection
	if { [llength $sel] > 1 } {
	    selOb [lindex $sel 0]
	}

	# get name/type/material of first selected object
	getName name
	getType type

	if { $type == "Material" } {
	    global MaterialAttrData
	    getProp
	    set mat $MaterialAttrData(Materialname)
	} else {
	    global ${type}_props
	    if { ! [info exists ${type}_props] } {
		init_${type}
	    }
	    if { [lsearch -exact ${type}_props Material] != -1 } {
		getMat
		set mat $matPropData(Materialname)
	    }
	    if { $type == "Instance" } {
		getMaster master
		set ObjectSearch(master) $master
	    } else {
		getAttr
		if { $::attrPropData(RefCount) > 0 } {
		    set ObjectSearch(master) \
			[lindex [$ay(tree) selection get] 0]
		    set master $ObjectSearch(master)
		}
	    }
	}

	# restore original selection
	selOb $sel
    }
    # if

    # build example expressions
    if { $master != "" || $type == "Instance" } {
	lappend expressions "Instances"
    }
    if { $type == "Instance" } {
	lappend expressions "Master"
    }
    if { $name != "" } {
	lappend expressions "\$name == \"$name\""
    }
    if { $type != "" } {
	lappend expressions "\$type == \"$type\""
    }
    if { $mat != "" } {
	lappend expressions "\$mat == \"$mat\""
    }
    if { $name == "" } {
	lappend expressions "\$name == \"name\""
    }
    if { $type == "" } {
	lappend expressions "\$type == \"type\""
    }
    if { $mat == "" } {
	lappend expressions "\$mat == \"matname\""
    }
    lappend expressions "\$SphereAttr(Radius) == 1.0"
    lappend expressions "myProc"

    if { ![info exists ObjectSearch(Action)] } {
	set ObjectSearch(Action) "Highlight"
    }

    # complete dialog GUI
    addString $w.f1 ObjectSearch Expression $expressions
    addString $w.f1 ObjectSearch Action\
	{Highlight Collect Copy Delete "\[myProc\]"}
    addColor $w ObjectSearch HighlightColor
    addMenu $w ObjectSearch Scope {All Selection Level Collection}
    addCheck $w ObjectSearch ClearHighlight
    addCheck $w ObjectSearch ClearClipboard

    set f [frame $w.fb]

    button $f.bok -text "Ok" -width 5 -command {
	set ObjectSearch(HighlightColor) [format "#%02x%02x%02x"\
	  $ObjectSearch(HighlightColor_R) $ObjectSearch(HighlightColor_G)\
	  $ObjectSearch(HighlightColor_B)]
	set aid [after 500 {mouseWatch 1 {. .tbw}}]

	searchOb $ObjectSearch(Expression) $ObjectSearch(Action) 1

	mouseWatch 0 {. .tbw}
	catch {after cancel $aid}
    }
    # ok

    button $f.bca -text "Cancel" -width 5 -command "\
      grab release $w;\
      restoreFocus $oldFocus;\
      destroy $w"

    pack $f.bok $f.bca -in $f -side left -fill x -expand yes
    pack $f -in $w -side bottom -fill x

    # Esc-key && close via window decoration == Cancel button
    bind $w <Escape> "$f.bca invoke"
    wm protocol $w WM_DELETE_WINDOW "$f.bca invoke"

    # establish "Help"-binding
    shortcut_addcshelp $w ayam-2.html objsearch

    winRestoreOrCenter $w $t
    grab $w
    focus $w.f1.fExpression.e
    tkwait window $w

    winAutoFocusOn

 return;
}
# objectsearch_open

array set ObjectSearch {
Scope All
ClearHighlight 1
HighlightColor_R 180
HighlightColor_G 0
HighlightColor_B 0
HighlightColor #b40000
}
