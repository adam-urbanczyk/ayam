# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2001 by Randolf Schultz
# (randolf.schultz@gmail.com) and others.
#
# All rights reserved.
#
# See the file License for details.

# action.tcl - interactive actions

#actionEnd:
# bound to variable trace for ay(action) that in turn
# designates whether modeling actions are going on;
# if ay(action) is written and 0 (an action ended),
# we enforce an additional notification, so that
# objects may adapt their notification (do low quality
# but fast work, when a modelling action is going on,
# do high quality but slow work, when the modelling
# action finished)
proc actionEnd { n1 n2 op } {
    global ay

    if { $ay(action) == 0 } {
	forceNot
     }

 return;
}
# actionEnd

trace variable ay(action) w actionEnd


#actionBindRelease:
# standard release binding for modeling actions:
# force notification (via ay(action)/actionEnd above);
# redraw all views; update property GUI
proc actionBindRelease { w } {
    bind $w <ButtonRelease-1> {
	set ay(action) 0
	update
	rV
	plb_update
	focus %W
    }
 return;
}
# actionBindRelease


#actionSetMark:
# helper for all actions about a user specified point (the mark)
# e.g. rotate about and scale about
proc actionSetMark { w { nextaction "" } } {
    global ayviewshortcuts

    if { [string first ".view" $w] == 0 } {
	set w [winfo toplevel $w]
    }

    viewTitle $w "" "Mark Point"

    viewSetMAIcon $w.f3D.togl ay_Mark_img "Mark_Point"

    # set mark from mouse click
    bind $w.f3D.togl <ButtonPress-1> "\
	    %W mc;\
	    update;\
	    %W setconf -gmark %x %y 1;"

    # set mark from selected objects center of gravity
    bind $w $ayviewshortcuts(Center) "\
	    %W.f3D.togl mc;\
	    update;\
	    %W.f3D.togl setconf -cmark 0;"

    # set mark from selected points center of gravity
    bind $w $ayviewshortcuts(CenterP) "\
	    %W.f3D.togl mc;\
	    update;\
	    %W.f3D.togl setconf -cmark 1;"

    # if nextaction is not empty, we are an intermediate
    # action, embedded into some other action, which we arrange
    # to re-start here (after setting the mark):
    if { $nextaction != "" } {
	bind $w.f3D.togl <ButtonPress-1> "+ $nextaction %W;"
	# take over old mark
	bind $w <Key-Return> "\
          bind %W <Key-Return> \"\";\
          $nextaction %W.f3D.togl"
	bind $w $ayviewshortcuts(Center) "+ $nextaction %W.f3D.togl;"
	bind $w $ayviewshortcuts(CenterP) "+ $nextaction %W.f3D.togl;"
    }

 return;
}
# actionSetMark


#
proc actionRotView { w } {

    viewTitle $w "" "Rotate_View"
    viewSetMAIcon $w ay_RotV_img "Rotate_View"

    actionClearB1 $w

    bind $w <ButtonPress-1> {
	set ay(action) 1
	undo save RotView
	%W mc
	set oldx %x
	set oldy %y
	update
    }

    bind $w <B1-Motion> {
	%W setconf -drotx [expr {$oldx - %x}] -droty [expr {$oldy - %y}]
	set oldx %x
	set oldy %y
	update
    }

    bind $w <Motion> ""

    actionBindRelease $w

    $w setconf -drawh 0

 return;
}
# actionRotView


#
proc actionMoveView { w } {

    viewTitle $w "" "Move_View"
    viewSetMAIcon $w ay_MoveV_img "Move_View"

    actionClearB1 $w

    bind $w <ButtonPress-1> {
	set ay(action) 1
	undo save MovView
	%W mc
	%W movevac -start %x %y
	update
    }

    bind $w <B1-Motion> {
	%W movevac -winxy %x %y
	update
    }

    bind $w <Motion> ""

    actionBindRelease $w

    $w setconf -drawh 0

 return;
}
# actionMoveView


#
proc actionZoomView { w } {

    viewTitle $w "" "Zoom_View"
    viewSetMAIcon $w ay_ZoomV_img "Zoom_View"

    actionClearB1 $w

    bind $w <ButtonPress-1> {
	set ay(action) 1
	undo save ZoomView
	%W mc
	%W zoomvac -start %y
	update
    }

    bind $w <B1-Motion> {
	%W zoomvac -winy %y
	update
    }

    bind $w <Motion> ""

    actionBindRelease $w

    $w setconf -drawh 0
}
# actionZoomView


