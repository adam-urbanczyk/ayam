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
	# only initiate notification for modified objects
	# this also avoids unneeded lengthy notification runs in
	# view actions (view-rotate, view-move etc.)
	notifyOb -mod
	$ay(currentView) setconf -a 0
    } else {
	$ay(currentView) setconf -a 1
    }

 return;
}
# actionEnd

trace variable ay(action) w actionEnd


#actionBindRelease:
# standard release binding for modeling actions:
# force notification (via ay(action)/actionEnd above);
# redraw all views; update property GUI
proc actionBindRelease { w {normalize 1} } {
    if { $normalize } {
	bind $w <ButtonRelease-1> {
	    set sel ""
	    getSel sel
	    if { $sel != "" } {
		if { $ay(cVPnts) } {
		    if { $ayprefs(NormalizePoints) } {
			normPnts
		    }
		} else {
		    if { $ayprefs(NormalizeTrafos) } {
			normTrafos;getTrafo
		    }
		}
            }
        }
    }
    bind $w <ButtonRelease-1> {+
	set ay(action) 0
	update
	rV
	plb_update
	focus %W
    }
 return;
}
# actionBindRelease


#actionBindCenter:
# establish standard bindings for some modelling actions
# that switch to about cog immediately, without going
# the detour via actionSetMark below
proc actionBindCenter { w { nextaction "" } } {
    global ayviewshortcuts

    if { [string first ".view" $w] == 0 } {
	set t [winfo toplevel $w]
    } else {
	set t [winfo parent [winfo parent $w]]
    }

    # set mark from selected objects center of gravity
    bind $t $ayviewshortcuts(CenterO) {
	if { [string first ".view" %W] == 0 } {
	    set w [winfo toplevel %W]
	} else {
	    set w %W
	}
	$w.f3D.togl mc
	$w.f3D.togl setconf -cmark 0
    }

    # set mark from selected points center of gravity
    bind $t $ayviewshortcuts(CenterPC) {
	if { [string first ".view" %W] == 0 } {
	    set w [winfo toplevel %W]
	} else {
	    set w %W
	}
	$w.f3D.togl mc
	$w.f3D.togl setconf -cmark 2
    }

    # set mark from selected points bounding box center
    bind $t $ayviewshortcuts(CenterPB) {
	if { [string first ".view" %W] == 0 } {
	    set w [winfo toplevel %W]
	} else {
	    set w %W
	}
	$w.f3D.togl mc
	$w.f3D.togl setconf -cmark 1
    }

    if { $nextaction != "" } {
	bind $t $ayviewshortcuts(CenterO) "+ $nextaction \$w.f3D.togl;"
	bind $t $ayviewshortcuts(CenterPC) "+ $nextaction \$w.f3D.togl;"
	bind $t $ayviewshortcuts(CenterPB) "+ $nextaction \$w.f3D.togl;"
    }

 return;
}
# actionBindCenter


