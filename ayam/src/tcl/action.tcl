# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2001 by Randolf Schultz
# (rschultz@informatik.uni-rostock.de) and others.
#
# All rights reserved.
#
# See the file License for details.

# action.tcl - interactive actions

set ay(LastSelection) ""

#stdReleaseBind:
# standard release binding: force notification (if neccesary);
# redraw all views; update property GUI
proc stdReleaseBind { w } {
    bind $w <ButtonRelease-1> {
	if { $ayprefs(LazyNotify) == 1 } { forceNot }
	rV
	plb_update
    }
}
# stdReleaseBind

#
proc actionRotView { w } {

    viewTitle $w "" "Rotate_View"

    bind $w <ButtonPress-1> {
	undo save
	%W mc
	set oldx %x
	set oldy %y
	update
    }

    bind $w <B1-Motion> {
	%W setconf -drotx [expr ($oldx - %x)] -droty [expr ($oldy - %y)]
	set oldx %x
	set oldy %y
	update
    }

    stdReleaseBind $w
}

#
proc actionMoveView { w } {

    viewTitle $w "" "Move_View"

    bind $w <ButtonPress-1> {
	undo save
	%W mc
	%W movevac -start %x %y
	update
    }

    bind $w <B1-Motion> {
	%W movevac -winxy %x %y
	update
    }

    stdReleaseBind $w
}

#
proc actionZoomView { w } {

    viewTitle $w "" "Zoom_View"

    bind $w <ButtonPress-1> {
	undo save
	%W mc
	%W zoomvac -start %y
	update
    }

    bind $w <B1-Motion> {
	%W zoomvac -winy %y
	update
    }

    stdReleaseBind $w
}

#
proc actionMoveZView { w } {

    viewTitle $w "" "MoveZ_View"

    bind $w <ButtonPress-1> {
	undo save
	%W mc
	%W movezvac -start %y
	update
    }

    bind $w <B1-Motion> {
	%W movezvac -winy %y
	update
    }

    stdReleaseBind $w
}

#
proc actionMoveOb { w } {

    viewTitle $w "" "Move"

    bind $w <ButtonPress-1> {
	undo save
	%W mc
	%W moveoac -start %x %y
	update
    }

    bind $w <B1-Motion> {
	%W moveoac -winxy %x %y
	update
    }

    stdReleaseBind $w
}

#
proc actionRotOb { w } {

    viewTitle $w "" "Rotate"

    bind $w <ButtonPress-1> {
	undo save
	%W mc
	%W rotoac -start %x %y
	update
    }

    bind $w <B1-Motion> {
	%W rotoac -winxy %x %y
	update
    }

    stdReleaseBind $w
}


proc actionRotObabindp { w x y } {

    viewTitle $w "" "Rotate_around_Point"

    bind $w <ButtonPress-1> "%W mc;%W rotoaac -start %x %y $x $y"
    bind $w <B1-Motion> "%W rotoaac -winxy %x %y $x $y"
}

proc actionRotObA { w } {

    viewTitle $w "" "Select_a_Point"

    bind $w <ButtonPress-1> {
	undo save
	%W mc
	update
	%W setconf -mark %x %y 1
	actionRotObabindp %W %x %y
    }
    bind $w <ButtonRelease-1> ""
    bind $w <B1-Motion> ""
}





#
proc actionSc1DXOb { w } {

    viewTitle $w "" "Scale1DX"

    bind $w <ButtonPress-1> {
	undo save
	%W mc
	%W sc1dxoac -start %x %y
	update
    }

    bind $w <B1-Motion> {
	%W sc1dxoac -winxy %x %y
	update
    }

    stdReleaseBind $w

}
# actionSc1DXOb

#
proc actionSc1DYOb { w } {

    viewTitle $w "" "Scale1DY"

    bind $w <ButtonPress-1> {
	undo save
	%W mc
	%W sc1dyoac -start %x %y
	update
    }

    bind $w <B1-Motion> {
	%W sc1dyoac -winxy %x %y
	update
    }

    stdReleaseBind $w

}
# actionSc1DYOb

#
proc actionSc1DZOb { w } {

    viewTitle $w "" "Scale1DZ"

    bind $w <ButtonPress-1> {
	undo save
	%W mc
	%W sc1dzoac -start %x %y
	update
    }

    bind $w <B1-Motion> {
	%W sc1dzoac -winxy %x %y
	update
    }

    stdReleaseBind $w

}
# actionSc1DZOb

#
proc actionSc2DOb { w } {

    viewTitle $w "" "Scale2D"

    bind $w <ButtonPress-1> {
	undo save
	%W mc
	%W sc2doac -start %x %y
	update
    }
    bind $w <B1-Motion> {
	%W sc2doac -winxy %x %y
	update
    }

    stdReleaseBind $w

}
# actionSc2DOb