#
proc actionMoveZView { w } {

    viewTitle $w "" "MoveZ_View"
    viewSetMAIcon $w ay_MoveV_img "MoveZ_View"

    actionClearB1 $w

    bind $w <ButtonPress-1> {
	set ay(action) 1
	undo save MovZView
	%W mc
	%W movezvac -start %y
	update
    }

    bind $w <B1-Motion> {
	%W movezvac -winy %y
	update
    }

    bind $w <Motion> ""

    actionBindRelease $w

    $w setconf -drawh 0

 return;
}
# actionMoveZView


#
proc actionMoveOb { w } {
    global ay ayprefs ayviewshortcuts

    viewTitle $w "" "Move"
    viewSetMAIcon $w ay_Move_img "Move"

    actionClearB1 $w

    bind $w <ButtonPress-1> {
	set ay(action) 1
	undo save MoveObj
	%W mc
	if { $ay(restrict) > 0 } {
	    %W moveoac -start %x %y $ay(restrict)
	} else {
	    %W moveoac -start %x %y
	}
	update
    }

    bind $w <B1-Motion> {
	%W moveoac -winxy %x %y
    }

    bind $w <Motion> ""

    actionBindRelease $w

    $w setconf -drawh 1

    # always start unrestricted
    set ay(restrict) 0

    if { [string first ".view" $w] == 0 } {
	set w [winfo toplevel $w]
    } else {
	set w [winfo parent [winfo parent $w]]
    }

    # allow restriction: x only
    bind $w $ayviewshortcuts(RestrictX) "\
	set ay(restrict) 1;\
	viewSetMAIcon $w.f3D.togl ay_MoveX_img \"MoveX\";"

    # allow restriction: y only
    bind $w $ayviewshortcuts(RestrictY) "\
	set ay(restrict) 2;\
	viewSetMAIcon $w.f3D.togl ay_MoveY_img \"MoveY\";"

    # allow restriction: z only
    bind $w $ayviewshortcuts(RestrictZ) "\
	set ay(restrict) 3;\
	viewSetMAIcon $w.f3D.togl ay_MoveZ_img \"MoveZ\";"    

 return;
}
# actionMoveOb


#
proc actionRotOb { w } {
    global ay ayviewshortcuts

    viewTitle $w "" "Rotate"
    viewSetMAIcon $w ay_Rotate_img "Rotate"

    actionClearB1 $w

    bind $w <ButtonPress-1> {
	set ay(action) 1
	undo save RotObj
	%W mc
	%W rotoac -start %x %y
	update
    }

    bind $w <B1-Motion> {
	%W rotoac -winxy %x %y
	update
    }

    bind $w <Motion> ""

    actionBindRelease $w

    $w setconf -drawh 1

    if { [string first ".view" $w] == 0 } {
	set w [winfo toplevel $w]
    } else {
	set w [winfo parent [winfo parent $w]]
    }

    bind $w $ayviewshortcuts(About) { actionSetMark %W actionRotObA }

 return;
}
# actionRotOb


#
proc actionRotObA { w } {

    viewTitle $w "" "RotateA"
    viewSetMAIcon $w ay_RotateA_img "RotateA"

    actionClearB1 $w

    bind $w <ButtonPress-1> "set ay(action) 1; %W mc;\
	    %W rotoaac -start %x %y"
    bind $w <B1-Motion> "%W rotoaac -winxy %x %y"

    actionBindRelease $w

    bind $w <Motion> ""
    $w setconf -drawh 1

 return;
}
# actionRotObA


#
proc actionSc1DXOb { w } {
    global ay ayviewshortcuts

    viewTitle $w "" "Scale1DX"
    viewSetMAIcon $w ay_Scale1DX_img "Scale1DX"

    actionClearB1 $w

    bind $w <ButtonPress-1> {
	set ay(action) 1
	undo save Sc1DXObj
	%W mc
	%W sc1dxoac -start %x %y
	update
    }

    bind $w <B1-Motion> {
	%W sc1dxoac -winxy %x %y
	update
    }

    bind $w <Motion> ""

    actionBindRelease $w

    $w setconf -drawh 1

    if { [string first ".view" $w] == 0 } {
	set w [winfo toplevel $w]
    } else {
	set w [winfo parent [winfo parent $w]]
    }

    bind $w $ayviewshortcuts(About) { actionSetMark %W actionSc1DXAOb }

 return;
}
# actionSc1DXOb