#actionSetMark:
# helper for all actions about a user specified point (the mark)
# e.g. rotate about and scale about
# but also a full fledged action to set the mark
proc actionSetMark { w { nextaction "" } } {
    global ayprefs

    viewTitle $w "" "Mark Point"

    viewSetMAIcon $w ay_Mark_img "Mark_Point"

    if { $nextaction == "" } {
	actionClearB1 $w
    }

    # set mark from mouse click
    bind $w <ButtonPress-1> {
	%W mc
	%W setconf -mark %x %y
    }

    # if nextaction is not empty, we are an intermediate
    # action, embedded into some other action, which we arrange
    # to re-start here (after setting the mark):
    if { $nextaction != "" } {

	if { [string first ".view" $w] == 0 } {
	    set t [winfo toplevel $w]
	} else {
	    set t [winfo parent [winfo parent $w]]
	}

	bind $w <ButtonPress-1> "+ $nextaction %W;"
	# take over old mark
	bind $t <Key-Return> "bind $t <Key-Return> \"\";\
                              $nextaction $t.f3D.togl;"

    }

    actionBindCenter $w $nextaction

    if { $ayprefs(FlashPoints) == 1 } {
	bind $w <Motion> {
	    %W startpepac %x %y -readonly -flash
	}
	if { $ayprefs(FixFlashPoints) == 1 } {
	    bind $w <ButtonRelease-1> "+\
          %W startpepac %x %y -readonly -flash -ignoreold;\
          %W startpepac %x %y -readonly -flash -ignoreold"
	    bind $w <Shift-ButtonRelease-1> "+\
          %W startpepac %x %y -readonly -flash -ignoreold;\
          %W startpepac %x %y -readonly -flash -ignoreold"
	} else {
	    bind $w <ButtonRelease-1> "+\
          %W startpepac %x %y -readonly -flash -ignoreold;"
	    bind $w <Shift-ButtonRelease-1> "+\
          %W startpepac %x %y -readonly -flash -ignoreold;"
	}
    }

    $w setconf -drawh 1

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
	%W mc
	if { $ay(cVUndo) } {
	    undo save RotView
	}
	set oldx %x
	set oldy %y
    }

    bind $w <B1-Motion> {
	%W setconf -drotx [expr {$oldx - %x}] -droty [expr {$oldy - %y}]
	set oldx %x
	set oldy %y
    }

    bind $w <Motion> ""

    actionBindRelease $w 0

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
	%W mc
	if { $ay(cVUndo) } {
	    undo save MovView
	}
	%W movevac -start %x %y
    }

    bind $w <B1-Motion> {
	%W movevac -winxy %x %y
    }

    bind $w <Motion> ""

    actionBindRelease $w 0

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
	%W mc
	if { $ay(cVUndo) } {
	    undo save ZoomView
	}
	%W zoomvac -start %y
    }

    bind $w <B1-Motion> {
	%W zoomvac -winy %y
    }

    bind $w <Motion> ""

    actionBindRelease $w 0

    $w setconf -drawh 0
}
# actionZoomView


#
proc actionMoveZView { w } {

    viewTitle $w "" "MoveZ_View"
    viewSetMAIcon $w ay_MoveVZ_img "MoveZ_View"

    actionClearB1 $w

    bind $w <ButtonPress-1> {
	set ay(action) 1
	%W mc
	if { $ay(cVUndo) } {
	    undo save MovZView
	}
	%W movezvac -start %y
    }

    bind $w <B1-Motion> {
	%W movezvac -winy %y
    }

    bind $w <Motion> ""

    actionBindRelease $w 0

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
	set t [winfo toplevel $w]
    } else {
	set t [winfo parent [winfo parent $w]]
    }

    # allow restriction: x only
    bind $t $ayviewshortcuts(RestrictX) "\
	set ay(restrict) 1;\
	viewSetMAIcon $t.f3D.togl ay_MoveX_img \"MoveX\";"

    # allow restriction: y only
    bind $t $ayviewshortcuts(RestrictY) "\
	set ay(restrict) 2;\
	viewSetMAIcon $t.f3D.togl ay_MoveY_img \"MoveY\";"

    # allow restriction: z only
    bind $t $ayviewshortcuts(RestrictZ) "\
	set ay(restrict) 3;\
	viewSetMAIcon $t.f3D.togl ay_MoveZ_img \"MoveZ\";"

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
    }

    bind $w <B1-Motion> {
	%W rotoac -winxy %x %y
    }

    bind $w <Motion> ""

    actionBindRelease $w

    $w setconf -drawh 1

    if { [string first ".view" $w] == 0 } {
	set t [winfo toplevel $w]
    } else {
	set t [winfo parent [winfo parent $w]]
    }

    bind $t $ayviewshortcuts(About) "\
	$w mc;\
	if { \$ay(cVDrawMark) } {\
	    actionRotObA $w;\
	} else {\
	    actionSetMark $w actionRotObA;\
	}"

    actionBindCenter $w actionRotObA

 return;
}
# actionRotOb


