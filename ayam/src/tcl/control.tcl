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
		update
		catch {forAll_tree 1 $command}
		goUp
		set ay(CurrentLevel) $oldclevel
		$tree selection clear
		$tree selection set $sel
		treeSelect $sel
		plb_update
	    }
	}

	catch {eval $command}
    }

 return;
}
# forAll_tree

#
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
		goDown $sel
		uS
		$lb selection clear 0 end
		selOb
		catch {forAll 1 $command}
		goUp
		uS
		$lb selection set $sel
		selOb $sel
		plb_update
	    }
	}
	$lb selection clear 0 end
	$lb selection set $sel
	selOb $sel
	plb_update
	catch {eval $command}
    }

    uS
    plb_update

 return;
}
# forAll_lb


#
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


#
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
		update
		catch {forAll_tree 1 $command}
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
            catch {eval $command}
        }

    }

 return;
}
# forAllT_tree


#
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
		goDown $sel
		uS
		$lb selection clear 0 end
		selOb
		catch {forAll 1 $command}
		goUp
		uS
		$lb selection set $sel
		selOb $sel
		plb_update
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
            catch {eval $command}
        }
    }

    uS
    plb_update

 return;
}
# forAllT_lb

#
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
	catch {forAllT_tree $recursive $type $command}

	# recover state of tree widget
	$tree selection set $sel
	treeSelect $sel
	plb_update
	set ay(CurrentLevel) $clevel
	set ay(SelectedLevel) $slevel
	set ay(DropActive) 0
	rV
    } else {
	forAllT_lb $recursive $type $command


    }

 return;
}

# allow canceling of user Tcl-scripts in the console
bind .fl.con.console <Control-KeyPress-C> {set cancelled 1}
bind .fl.con.console <Control-KeyRelease-C> {set cancelled 0}

set cancelled 0

proc cancelled {} {
    global cancelled
    # give the user a chance to press Ctrl+C
    update
    if { $cancelled == 1 } {
	error "Cancelled!"
    }
}

rename while _while
proc while {condition body} {
    uplevel [list _while $condition "cancelled;$body"]
}

rename for _for
proc for {start test next body} {
    uplevel [list _for $start $test $next "cancelled;$body"]
}