#
proc actionSc1DXAOb { w } {

    viewTitle $w "" "Scale1DXA"
    viewSetMAIcon $w ay_Scale1DXA_img "Scale1DXA"

    actionClearB1 $w

    bind $w <ButtonPress-1> {
	set ay(action) 1
	undo save Sc1DXAObj
	%W mc
	%W sc1dxaoac -start %x %y
	update
    }

    bind $w <B1-Motion> {
	%W sc1dxaoac -winxy %x %y
	update
    }

    bind $w <Motion> ""

    actionBindRelease $w

    $w setconf -drawh 1

 return;
}
# actionSc1DXAOb


#
proc actionSc1DYOb { w } {
    global ay ayviewshortcuts

    viewTitle $w "" "Scale1DY"
    viewSetMAIcon $w ay_Scale1DY_img "Scale1DY"

    actionClearB1 $w

    bind $w <ButtonPress-1> {
	set ay(action) 1
	undo save Sc1DYObj
	%W mc
	%W sc1dyoac -start %x %y
	update
    }

    bind $w <B1-Motion> {
	%W sc1dyoac -winxy %x %y
	update
    }

    bind $w <Motion> ""

    actionBindRelease $w

    $w setconf -drawh 1

    if { [string first ".view" $w] == 0 } {
	set w [winfo toplevel $w]
    } else {
	set w [winfo parent [winfo parent $w]]
    }

    bind $w $ayviewshortcuts(About) { actionSetMark %W actionSc1DYAOb }

 return;
}
# actionSc1DYOb


#
proc actionSc1DYAOb { w } {

    viewTitle $w "" "Scale1DYA"
    viewSetMAIcon $w ay_Scale1DYA_img "Scale1DYA"

    actionClearB1 $w

    bind $w <ButtonPress-1> {
	set ay(action) 1
	undo save Sc1DYAObj
	%W mc
	%W sc1dyaoac -start %x %y
	update
    }

    bind $w <B1-Motion> {
	%W sc1dyaoac -winxy %x %y
	update
    }

    bind $w <Motion> ""

    actionBindRelease $w

    $w setconf -drawh 1

 return;
}
# actionSc1DYAOb


#
proc actionSc1DZOb { w } {
    global ay ayviewshortcuts

    viewTitle $w "" "Scale1DZ"
    viewSetMAIcon $w ay_Scale1DZ_img "Scale1DZ"

    actionClearB1 $w

    bind $w <ButtonPress-1> {
	set ay(action) 1
	undo save Sc1DZObj
	%W mc
	%W sc1dzoac -start %x %y
	update
    }

    bind $w <B1-Motion> {
	%W sc1dzoac -winxy %x %y
	update
    }

    bind $w <Motion> ""

    actionBindRelease $w

    $w setconf -drawh 1

    if { [string first ".view" $w] == 0 } {
	set w [winfo toplevel $w]
    } else {
	set w [winfo parent [winfo parent $w]]
    }
    bind $w $ayviewshortcuts(About) { actionSetMark %W actionSc1DZAOb }

 return;
}
# actionSc1DZOb


#
proc actionSc1DZAOb { w } {

    viewTitle $w "" "Scale1DZA"
    viewSetMAIcon $w ay_Scale1DZA_img "Scale1DZA"

    actionClearB1 $w

    bind $w <ButtonPress-1> {
	set ay(action) 1
	undo save Sc1DZAObj
	%W mc
	%W sc1dzaoac -start %x %y
	update
    }

    bind $w <B1-Motion> {
	%W sc1dzaoac -winxy %x %y
	update
    }

    bind $w <Motion> ""

    actionBindRelease $w

    $w setconf -drawh 1

 return;
}
# actionSc1DZAOb


#
proc actionSc2DOb { w } {
    global ay ayviewshortcuts

    viewTitle $w "" "Scale2D"
    viewSetMAIcon $w ay_Scale2D_img "Scale2D"

    actionClearB1 $w

    bind $w <ButtonPress-1> {
	set ay(action) 1
	undo save Sc2DObj
	%W mc
	%W sc2doac -start %x %y
	update
    }

    bind $w <B1-Motion> {
	%W sc2doac -winxy %x %y
	update
    }

    bind $w <Motion> ""

    actionBindRelease $w

    $w setconf -drawh 1

    if { [string first ".view" $w] == 0 } {
	set w [winfo toplevel $w]
    } else {
	set w [winfo parent [winfo parent $w]]
    }

    bind $w $ayviewshortcuts(About) { actionSetMark %W actionSc2DAOb }

    # allow restriction: x only
    bind $w $ayviewshortcuts(RestrictX) "actionSc1DXOb $w.f3D.togl"

    # allow restriction: y only
    bind $w $ayviewshortcuts(RestrictY) "actionSc1DYOb $w.f3D.togl"

    # allow restriction: z only
    bind $w $ayviewshortcuts(RestrictZ) "actionSc1DZOb $w.f3D.togl"    

 return;
}
# actionSc2DOb