#
proc actionRotObA { w } {
    global ayprefs ayviewshortcuts

    viewTitle $w "" "RotateA"
    viewSetMAIcon $w ay_RotateA_img "RotateA"

    actionClearB1 $w

    bind $w <ButtonPress-1> {
	set ay(action) 1
	undo save RotObjA
	%W mc
	%W rotoaac -start %x %y
    }
    bind $w <B1-Motion> {
	%W rotoaac -winxy %x %y
    }

    actionBindRelease $w

    bind $w <Motion> ""

    $w setconf -drawh 1

    if { [string first ".view" $w] == 0 } {
	set t [winfo toplevel $w]
    } else {
	set t [winfo parent [winfo parent $w]]
    }
    bind $t $ayviewshortcuts(About) "actionSetMark $w actionRotObA"

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
    }

    bind $w <B1-Motion> {
	%W sc1dxoac -winxy %x %y
    }

    bind $w <Motion> ""

    actionBindRelease $w

    $w setconf -drawh 1

    if { [string first ".view" $w] == 0 } {
	set t [winfo toplevel $w]
    } else {
	set t [winfo parent [winfo parent $w]]
    }

    bind $t $ayviewshortcuts(About) "\
	$w mc;\
	if { \$ay(cVDrawMark) } {\
	    actionSc1DXAOb $w;\
	} else {\
	    actionSetMark $w actionSc1DXAOb;\
	}"

    actionBindCenter $w actionSc1DXAOb

 return;
}
# actionSc1DXOb


#
proc actionSc1DXAOb { w } {
    global ayprefs ayviewshortcuts

    viewTitle $w "" "Scale1DXA"
    viewSetMAIcon $w ay_Scale1DXA_img "Scale1DXA"

    actionClearB1 $w

    bind $w <ButtonPress-1> {
	set ay(action) 1
	undo save Sc1DXAObj
	%W mc
	%W sc1dxaoac -start %x %y
    }

    bind $w <B1-Motion> {
	%W sc1dxaoac -winxy %x %y
    }

    bind $w <Motion> ""

    actionBindRelease $w

    $w setconf -drawh 1

    if { [string first ".view" $w] == 0 } {
	set t [winfo toplevel $w]
    } else {
	set t [winfo parent [winfo parent $w]]
    }
    bind $t $ayviewshortcuts(About) "actionSetMark $w actionSc1DXAOb"

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
    }

    bind $w <B1-Motion> {
	%W sc1dyoac -winxy %x %y
    }

    bind $w <Motion> ""

    actionBindRelease $w

    $w setconf -drawh 1

    if { [string first ".view" $w] == 0 } {
	set t [winfo toplevel $w]
    } else {
	set t [winfo parent [winfo parent $w]]
    }

    bind $t $ayviewshortcuts(About) "\
	$w mc;\
	if { \$ay(cVDrawMark) } {\
	    actionSc1DYAOb $w;\
	} else {\
	    actionSetMark $w actionSc1DYAOb;\
	}"

    actionBindCenter $w actionSc1DYAOb

 return;
}
# actionSc1DYOb


#
proc actionSc1DYAOb { w } {
    global ayprefs ayviewshortcuts

    viewTitle $w "" "Scale1DYA"
    viewSetMAIcon $w ay_Scale1DYA_img "Scale1DYA"

    actionClearB1 $w

    bind $w <ButtonPress-1> {
	set ay(action) 1
	undo save Sc1DYAObj
	%W mc
	%W sc1dyaoac -start %x %y
    }

    bind $w <B1-Motion> {
	%W sc1dyaoac -winxy %x %y
    }

    bind $w <Motion> ""

    actionBindRelease $w

    $w setconf -drawh 1

    if { [string first ".view" $w] == 0 } {
	set t [winfo toplevel $w]
    } else {
	set t [winfo parent [winfo parent $w]]
    }
    bind $t $ayviewshortcuts(About) "actionSetMark $w actionSc1DYAOb"

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
    }

    bind $w <B1-Motion> {
	%W sc1dzoac -winxy %x %y
    }

    bind $w <Motion> ""

    actionBindRelease $w

    $w setconf -drawh 1

    if { [string first ".view" $w] == 0 } {
	set t [winfo toplevel $w]
    } else {
	set t [winfo parent [winfo parent $w]]
    }

    bind $t $ayviewshortcuts(About) "\
	$w mc;\
	if { \$ay(cVDrawMark) } {\
	    actionSc1DZAOb $w;\
	} else {\
	    actionSetMark $w actionSc1DZAOb;\
	}"

    actionBindCenter $w actionSc1DZAOb

 return;
}
# actionSc1DZOb