#
proc actionSc3DOb { w } {

    viewTitle $w "" "Scale3D"

    bind $w <ButtonPress-1> {
	undo save
	%W mc
	%W sc3doac -start %x %y
	update
    }
    bind $w <B1-Motion> {
	%W sc3doac -winxy %x %y
	update
    }

    stdReleaseBind $w

}
# actionSc3DOb

#
proc actionStr2DOb { w } {

    viewTitle $w "" "Stretch2D"

    bind $w <ButtonPress-1> {
	undo save
	%W mc
	%W str2doac -start %x %y
	update
    }

    bind $w <B1-Motion> {
	%W str2doac -winxy %x %y
	update
    }

    stdReleaseBind $w

}
# actionStr2DOb

#
proc actionTagP { w } {

    viewTitle $w "" "Select_Points"

    bind $w <ButtonPress-1> {
#	undo save
	%W mc
	set oldx %x
	set oldy %y
	%W selpac %x %y
    }

    bind $w <ButtonRelease-1> {
	if { ($oldx != %x) || ($oldy != %y)} {
	    %W selpac %x %y $oldx $oldy
	}
	$w.f3D.togl setconf -rect $oldx $oldy %x %y 0
	rV
	update
    }

    bind $w <B1-Motion> {
	$w.f3D.togl setconf -rect $oldx $oldy %x %y 1
    }

}
# actionTagP

#
proc actionDelTagP { w } {
#    undo save
    $w deselpac
    rV

}
# actionDelTagP

array set editPointDarray {
    x 0.0
    x2 0.0
    y 0.0
    y2 0.0
    z 0.0
    z2 0.0
    w 0.0
    w2 0.0
    changed 0
    justupdated 0
}

#editPointDp:
# helper for actionDEditP
# directly edit coordinates of points
proc editPointDp { } {
upvar #0 editPointDarray array

set w .editPointDw

set array(changed) 0
set array(x2) $array(x)
set array(y2) $array(y)
set array(z2) $array(z)
set array(w2) $array(w)

if { [winfo exists $w] } {
    update
    set f1 $w.f1
    set f $f1.fx
    $f.e delete @0 end
    $f.e insert @0 $array(x)
    set f $f1.fy
    $f.e delete @0 end
    $f.e insert @0 $array(y)
    set f $f1.fz
    $f.e delete @0 end
    $f.e insert @0 $array(z)
    set f $f1.fw
    $f.e delete @0 end
    $f.e insert @0 $array(w)
    set array(justupdated) 1
    update
    set array(x2) [.editPointDw.f1.fx.e get]
    set array(y2) [.editPointDw.f1.fy.e get]
    set array(z2) [.editPointDw.f1.fz.e get]
    set array(w2) [.editPointDw.f1.fw.e get]

    return;
}

catch {destroy $w}
toplevel $w
wm title $w "Ayam - Edit_Points"
wm iconname $w "Ayam"
#wm transient $w .

set f [frame $w.f1]
pack $f -in $w -side top -fill x

set f [frame $f.fx]

label $f.l -text "X" -width 4
entry $f.e -width 8
$f.e insert @0 $array(x)

pack $f.l $f.e -in $f -padx 2 -pady 2 -side left -fill x -expand yes
pack $f -in $w.f1 -side top  -fill x


set f $w.f1
set f [frame $f.fy]

label $f.l -text "Y" -width 4
entry $f.e -width 8
$f.e insert @0 $array(y)

pack $f.l $f.e -in $f -padx 2 -pady 2 -side left -fill x -expand yes
pack $f -in $w.f1 -side top  -fill x


set f $w.f1
set f [frame $f.fz]

label $f.l -text "Z" -width 4
entry $f.e -width 8
$f.e insert @0 $array(z)

pack $f.l $f.e -in $f -padx 2 -pady 2 -side left -fill x -expand yes
pack $f -in $w.f1 -side top  -fill x

set f $w.f1
set f [frame $f.fw]

label $f.l -text "W" -width 4
entry $f.e -width 8
$f.e insert @0 $array(w)

pack $f.l $f.e -in $f -padx 2 -pady 2 -side left -fill x -expand yes
pack $f -in $w.f1 -side top  -fill x

update
set array(x2) [.editPointDw.f1.fx.e get]
set array(y2) [.editPointDw.f1.fy.e get]
set array(z2) [.editPointDw.f1.fz.e get]
set array(w2) [.editPointDw.f1.fw.e get]

set f [frame $w.f2]
button $f.bok -text "Ok" -width 5 -command { 
    upvar #0 editPointDarray array

    set array(x) [.editPointDw.f1.fx.e get]
    set array(y) [.editPointDw.f1.fy.e get]
    set array(z) [.editPointDw.f1.fz.e get]
    set array(w) [.editPointDw.f1.fw.e get]

    if { ($array(x) != $array(x2)) ||
         ($array(y) != $array(y2)) ||
         ($array(z) != $array(z2)) ||
         ($array(w) != $array(w2))} {
	     set array(changed) 1
    }


#    grab release .editPointDw
    destroy .editPointDw

}

button $f.bca -text "Cancel" -width 5 -command "destroy $w"

pack $f.bok $f.bca -in $f -side left -fill x -expand yes
pack $f -in $w -side bottom -fill x

winCenter $w
#grab $w
focus $f.bok
tkwait window $w
update
set array(justupdated) 0
return;
}
# editPointDp