#
proc actionSc2DAOb { w } {

    viewTitle $w "" "Scale2D"
    viewSetMAIcon $w ay_Scale2DA_img "Scale2DA"

    actionClearB1 $w

    bind $w <ButtonPress-1> {
	set ay(action) 1
	undo save Sc2DAObj
	%W mc
	%W sc2daoac -start %x %y
	update
    }

    bind $w <B1-Motion> {
	%W sc2daoac -winxy %x %y
	update
    }

    bind $w <Motion> ""

    actionBindRelease $w

    $w setconf -drawh 1

 return;
}
# actionSc2DAOb


#
proc actionSc3DOb { w } {
    global ay ayviewshortcuts

    viewTitle $w "" "Scale3D"
    viewSetMAIcon $w ay_Scale3D_img "Scale3D"

    actionClearB1 $w

    bind $w <ButtonPress-1> {
	set ay(action) 1
	undo save Sc3DObj
	%W mc
	%W sc3doac -start %x %y
	update
    }

    bind $w <B1-Motion> {
	%W sc3doac -winxy %x %y
	update
    }

    bind $w <Motion> ""

    actionBindRelease $w

    $w setconf -drawh 1

    if { [string first ".view" $w] == 0 } {
	set w [winfo toplevel $w]
    } else {
	set w [winfo parent [winfo parent $w]]
    }

    bind $w $ayviewshortcuts(About) { actionSetMark %W actionSc3DAOb }

 return;
}
# actionSc3DOb


#
proc actionSc3DAOb { w } {

    viewTitle $w "" "Scale3DA"
    viewSetMAIcon $w ay_Scale3DA_img "Scale3DA"

    actionClearB1 $w

    bind $w <ButtonPress-1> {
	set ay(action) 1
	undo save Sc3DAObj
	%W mc
	%W sc3daoac -start %x %y
	update
    }

    bind $w <B1-Motion> {
	%W sc3daoac -winxy %x %y
	update
    }

    bind $w <Motion> ""

    actionBindRelease $w

    $w setconf -drawh 1

 return;
}
# actionSc3DAOb


#
proc actionStr2DOb { w } {
    global ay ayviewshortcuts

    viewTitle $w "" "Stretch2D"
    viewSetMAIcon $w ay_Stretch2D_img "Stretch2D"

    actionClearB1 $w

    bind $w <ButtonPress-1> {
	set ay(action) 1
	undo save Str2DObj
	%W mc
	%W str2doac -start %x %y
	update
    }

    bind $w <B1-Motion> {
	%W str2doac -winxy %x %y
	update
    }

    bind $w <Motion> ""

    actionBindRelease $w

    $w setconf -drawh 1

    if { [string first ".view" $w] == 0 } {
	set w [winfo toplevel $w]
    } else {
	set w [winfo parent [winfo parent $w]]
    }

    bind $w $ayviewshortcuts(About) { actionSetMark %W actionStr2DAOb }

 return;
}
# actionStr2DOb


#
proc actionStr2DAOb { w } {

    viewTitle $w "" "Stretch2DA"
    viewSetMAIcon $w ay_Stretch2DA_img "Stretch2DA"

    actionClearB1 $w

    bind $w <ButtonPress-1> {
	set ay(action) 1
	undo save Str2DAObj
	%W mc
	%W str2daoac -start %x %y
	update
    }

    bind $w <B1-Motion> {
	%W str2daoac -winxy %x %y
	update
    }

    bind $w <Motion> ""

    actionBindRelease $w

    $w setconf -drawh 1

 return;
}
# actionStr2DAOb