#
proc actionSc1DZAOb { w } {
    global ayprefs ayviewshortcuts

    viewTitle $w "" "Scale1DZA"
    viewSetMAIcon $w ay_Scale1DZA_img "Scale1DZA"

    actionClearB1 $w

    bind $w <ButtonPress-1> {
	set ay(action) 1
	undo save Sc1DZAObj
	%W mc
	%W sc1dzaoac -start %x %y
    }

    bind $w <B1-Motion> {
	%W sc1dzaoac -winxy %x %y
    }

    bind $w <Motion> ""

    actionBindRelease $w

    $w setconf -drawh 1

    if { [string first ".view" $w] == 0 } {
	set t [winfo toplevel $w]
    } else {
	set t [winfo parent [winfo parent $w]]
    }
    bind $t $ayviewshortcuts(About) "actionSetMark $w actionSc1DZAOb"

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
    }

    bind $w <B1-Motion> {
	%W sc2doac -winxy %x %y
    }

    bind $w <Motion> ""

    actionBindRelease $w

    $w setconf -drawh 1

    if { [string first ".view" $w] == 0 } {
	set t [winfo toplevel $w]
    } else {
	set t [winfo parent [winfo parent $w]]
    }

    bind $t $ayviewshortcuts(About) "\
	$w mc;\
	if { \$ay(cVDrawMark) } {\
	    actionSc2DAOb $w;\
	} else {\
	    actionSetMark $w actionSc2DAOb;\
	}"

    # allow restriction: x only
    bind $t $ayviewshortcuts(RestrictX) "actionSc1DXOb $w"

    # allow restriction: y only
    bind $t $ayviewshortcuts(RestrictY) "actionSc1DYOb $w"

    # allow restriction: z only
    bind $t $ayviewshortcuts(RestrictZ) "actionSc1DZOb $w"

    actionBindCenter $w actionSc2DAOb

 return;
}
# actionSc2DOb


#
proc actionSc2DAOb { w } {
    global ayprefs ayviewshortcuts

    viewTitle $w "" "Scale2D"
    viewSetMAIcon $w ay_Scale2DA_img "Scale2DA"

    actionClearB1 $w

    bind $w <ButtonPress-1> {
	set ay(action) 1
	undo save Sc2DAObj
	%W mc
	%W sc2daoac -start %x %y
    }

    bind $w <B1-Motion> {
	%W sc2daoac -winxy %x %y
    }

    bind $w <Motion> ""

    actionBindRelease $w

    $w setconf -drawh 1

    if { [string first ".view" $w] == 0 } {
	set t [winfo toplevel $w]
    } else {
	set t [winfo parent [winfo parent $w]]
    }
    bind $t $ayviewshortcuts(About) "actionSetMark $w actionSc2DAOb"

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
    }

    bind $w <B1-Motion> {
	%W sc3doac -winxy %x %y
    }

    bind $w <Motion> ""

    actionBindRelease $w

    $w setconf -drawh 1

    if { [string first ".view" $w] == 0 } {
	set t [winfo toplevel $w]
    } else {
	set t [winfo parent [winfo parent $w]]
    }

    bind $t $ayviewshortcuts(About) "\
	$w mc;\
	if { \$ay(cVDrawMark) } {\
	    actionSc3DAOb $w;\
	} else {\
	    actionSetMark $w actionSc3DAOb;\
	}"

    actionBindCenter $w actionSc3DAOb

 return;
}
# actionSc3DOb


#
proc actionSc3DAOb { w } {
    global ayprefs ayviewshortcuts

    viewTitle $w "" "Scale3DA"
    viewSetMAIcon $w ay_Scale3DA_img "Scale3DA"

    actionClearB1 $w

    bind $w <ButtonPress-1> {
	set ay(action) 1
	undo save Sc3DAObj
	%W mc
	%W sc3daoac -start %x %y
    }

    bind $w <B1-Motion> {
	%W sc3daoac -winxy %x %y
    }

    bind $w <Motion> ""

    actionBindRelease $w

    $w setconf -drawh 1

    if { [string first ".view" $w] == 0 } {
	set t [winfo toplevel $w]
    } else {
	set t [winfo parent [winfo parent $w]]
    }
    bind $t $ayviewshortcuts(About) "actionSetMark $w actionSc3DAOb"

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
    }

    bind $w <B1-Motion> {
	%W str2doac -winxy %x %y
    }

    bind $w <Motion> ""

    actionBindRelease $w

    $w setconf -drawh 1

    if { [string first ".view" $w] == 0 } {
	set t [winfo toplevel $w]
    } else {
	set t [winfo parent [winfo parent $w]]
    }

    bind $t $ayviewshortcuts(About) "\
	$w mc;\
	if { \$ay(cVDrawMark) } {\
	    actionStr2DAOb $w;\
	} else {\
	    actionSetMark $w actionStr2DAOb;\
	}"

    actionBindCenter $w actionStr2DAOb

 return;
}
# actionStr2DOb