#
proc actionDEditP { w } {

    viewTitle $w "" "Direct_Point_Edit"

    bind $w <ButtonPress-1> {
	undo save
	%W mc
	%W setconf -mark %x %y 1
	%W dpepac %x %y
	update
    }
    bind $w <B1-Motion> {}

}
# actionDEditP

#
proc actionEditP { w } {

    viewTitle $w "" "Edit_Points"

    bind $w <ButtonPress-1> {
	undo save
	%W mc
	%W startpepac %x %y
	%W pepac -start %x %y
    }

    bind $w <B1-Motion> {
	%W pepac -winxy %x %y
	%W render
    }

    stdReleaseBind $w

}
# actionEditP

#
proc actionEditWP { w } {

    viewTitle $w "" "Edit_Weights"

    bind $w <ButtonPress-1> {
	undo save
	%W startpepac %x %y
	%W wepac -start %x
    }

    bind $w <B1-Motion> {
	%W wepac -winx %x
	%W render
    }

    stdReleaseBind $w

}
# actionEditWP

#
proc actionResetWP { w } {

    $w wrpac
    rV
}
# actionResetWP


#
proc actionInsertP { w } {

    viewTitle $w "" "Insert_Points"

    bind $w <ButtonPress-1> {
	undo save
	%W mc
	%W insertpac %x %y
    }

    stdReleaseBind $w

}
# actionInsertP

#
proc actionDeleteP { w } {

    viewTitle $w "" "Delete_Points"

    bind $w <ButtonPress-1> {
	undo save
	%W mc
	%W deletepac %x %y
    }

    stdReleaseBind $w

}
# actionDeleteP

#
proc actionFindU { w } {

    viewTitle $w "" "Find_U"

    bind $w <ButtonPress-1> {
	%W mc
	%W finduac %x %y
    }

    bind $w <B1-Motion> { }

    bind $w <ButtonRelease-1> { %W finduac -end %x %y; %W redraw }

}
# actionFindU

#
proc actionSplitNC { w } {

    viewTitle $w "" "Split_Curve"

    bind $w <ButtonPress-1> {
	set ay_error 0
	%W mc
	%W finduac %x %y
    }

    bind $w <B1-Motion> { }

    bind $w <ButtonRelease-1> {
	%W finduac -end %x %y
	update
	if { $ay_error == 0 } {
	    splitNC $u
	    uCR; sL; rV
	}
	
    }

}
# actionSplitNC

#actionClear:
# not really an action, clears all bindings
# normally bound to the Esc-key
proc actionClear { w } {
    viewTitle $w "" "Pick"
    bind $w <ButtonPress-1> ""
    bind $w <B1-Motion> ""
    bind $w <ButtonRelease-1> ""
    set t [winfo toplevel $w]

    bind $w <ButtonPress-1> {
	set oldx %x
	set oldy %y
    }

    bind $w <ButtonRelease-1> {
	$w.f3D.togl setconf -rect $oldx $oldy %x %y 0

	if { [winfo exists .reconsider] == 0} {
	    if { ($oldx == %x) || ($oldy == %y)} {
		%W processObjSel node %x %y
		singleObjSel $node
	    } else {
		%W processObjSel node $oldx $oldy %x %y
		multipleObjSel $node
	    }
	    plb_update
	    rV
	    update
	}
    }

    bind $w <Shift-ButtonRelease-1> {
	$w.f3D.togl setconf -rect $oldx $oldy %x %y 0

	if { [winfo exists .reconsider] == 0} {
	    if { ($oldx == %x) || ($oldy == %y)} {
		%W processObjSel node %x %y
		addObjSel $node
	    } else {
		%W processObjSel node $oldx $oldy %x %y
		addMultipleObjSel $node
	    }
	    plb_update
	    rV
	    update
	}
    }
    
    bind $w <B1-Motion> {
	$w.f3D.togl setconf -rect $oldx $oldy %x %y 1
    }

    $t.f3D.togl setconf -mark 0 0 0
}
# actionClear