#
proc actionTagP { w } {
    global ayprefs ayviewshortcuts

    viewTitle $w "" "Select_Points"
    viewSetMAIcon $w ay_Tag_img "Select_Points"

    actionClearB1 $w

    bind $w <ButtonPress-1> {
#	undo save
	%W mc
	set oldx %x
	set oldy %y
    }

    bind $w <ButtonRelease-1> {
	if { ($oldx != %x) || ($oldy != %y)} {
	    %W selpac %x %y $oldx $oldy
	} else {
	    %W selpac %x %y
	}
	%W setconf -rect $oldx $oldy %x %y 0
	rV
	update
	focus %W
    }

    bind $w <${ayviewshortcuts(TagMod)}-ButtonRelease-1> {
	if { ($oldx != %x) || ($oldy != %y)} {
	    %W selpac %x %y $oldx $oldy 1
	} else {
	    %W selpac %x %y
	}
	%W setconf -rect $oldx $oldy %x %y 0
	rV
	update
	focus %W
    }

    bind $w <B1-Motion> {
	%W setconf -rect $oldx $oldy %x %y 1
    }

    bind $w <Motion> ""

    if { $ayprefs(FlashPoints) == 1 } {
	bind $w <Motion> {
	    %W startpepac %x %y -flash
	}
	if { $ayprefs(FixFlashPoints) == 1 } {
	    bind $w <ButtonRelease-1> "+\
          %W startpepac %x %y -flash -ignoreold;\
          %W startpepac %x %y -flash -ignoreold"
	    bind $w <Shift-ButtonRelease-1> "+\
          %W startpepac %x %y -flash -ignoreold;\
          %W startpepac %x %y -flash -ignoreold"
	} else {
	    bind $w <ButtonRelease-1> "+\
          %W startpepac %x %y -flash -ignoreold;"
	    bind $w <Shift-ButtonRelease-1> "+\
          %W startpepac %x %y -flash -ignoreold;"
	}

    }
    $w setconf -drawh 1

 return;
}
# actionTagP


#
proc actionDelTagP { w } {
#    undo save
    $w deselpac
    rV

 return;
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

    lx 0.0
    ly 0.0
    lz 0.0
    lw 0.0

    wx 0.0
    wy 0.0
    wz 0.0
    ww 0.0

    changed 0
    window ""
    valid 0
    local 1
}
# array

# updEditPointDarray:
# helper for editPointDp
proc updEditPointDarray { w } {
    upvar #0 editPointDarray array

    if { $array(local) == 1 } {
	set array(x) $array(lx)
	set array(y) $array(ly)
	set array(z) $array(lz)
	set array(w) $array(lw)
    } else {
	set array(x) $array(wx)
	set array(y) $array(wy)
	set array(z) $array(wz)
	set array(w) $array(ww)
    }

    set array(changed) 0
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

    update
    set array(x2) [$w.f1.fx.e get]
    set array(y2) [$w.f1.fy.e get]
    set array(z2) [$w.f1.fz.e get]
    set array(w2) [$w.f1.fw.e get]

 return;
}
# updEditPointDarray


#editPointApply:
# helper for actionDEditP
# apply the changes from the dialog
proc editPointApply { } {
    global ay
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
	if { [winfo exists $array(window)] } {
	    undo save DEditPnt
	    $array(window) dpepac -apply
	    rV
	    plb_update
	} else {
	    ayError 2 "editPointDp" "Lost window to apply changes to!"
	}
    }
    # if

 return;
}
# editPointApply