#
proc actionStr2DAOb { w } {
    global ayprefs ayviewshortcuts

    viewTitle $w "" "Stretch2DA"
    viewSetMAIcon $w ay_Stretch2DA_img "Stretch2DA"

    actionClearB1 $w

    bind $w <ButtonPress-1> {
	set ay(action) 1
	undo save Str2DAObj
	%W mc
	%W str2daoac -start %x %y
    }

    bind $w <B1-Motion> {
	%W str2daoac -winxy %x %y
    }

    bind $w <Motion> ""

    actionBindRelease $w

    $w setconf -drawh 1

    if { [string first ".view" $w] == 0 } {
	set t [winfo toplevel $w]
    } else {
	set t [winfo parent [winfo parent $w]]
    }
    bind $t $ayviewshortcuts(About) "actionSetMark $w actionStr2DAOb"

 return;
}
# actionStr2DAOb


#
proc actionTagP { w } {
    global ayprefs ayviewshortcuts

    viewTitle $w "" "Select_Points"
    viewSetMAIcon $w ay_Tag_img "Select_Points"

    if { $ayprefs(TagResetTagged) == 1 } {
	selPnts; rV
    }

    actionClearB1 $w

    bind $w <ButtonPress-1> {
	# undo save TagP
	%W mc
	set oldx %x
	set oldy %y
    }

    bind $w <ButtonRelease-1> {
	if { ($oldx != %x) || ($oldy != %y) } {
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
	if { ($oldx != %x) || ($oldy != %y) } {
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
	    %W startpepac %x %y -readonly -flash
	}
	if { $ayprefs(FixFlashPoints) == 1 } {
	    bind $w <ButtonRelease-1> "+\
          %W startpepac %x %y -readonly -flash -ignoreold;\
          %W startpepac %x %y -readonly -flash -ignoreold"
	    bind $w <Shift-ButtonRelease-1> "+\
          %W startpepac %x %y -readonly -flash -ignoreold;\
          %W startpepac %x %y -readonly -flash -ignoreold"
	} else {
	    bind $w <ButtonRelease-1> "+\
          %W startpepac %x %y -readonly -flash -ignoreold;"
	    bind $w <Shift-ButtonRelease-1> "+\
          %W startpepac %x %y -readonly -flash -ignoreold;"
	}
    }
    $w setconf -drawh 1

 return;
}
# actionTagP


#
proc actionDelTagP { w } {
    # undo save DelTagP
    $w deselpac
    rV

 return;
}
# actionDelTagP


# in this global array the numeric edit points
# action keeps its data
array set editPntArr {
    x 0.0
    y 0.0
    z 0.0
    w 0.0

    lx 0.0
    ly 0.0
    lz 0.0
    lw 0.0

    wx 0.0
    wy 0.0
    wz 0.0
    ww 0.0

    window ""
    valid 0
    local 1
}
# array

proc editPointDialogClear { w } {
    foreach e {x y z w} {$w.f1.f$e.e delete 0 end}
}

proc editPointDialogSet { w arr } {
    global $arr
    editPointDialogClear $w
    foreach e {x y z w} {eval $w.f1.f$e.e insert 0 \$${arr}($e)}
}

#editPointDialogUpdate:
# helper for editPointDialog
# updates the dialog entries
proc editPointDialogUpdate { w } {
    upvar #0 editPntArr array

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

    update
    editPointDialogSet $w editPntArr
    update

 return;
}
# editPointDialogUpdate


#editPointDialogApply:
# helper for editPointDialog
# apply the changes from the dialog
proc editPointDialogApply { w } {
    global ay
    upvar #0 editPntArr array

    set array(x) [$w.f1.fx.e get]
    set array(y) [$w.f1.fy.e get]
    set array(z) [$w.f1.fz.e get]
    set array(w) [$w.f1.fw.e get]

    # if there are scripts or variable accesses in the
    # entry fields, execute / realize them ...
    if { [string match {*\[*} $array(x)] } {
	eval set array(x) $array(x)
    } else {
	if { [string match {*\$*} $array(x)] } {
	    eval [subst "set array(x) $array(x)"]
	}
    }

    if { [string match {*\[*} $array(y)] } {
	eval set array(y) $array(y)
    } else {
	if { [string match {*\$*} $array(y)] } {
	    eval [subst "set array(y) $array(y)"]
	}
    }

    if { [string match {*\[*} $array(z)] } {
	eval set array(z) $array(z)
    } else {
	if { [string match {*\$*} $array(z)] } {
	    eval [subst "set array(z) $array(z)"]
	}
    }

    if { [string match {*\[*} $array(w)] } {
	eval set array(w) $array(w)
    } else {
	if { [string match {*\$*} $array(w)] } {
	    eval [subst "set array(w) $array(w)"]
	}
    }

    # in floating window GUI mode our window
    # may be closed now, better check that...
    if { [winfo exists $array(window)] } {
	undo save EditPntNum
	set focusWin [focus]
	$array(window) penpac -apply
	rV
	plb_update
	after idle "focus $focusWin"
    } else {
	ayError 2 "editPointApply" "Lost window to apply changes to!"
    }

 return;
}
# editPointDialogApply


#editPointDialog:
# helper for actionEditNumP below
# open the numeric point edit dialog
proc editPointDialog { win } {
    upvar #0 editPntArr array
    global ay ayprefs aymainshortcuts tcl_platform AYWITHAQUA

    set w .editPointDw

    if { [winfo exists $w] } {
	editPointDialogUpdate $w
	return;
    }

    catch {destroy $w}
    set t "Edit Point"
    winDialog $w $t

    if { $ayprefs(FixDialogTitles) == 1 } {
	pack [frame $w.fl] -in $w -side top
	pack [label $w.fl.l -text $t] -in $w.fl -side left -fill x -expand yes
    }

    set f [frame $w.f1]
    pack $f -in $w -side top -fill x

    set f [frame $f.fm]
    menubutton $f.mb -text "Local" -menu $f.mb.m -relief raised\
	    -padx 0 -pady 1 -indicatoron 1
    if { $tcl_platform(platform) == "windows" } {
	$f.mb configure -pady 1
    }
    if { $AYWITHAQUA } {
	$f.mb configure -pady 2
    }
    set m [menu $f.mb.m -tearoff 0]
    $m add command -label "Local" -command {
	global editPntArr
	set editPntArr(local) 1
	.editPointDw.f1.fm.mb configure -text "Local"
	editPointDialogUpdate .editPointDw
    }
    $m add command -label "World" -command {
	global editPntArr
	set editPntArr(local) 0
	.editPointDw.f1.fm.mb configure -text "World"
	editPointDialogUpdate .editPointDw
    }
    pack $f.mb -in $f -side left -fill x -expand yes -pady 0
    pack $f -in $w.f1 -side top -fill x

    # separating space
    pack [label $w.f1.l0 -height 1 -image ay_Empty_img -pady 0]\
	-in $w.f1 -side top -fill x -pady 0

    # create four entries
    foreach e {X Y Z W} {
	set lce [string tolower $e]
	set f $w.f1
	set f [frame $f.f$lce]
	label $f.l -text $e -width 4
	entry $f.e -width 8
	pack $f.l -in $f -padx 2 -pady 2 -side left -fill x -expand no
	pack $f.e -in $f -padx 2 -pady 2 -side left -fill x -expand yes
	pack $f -in $w.f1 -side top -fill x

	bind $f.e <Key-Return> "editPointDialogApply $w;break"
	catch {bind $f.e <Key-KP_Enter> "editPointDialogApply $w;break"}
    }

    # separating space
    pack [label $w.f1.l2 -height 2 -image ay_Empty_img -pady 0]\
	-in $w.f1 -side top -fill x -pady 0

    update
    editPointDialogUpdate $w

    set f [frame $w.f2]
    button $f.bok -text "Apply" -width 5 -pady $ay(pady)\
	-command "editPointDialogApply $w"
    button $f.bca -text "Cancel" -width 5 -pady $ay(pady) -command "\
            global editPntArr;\
	    if { \[winfo exists \$editPntArr(window)\] } {\
	     focus \$editPntArr(window) } else { resetFocus };\
	    destroy $w"

    pack $f.bok $f.bca -in $f -side left -fill x -expand yes
    pack $f -in $w -side bottom -fill x

    # create popup
    set m [menu $w.popup -tearoff 0]
    $m add command -label "Clear" -command "editPointDialogClear $w"
    $m add command -label "Reset" -command "editPointDialogUpdate $w"
    $m add command -label "Fetch Mark" -command "editPointDialogSet $w aymark"

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

    bind $w <Key-Return> "editPointDialogApply $w;break"
    catch {bind $w <Key-KP_Enter> "editPointDialogApply $w;break"}

    focus $f.bok

    bind $w <ButtonPress-$aymainshortcuts(CMButton)> "winOpenPopup $w"

    if {  [string first ".view" $win] != 0 } {
	# internal view
	winRestoreOrCenter $w $t
    } else {
	# external view => center dialog on view
	wm transient $w [winfo toplevel $win]
	winCenter $w [winfo toplevel $win]
    }

 return;
}
# editPointDialog


#
proc actionEditNumP { w } {
    global ayprefs ayviewshortcuts

    viewTitle $w "" "Edit_Points_Num"
    viewSetMAIcon $w ay_EditN_img "Edit_Points_Num"

    actionClearB1 $w

    bind $w <ButtonPress-1> {
	%W mc
	set oldx %x
	set oldy %y
	set editPntArr(valid) 0
	%W startpepac %x %y -flash -ignoreold
	%W penpac -start %x %y
	set editPntArr(window) %W
	if { $editPntArr(valid) == 1 } {
	    editPointDialog %W
	}
	update
    }

    # next bindings taken from tag action (actionTagP)
    bind $w <ButtonRelease-1> {
	if { ($oldx != %x) || ($oldy != %y) } {
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
	if { ($oldx != %x) || ($oldy != %y) } {
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
#    bind $w <ButtonRelease-1> "focus %W"

    if { $ayprefs(FlashPoints) == 1 } {
	bind $w <Motion> {
	    %W startpepac %x %y -flash
	}
    }

    $w setconf -drawh 1

 return;
}
# actionEditNumP


#
proc actionEditP { w } {
    global ayprefs

    viewTitle $w "" "Edit_Points"
    viewSetMAIcon $w ay_Edit_img "Edit_Points"

    actionClearB1 $w

    bind $w <Motion> ""

    bind $w <ButtonPress-1> {
	set ay(action) 1
	undo save EditPnt
	%W mc
	if { $ayprefs(FlashPoints) == 1 } {
	    %W startpepac %x %y -flash
	    %W pepac -start %x %y -flash
	} else {
	    %W startpepac %x %y
	    %W pepac -start %x %y
	}
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
    global ayprefs

    viewTitle $w "" "Reset_Weights"
    viewSetMAIcon $w ay_ResetW_img "Reset_Weights"

    actionClearB1 $w

    bind $w <ButtonPress-1> {
	undo save ResetPntW
	if { $ayprefs(FlashPoints) == 1 } {
	    %W startpepac %x %y -flash
	} else {
	    %W startpepac %x %y
	}
    }

    if { $ayprefs(FlashPoints) == 1 } {
	bind $w <Motion> {
	    %W startpepac %x %y -flash
	}
    }

    bind $w <ButtonRelease-1> {
	%W wrpac -selected
	rV
	plb_update
	focus %W
    }

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
# actionResetWP


#
proc actionResetAllWP { w } {
    undo save ResetAllW
    $w wrpac
    plb_update
    rV
 return;
}
# actionResetAllWP


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
proc actionFindUV { w } {

    viewTitle $w "" "Find_UV"
    viewSetMAIcon $w ay_FindUV_img "Find_UV"

    actionClearB1 $w

    bind $w <ButtonPress-1> {
	%W mc
	%W finduvac %x %y
    }

    bind $w <B1-Motion> ""

    bind $w <Motion> ""

    bind $w <ButtonRelease-1> {
	%W finduvac -end %x %y
	%W redraw
	focus %W
    }

    $w setconf -drawh 0

 return;
}
# actionFindUV


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
    global ayprefs ayviewshortcuts

    viewTitle $w "" "Pick"
    viewSetMAIcon $w ay_Pick_img "Pick"

    bind $w <ButtonPress-1> {
	set oldx %x
	set oldy %y
    }

    bind $w <ButtonRelease-1> {
	%W setconf -rect $oldx $oldy %x %y 0

	if { [winfo exists .reconsider] == 0 } {
	    if { ($oldx == %x) || ($oldy == %y) } {
		%W processObjSel node %x %y
		singleObjSel $node
	    } else {
		%W processObjSel node $oldx $oldy %x %y
		multipleObjSel $node
	    }
	}

	focus %W
    }
    # bind

    bind $w <${ayviewshortcuts(PickMod)}-ButtonRelease-1> {
	%W setconf -rect $oldx $oldy %x %y 0

	if { [winfo exists .reconsider] == 0} {
	    if { ($oldx == %x) || ($oldy == %y) } {
		%W processObjSel node %x %y
		addObjSel $node
	    } else {
		%W processObjSel node $oldx $oldy %x %y
		addMultipleObjSel $node
	    }
	}

	focus %W
    }
    # bind

    bind $w <B1-Motion> {
	%W setconf -rect $oldx $oldy %x %y 1
    }

    if { $ayprefs(FlashObjects) } {
	bind $w <Motion> {
	    if { [winfo exists .reconsider] == 0} {
		%W processObjSel - %x %y
	    }
	}
	bind $w <Leave> {
	    %W processObjSel +
	}
    } else {
	bind $w <Motion> ""
    }

 return;
}
# actionPick


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


#
proc actionSnapToMark { w } {
    undo save SnapToMark
    $w snapmac 0
    rV
 return;
}
# actionSnapToMark


#
proc actionSnapToMarkO { w } {
    undo save SnapToMarkO
    $w snapmac 1
    rV
    plb_update
    after idle "focus $w"
 return;
}
# actionSnapToMarkO


#
proc actionIncMultP { w } {
    undo save IncMultP
    $w multpac 0
    rV
    plb_update
    after idle "focus $w"
 return;
}
# actionIncMultP


#
proc actionDecMultP { w } {
    undo save DecMultP
    $w multpac 1
    rV
    plb_update
    after idle "focus $w"
 return;
}
# actionDecMultP


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


#actionClearKbd:
# helper procedure to clear all left over keyboard bindings;
# all modeling actions call this before adding their bindings
proc actionClearKbd { w } {
    global ayviewshortcuts

    bind $w $ayviewshortcuts(CenterO) ""
    bind $w $ayviewshortcuts(CenterPC) ""
    bind $w $ayviewshortcuts(CenterPB) ""

    bind $w <Key-Return> ""

 return;
}
# actionClearKbd


#actionClear:
# not really an action, clears all bindings, establishes picking bindings
# when requested via preferences and is normally bound to the Esc-key
proc actionClear { w } {
    global ayprefs ayviewshortcuts

    actionClearB1 $w
    actionClearKbd $w

    if { [string first ".view" $w] != 0 } {
	set t [winfo parent [winfo parent $w]]
	bind $w <ButtonPress-1> "focus \$w"
    } else {
	set t [winfo toplevel $w]
	bind $w <ButtonPress-1> ""
    }

    bind $w <Motion> ""

    if { $ayprefs(DefaultAction) == 0 } {
	viewTitle $w "" "None"
	viewSetMAIcon $w ay_Empty_img ""
    } else {
	actionPick $w
    }

    # do not draw points in none/pick action
    $w setconf -drawh 0

    # the following after scripts arrange for a short period
    # 0.1 - 1s after the first press of the <Esc> key, that a second
    # press of the <Esc> key also resets the mark and the focus
    after 100 "bind $t <$ayviewshortcuts(Break)>\
               \"$t.f3D.togl setconf -drawh 0;\
                 $t.f3D.togl setconf -mark n;\
                 resetFocus\""
    # after 1s, the old binding is in effect
    after 1000 "bind $t <$ayviewshortcuts(Break)> \"actionClear $t.f3D.togl\""

    # re-establish standard set mark binding
    bind $t <$ayviewshortcuts(About)> "actionSetMark $t.f3D.togl"

 return;
}
# actionClear