#editPointDp:
# helper for actionDEditP
# directly edit coordinates of points
proc editPointDp { } {
    upvar #0 editPointDarray array
    global ay tcl_platform AYWITHAQUA

    set w .editPointDw

    set array(changed) 0

    if { [winfo exists $w] } {
	updEditPointDarray $w
	return;
    }

    catch {destroy $w}
    toplevel $w -class Ayam
    wm title $w "Edit_Point"
    wm iconname $w "Ayam"
    if { ! $AYWITHAQUA } {
	wm transient $w .
    } else {
	winMakeFloat $w
    }

    set f [frame $w.f1]
    pack $f -in $w -side top -fill x

    set f [frame $f.fm]
    menubutton $f.mb -text "Local" -menu $f.mb.m -relief raised\
	    -padx 0 -pady 1
    if { $tcl_platform(platform) == "windows" } {
	$f.mb configure -pady 1
    }
    if { $AYWITHAQUA } {
	$f.mb configure -pady 2
    }
    set m [menu $f.mb.m -tearoff 0]
    $m add command -label "Local" -command {
	global editPointDarray
	set editPointDarray(local) 1
	.editPointDw.f1.fm.mb configure -text "Local"
	updEditPointDarray .editPointDw
    }
    $m add command -label "World" -command {
	global editPointDarray
	set editPointDarray(local) 0
	.editPointDw.f1.fm.mb configure -text "World"
	updEditPointDarray .editPointDw
    }
    pack $f.mb -in $f -side left -fill x -expand yes -pady 0
    pack $f -in $w.f1 -side top -fill x

    set f $w.f1
    set f [frame $f.fx]

    label $f.l -text "X" -width 4
    entry $f.e -width 8
    pack $f.l -in $f -padx 2 -pady 2 -side left -fill x -expand no
    pack $f.e -in $f -padx 2 -pady 2 -side left -fill x -expand yes
    pack $f -in $w.f1 -side top  -fill x

    bind $f.e <Key-Return>  "editPointApply;break"
    catch {bind $f.e <Key-KP_Enter>  "editPointApply;break"}

    set f $w.f1
    set f [frame $f.fy]

    label $f.l -text "Y" -width 4
    entry $f.e -width 8
    pack $f.l -in $f -padx 2 -pady 2 -side left -fill x -expand no
    pack $f.e -in $f -padx 2 -pady 2 -side left -fill x -expand yes
    pack $f -in $w.f1 -side top  -fill x

    bind $f.e <Key-Return>  "editPointApply;break"
    catch {bind $f.e <Key-KP_Enter>  "editPointApply;break"}

    set f $w.f1
    set f [frame $f.fz]

    label $f.l -text "Z" -width 4
    entry $f.e -width 8
    pack $f.l -in $f -padx 2 -pady 2 -side left -fill x -expand no
    pack $f.e -in $f -padx 2 -pady 2 -side left -fill x -expand yes
    pack $f -in $w.f1 -side top  -fill x

    bind $f.e <Key-Return>  "editPointApply;break"
    catch {bind $f.e <Key-KP_Enter>  "editPointApply;break"}

    set f $w.f1
    set f [frame $f.fw]

    label $f.l -text "W" -width 4
    entry $f.e -width 8
    pack $f.l -in $f -padx 2 -pady 2 -side left -fill x -expand no
    pack $f.e -in $f -padx 2 -pady 2 -side left -fill x -expand yes
    pack $f -in $w.f1 -side top  -fill x

    bind $f.e <Key-Return>  "editPointApply;break"
    catch {bind $f.e <Key-KP_Enter>  "editPointApply;break"}

    update
    updEditPointDarray $w

    set f [frame $w.f2]
    button $f.bok -text "Apply" -width 5 -pady $ay(pady)\
	-command editPointApply
    button $f.bca -text "Cancel" -width 5 -pady $ay(pady) -command "\
	    global ay;\
	    if { [winfo exists \$ay(currentView)] } {\
	    focus \$ay(currentView) } else { resetFocus };\
	    destroy $w"

    pack $f.bok $f.bca -in $f -side left -fill x -expand yes
    pack $f -in $w -side bottom -fill x

    winCenter $w

    # auto raise window, when obscured
    bind $w <Visibility> {
	# are we obscured?
	if { "%s" == "VisibilityPartiallyObscured" ||\
		"%s" == "VisibilityFullyObscured" } {
	    # yes: try to raise the window, but just one time
	    raise [winfo toplevel %W]
	    bind %W <Visibility> ""
	}
    }

    # Esc-key && close via window decoration == Cancel button
    bind $w <Escape> "$w.f2.bca invoke"
    wm protocol $w WM_DELETE_WINDOW "$w.f2.bca invoke"

    bind $w <Key-Return> "editPointApply;break"
    catch {bind $w <Key-KP_Enter> "editPointApply;break"}

    focus $f.bok

 return;
}
# editPointDp


#
proc actionDEditP { w } {
    global ayprefs

    viewTitle $w "" "Direct_Point_Edit"
    viewSetMAIcon $w ay_EditD_img "Direct_Point_Edit"

    actionClearB1 $w

    bind $w <ButtonPress-1> {
	%W mc
	set editPointDarray(valid) 0
	%W startpepac %x %y -flash -ignoreold
	%W dpepac -start %x %y
	set editPointDarray(window) %W
	if { $editPointDarray(valid) == 1 } {
	    editPointDp
	    %W setconf -mark %x %y 1
	}
	update
    }

    bind $w <B1-Motion> ""
    bind $w <Motion> ""
    bind $w <ButtonRelease-1> "focus %W"

    if { $ayprefs(FlashPoints) == 1 } {
	bind $w <Motion> {
	    %W startpepac %x %y -flash
	}
    }
    #if

    $w setconf -drawh 1

 return;
}
# actionDEditP


#
proc actionEditP { w } {
    global ayprefs

    viewTitle $w "" "Edit_Points"
    viewSetMAIcon $w ay_Edit_img "Edit_Points"

    actionClearB1 $w

    bind $w <ButtonPress-1> {
	set ay(action) 1
	undo save EditPnt
	%W mc
	if { $ayprefs(FlashPoints) == 1 } {
	    %W startpepac %x %y -flash
	} else {
	    %W startpepac %x %y
	}

	%W pepac -start %x %y
    }

    bind $w <B1-Motion> {
	%W pepac -winxy %x %y
    }

    if { $ayprefs(FlashPoints) == 1 } {
	bind $w <B1-Motion> {
	    %W pepac -winxy %x %y -flash
	}
	bind $w <Motion> {
	    %W startpepac %x %y -flash
	}
    }

    actionBindRelease $w

    if { $ayprefs(FlashPoints) == 1 } {
	if { $ayprefs(FixFlashPoints) == 1 } {
	    bind $w <ButtonRelease-1> "+\
          %W startpepac %x %y -flash ;\
          %W startpepac %x %y -flash "
	} else {
	    bind $w <ButtonRelease-1> "+\
          %W startpepac %x %y -flash ;"
	}
    }

    $w setconf -drawh 1

 return;
}
# actionEditP


#
proc actionEditWP { w } {
    global ayprefs

    viewTitle $w "" "Edit_Weights"
    viewSetMAIcon $w ay_EditW_img "Edit_Weights"

    actionClearB1 $w

    bind $w <ButtonPress-1> {
	set ay(action) 1
	undo save EditWPnt
	if { $ayprefs(FlashPoints) == 1 } {
	    %W startpepac %x %y -flash
	} else {
	    %W startpepac %x %y
	}
	%W wepac -start %x
    }

    bind $w <B1-Motion> {
	%W wepac -winx %x
	%W render
    }

    if { $ayprefs(FlashPoints) == 1 } {
	bind $w <Motion> {
	    %W startpepac %x %y -flash
	}
    }

    actionBindRelease $w

    if { $ayprefs(FlashPoints) == 1 } {
	if { $ayprefs(FixFlashPoints) == 1 } {
	    bind $w <ButtonRelease-1> "+\
          %W startpepac %x %y -flash ;\
          %W startpepac %x %y -flash "
	} else {
	    bind $w <ButtonRelease-1> "+\
          %W startpepac %x %y -flash "
	}
    }

    $w setconf -drawh 1

 return;
}
# actionEditWP


#
proc actionResetWP { w } {
    undo save ResetWeight
    $w wrpac
    rV
 return;
}
# actionResetWP


#
proc actionInsertP { w } {
    global ayprefs

    viewTitle $w "" "Insert_Points"
    viewSetMAIcon $w ay_Insert_img "Insert_Points"

    actionClearB1 $w

    bind $w <ButtonPress-1> {
	undo save InsPnt
	%W mc
	%W insertpac %x %y
    }

    bind $w <B1-Motion> ""

    bind $w <Motion> ""

    if { $ayprefs(FlashPoints) == 1 } {
	bind $w <Motion> {
	    %W startpepac %x %y -flash
	}
    }

    actionBindRelease $w

    if { $ayprefs(FlashPoints) == 1 } {
	bind $w <ButtonRelease-1> "+%W startpepac %x %y -flash -ignoreold"
    }

    $w setconf -drawh 1

 return;
}
# actionInsertP


#
proc actionDeleteP { w } {
    global ayprefs

    viewTitle $w "" "Delete_Points"
    viewSetMAIcon $w ay_Delete_img "Delete_Points"

    actionClearB1 $w

    bind $w <ButtonPress-1> {
	undo save DelPnt
	%W mc
	%W deletepac %x %y
    }

    bind $w <B1-Motion> ""

    bind $w <Motion> ""

    if { $ayprefs(FlashPoints) == 1 } {
	bind $w <Motion> {
	    %W startpepac %x %y -flash
	}
    }

    actionBindRelease $w

    if { $ayprefs(FlashPoints) == 1 } {
	bind $w <ButtonRelease-1> "+%W startpepac %x %y -flash -ignoreold"
    }

    $w setconf -drawh 1

 return;
}
# actionDeleteP


#
proc actionFindU { w } {

    viewTitle $w "" "Find_U"
    viewSetMAIcon $w ay_FindU_img "Find_U"

    actionClearB1 $w

    bind $w <ButtonPress-1> {
	%W mc
	%W finduac %x %y
    }

    bind $w <B1-Motion> ""

    bind $w <Motion> ""

    bind $w <ButtonRelease-1> {
	%W finduac -end %x %y
	%W redraw
	focus %W
    }

    $w setconf -drawh 0

 return;
}
# actionFindU


#
proc actionSplitNC { w } {

    viewTitle $w "" "Split_Curve"
    viewSetMAIcon $w ay_Split_img "Split_Curve"

    actionClearB1 $w

    bind $w <ButtonPress-1> {
	set ay_error 0
	%W mc
	%W finduac %x %y
    }

    bind $w <B1-Motion> ""

    bind $w <Motion> ""

    bind $w <ButtonRelease-1> {
	%W finduac -end %x %y
	update
	if { $ay_error == 0 } {
	    undo save Split
	    splitNC $u
	    uCR; sL; rV
	}
	focus %W
    }

    $w setconf -drawh 0

 return;
}
# actionSplitNC


#actionPick:
# establish object picking bindings
proc actionPick { w } {
    global ayviewshortcuts

    viewTitle $w "" "Pick"
    viewSetMAIcon $w ay_Pick_img "Pick"

    bind $w <ButtonPress-1> {
	set oldx %x
	set oldy %y
    }

    bind $w <ButtonRelease-1> {
	%W setconf -rect $oldx $oldy %x %y 0

	if { [winfo exists .reconsider] == 0} {
	    if { ($oldx == %x) || ($oldy == %y)} {
		%W processObjSel node %x %y
		singleObjSel $node
	    } else {
		%W processObjSel node $oldx $oldy %x %y
		multipleObjSel $node
	    }
	    rV
	    update
	}

	focus %W
    }

    bind $w <${ayviewshortcuts(PickMod)}-ButtonRelease-1> {
	%W setconf -rect $oldx $oldy %x %y 0

	if { [winfo exists .reconsider] == 0} {
	    if { ($oldx == %x) || ($oldy == %y)} {
		%W processObjSel node %x %y
		addObjSel $node
	    } else {
		%W processObjSel node $oldx $oldy %x %y
		addMultipleObjSel $node
	    }
	    rV
	    update
	}

	focus %W
    }

    bind $w <B1-Motion> {
	%W setconf -rect $oldx $oldy %x %y 1
    }

    bind $w <Motion> ""

    if { [string first ".view" $w] == 0 } {
	set t [winfo toplevel $w].f3D.togl
    } else {
	set t $w
    }

    $t setconf -drawh 0
    $t setconf -mark 0 0 0

 return;
}
#actionPick


#
proc actionSnapToGrid3D { w } {
    undo save SnapToGrid3D
    $w snapac
    rV
 return;
}
# actionSnapToGrid3D


#
proc actionSnapToGrid2D { w } {
    undo save SnapToGrid2D
    $w snapac 1
    rV
 return;
}
# actionSnapToGrid2D


#actionClearB1:
# helper procedure to clear all bindings to mouse button 1;
# all modeling actions call this before adding their bindings
proc actionClearB1 { w } {
    global ayviewshortcuts

    bind $w <B1-Motion> ""
    bind $w <ButtonPress-1> ""
    bind $w <ButtonRelease-1> ""
    bind $w <${ayviewshortcuts(PickMod)}-ButtonRelease-1> ""
    bind $w <${ayviewshortcuts(TagMod)}-ButtonRelease-1> ""

 return;
}
# actionClearB1


#actionClear:
# not really an action, clears all bindings, establishes picking bindings
# when requested via preferences and is normally bound to the Esc-key
proc actionClear { w } {
    global ayprefs ayviewshortcuts

    actionClearB1 $w

    if { [string first ".view" $w] != 0 } {
	bind $w <ButtonPress-1> "focus \$w"
    } else {
	bind $w <ButtonPress-1> ""
    }

    bind $w <Motion> ""

    if { $ayprefs(DefaultAction) == 0 } {
	viewTitle $w "" "None"
	viewSetMAIcon $w ay_Empty_img ""

	if { [string first ".view" $w] == 0 } {
	    set t [winfo toplevel $w].f3D.togl
	} else {
	    set t $w
	}

	$t setconf -drawh 0
	$t setconf -mark 0 0 0
    } else {
	actionPick $w
    }

    set p [winfo parent [winfo parent $w]]
    after 100 "bind $p <$ayviewshortcuts(Break)> \"resetFocus\""
    after 1000 "bind $p <$ayviewshortcuts(Break)> \"actionClear $p.f3D.togl\""

 return;
}
# actionClear
