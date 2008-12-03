# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2005 by Randolf Schultz
# (randolf.schultz@gmail.com) and others.
#
# All rights reserved.
#
# See the file License for details.

# shortcut.tcl - procedures to establish key bindings


# repcont - replace control
proc repcont { string } {
    regsub -all "Ctrl" $string "Control" dummy
    return $dummy
}
# repcont


# repkp - replace "KeyPress-"
proc repkp { string } {
    regsub -all "KeyPress-" $string "" dummy
    return $dummy
}
# repkp

# shortcut_swapmb:
#  swap mouse button numbers from Win32/X11 style to Aqua style
#  in shortcuts designated by ayprefs(SwapMBSC)
proc shortcut_swapmb { } {
 global ay ayprefs

    if { ($ayprefs(SwapMB) == 1) && ($ayprefs(WindowSystem) != $ay(ws)) } {
	foreach shortcut $ayprefs(SwapMBSC) {
	    eval [subst "set val \$::$shortcut"]
	    set val [string map { 2 3 3 2 } $val]
	    eval [subst "set ::$shortcut $val"]
	}
    }

 return;
}
# shortcut_swapmb


# shortcut_fkeys:
#  Setup Function-Key-Bindings for window w
proc shortcut_fkeys { w } {
 global aymainshortcuts

    bind $w <[repcont $aymainshortcuts(SwLazyNotify)]>\
	    prefs_toggleLazyNotification
    bind $w <[repcont $aymainshortcuts(SwNURBS)]>\
	    {prefs_toggleSurfaceWire 1}
    bind $w <[repcont $aymainshortcuts(SwWire)]>\
	    {prefs_toggleSurfaceWire 0}
    bind $w <[repcont $aymainshortcuts(SetSTP)]>\
	    {prefs_setSamplingTolerance 1}
    bind $w <[repcont $aymainshortcuts(SetSTL)]>\
	    {prefs_setSamplingTolerance 0}

 return;
}
# shortcut_fkeys


# shortcut_main:
#  Setup Keybindings for the Main Window
proc shortcut_main { w } {
    global ay aymainshortcuts AYWITHAQUA tcl_platform

    set m $ay(filemenu)
    bind $w <[repcont $aymainshortcuts(New)]> "$m invoke 0;break"
    $m entryconfigure 0 -accelerator $aymainshortcuts(New)
    bind $w <[repcont $aymainshortcuts(Replace)]> "$m invoke 2;break"
    $m entryconfigure 2 -accelerator $aymainshortcuts(Replace)
    bind $w <[repcont $aymainshortcuts(Insert)]> "$m invoke 3;break"
    $m entryconfigure 3 -accelerator $aymainshortcuts(Insert)
    bind $w <[repcont $aymainshortcuts(SaveAs)]> "$m invoke 5;break"
    $m entryconfigure 5 -accelerator $aymainshortcuts(SaveAs)
    bind $w <[repcont $aymainshortcuts(Save)]> "$m invoke 6;break"
    $m entryconfigure 6 -accelerator $aymainshortcuts(Save)
    bind $w <[repcont $aymainshortcuts(ExportRIB)]> "$m.ex invoke 0;break"
    $m.ex entryconfigure 0 -accelerator $aymainshortcuts(ExportRIB)

    bind $w <[repcont $aymainshortcuts(MRU1)]> "$m invoke 16;break"
    $m entryconfigure 16\
	    -accelerator [repkp $aymainshortcuts(MRU1)]
    bind $w <[repcont $aymainshortcuts(MRU2)]> "$m invoke 17;break"
    $m entryconfigure 17\
	    -accelerator [repkp $aymainshortcuts(MRU2)]
    bind $w <[repcont $aymainshortcuts(MRU3)]> "$m invoke 18;break"
    $m entryconfigure 18\
	    -accelerator [repkp $aymainshortcuts(MRU3)]
    bind $w <[repcont $aymainshortcuts(MRU4)]> "$m invoke 19;break"
    $m entryconfigure 19\
	    -accelerator [repkp $aymainshortcuts(MRU4)]


    bind $w <[repcont $aymainshortcuts(Quit)]> "$m invoke 21;break"
    $m entryconfigure 21 -accelerator $aymainshortcuts(Quit)


    bind $w <[repcont $aymainshortcuts(Apply)]> "$ay(appb) invoke;break"

    set m $ay(editmenu)
    bind $w <[repcont $aymainshortcuts(Copy)]> "$m invoke 0;break"
    $m entryconfigure 0 -accelerator $aymainshortcuts(Copy)
    bind $w <[repcont $aymainshortcuts(Cut)]> "$m invoke 1;break"
    $m entryconfigure 1 -accelerator $aymainshortcuts(Cut)
    bind $w <[repcont $aymainshortcuts(Paste)]> "$m invoke 2;break"
    $m entryconfigure 2 -accelerator $aymainshortcuts(Paste)

    bind $w <[repcont $aymainshortcuts(SelAll)]> "$m invoke 5;break"
    $m entryconfigure 5 -accelerator $aymainshortcuts(SelAll)
    bind $w <[repcont $aymainshortcuts(SelNone)]> "$m invoke 6;break"
    $m entryconfigure 6 -accelerator $aymainshortcuts(SelNone)

    bind $w <[repcont $aymainshortcuts(CopyP)]> "$m invoke 8;break"
    $m entryconfigure 8 -accelerator $aymainshortcuts(CopyP)
    bind $w <[repcont $aymainshortcuts(ICopyP)]> "$m invoke 9;break"
    $m entryconfigure 9 -accelerator $aymainshortcuts(ICopyP)
    bind $w <[repcont $aymainshortcuts(PasteP)]> "$m invoke 10;break"
    $m entryconfigure 10 -accelerator $aymainshortcuts(PasteP)

    bind $w <[repcont $aymainshortcuts(Undo)]> "$m invoke 12;break"
    $m entryconfigure 12 -accelerator $aymainshortcuts(Undo)
    bind $w <[repcont $aymainshortcuts(Redo)]> "$m invoke 13;break"
    $m entryconfigure 13 -accelerator $aymainshortcuts(Redo)

    bind $w <[repcont $aymainshortcuts(Material)]> "$m invoke 15;break"
    $m entryconfigure 15 -accelerator $aymainshortcuts(Material)

    bind $w <[repcont $aymainshortcuts(Master)]> "$m invoke 16;break"
    $m entryconfigure 16 -accelerator $aymainshortcuts(Master)


    bind $w <[repcont $aymainshortcuts(Prefs)]> "$m invoke 18;break"
    $m entryconfigure 18 -accelerator $aymainshortcuts(Prefs)

    set m $ay(toolsmenu)
    bind $w <[repcont $aymainshortcuts(LastTool)]> "$m invoke 0;break"
    $m entryconfigure 0 -accelerator $aymainshortcuts(LastTool)


    set m $ay(helpmenu)
    bind $w <[repcont $aymainshortcuts(Help)]> "$m invoke 0;break"
    $m entryconfigure 0 -accelerator $aymainshortcuts(Help)

    set m $ay(specialmenu)
    bind $w <[repcont $aymainshortcuts(Zap)]> zap
    $m entryconfigure 20 -accelerator $aymainshortcuts(Zap)

    bind $w <Map> unzap

    bind $w <[repcont $aymainshortcuts(RebuildTree)]> {
	global ay
	if { $ay(lb) == 0 } { tree_reset }
    }

    # bind function keys
    shortcut_fkeys $w

    if { $w  == "." } {
	bind $w <Enter> {
	    global ayprefs
	    if { $ayprefs(AutoFocus) == 1 } {
		if { "%W" == "[winfo toplevel %W]" } {
		    focus %W
		}
	    }
	}
    }

    bind $w <Left> {
	global ay
	if { $ay(lb) == 0 } {
	    set tree $ay(tree)
	    set cl $ay(CurrentLevel)

	    if { $cl == "root" } {
		break
	    }
	    set i [string last ":" $cl]
	    set newcl [string range $cl 0 [expr ${i}-1]]
	    tree_paintLevel $newcl
	    goUp
	    set ay(CurrentLevel) $newcl
	    set ay(SelectedLevel) $newcl
	    $tree selection set $cl
	    treeSelect $cl
	    tree_paintLevel $newcl
	    plb_update
	    rV

	} else {
	    $ay(olbbup) invoke
	    break
	}
	# if
    }
    # bind

    bind $w <Right> {
	global ay
	if { $ay(lb) == 0 } {
	    set tree $ay(tree)
	    set cl $ay(CurrentLevel)

	    set sel ""
	    set sel [$tree selection get]

	    if { $sel == "" } {
		break
	    }

	    set sel [lindex $sel 0]

	    $tree selection set $sel
	    treeSelect $sel

	    if { ! [hasChild] } {
		break
	    }
	    $tree itemconfigure $sel -open 1
	    goDown
	    set ay(CurrentLevel) $sel
	    set ay(SelectedLevel) $sel
	    tree_paintLevel $cl
	    $tree selection set ${sel}:0
	    treeSelect ${sel}:0
	    tree_paintLevel $sel
	    plb_update
	    rV

	} else {
	    $ay(olbbdwn) invoke
	    break
	}
	# if
    }
    # bind

    bind $w <Down> "selNPFL 0"
    bind $w <Shift-Down> "selAdd 1"
    bind $w <Shift-End> "selAdd 4"
    bind $w <Up> "selNPFL 1"
    bind $w <Shift-Up> "selAdd 0"
    bind $w <Shift-Home> "selAdd 3"
    bind $w <Home> "selNPFL 2"
    bind $w <End> "selNPFL 3"

    bind $w <space> {
	global ay
	if { $ay(lb) == 0 } {
	    set tree $ay(tree)
	    set sel ""
	    set sel [$tree selection get]
	    foreach node $sel {
		if { ! [$tree itemcget $node -open] } {
		    $tree itemconfigure $node -open 1
		} else {
		    $tree itemconfigure $node -open 0
		}
	    }
	    # foreach
	}
	# if
    }
    # bind

    bind $w <[repcont $aymainshortcuts(SProp0)]> "plb_showprop 0"
    bind $w <[repcont $aymainshortcuts(SProp1)]> "plb_showprop 1"
    bind $w <[repcont $aymainshortcuts(SProp2)]> "plb_showprop 2"
    bind $w <[repcont $aymainshortcuts(SProp3)]> "plb_showprop 3"
    bind $w <[repcont $aymainshortcuts(SProp4)]> "plb_showprop 4"
    bind $w <[repcont $aymainshortcuts(SProp5)]> "plb_showprop 5"
    bind $w <[repcont $aymainshortcuts(SProp6)]> "plb_showprop 6"
    bind $w <[repcont $aymainshortcuts(SProp7)]> "plb_showprop 7"
    bind $w <[repcont $aymainshortcuts(SProp8)]> "plb_showprop 8"
    bind $w <[repcont $aymainshortcuts(SProp9)]> "plb_showprop 9"

    bind $w <[repcont $aymainshortcuts(SProp00)]> "plb_showprop 0"
    bind $w <[repcont $aymainshortcuts(SProp11)]> "plb_showprop 1"
    bind $w <[repcont $aymainshortcuts(SProp22)]> "plb_showprop 2"
    bind $w <[repcont $aymainshortcuts(SProp33)]> "plb_showprop 3"
    bind $w <[repcont $aymainshortcuts(SProp44)]> "plb_showprop 4"
    bind $w <[repcont $aymainshortcuts(SProp55)]> "plb_showprop 5"
    bind $w <[repcont $aymainshortcuts(SProp66)]> "plb_showprop 6"
    bind $w <[repcont $aymainshortcuts(SProp77)]> "plb_showprop 7"
    bind $w <[repcont $aymainshortcuts(SProp88)]> "plb_showprop 8"
    bind $w <[repcont $aymainshortcuts(SProp99)]> "plb_showprop 9"

    bind $w <[repcont $aymainshortcuts(ContextMenu)]> \
	"if { $ay(lb) == 1 } {olb_openPopup \$ay(olb)} else {tree_openPopup \$ay(tree)}"

    if { $AYWITHAQUA == 1 } {
	bind $w <Command-q> exit
    }

    if { $tcl_platform(platform) == "windows" } {
	set m $ay(filemenu)
	bind $w <Alt-F4> "$m invoke 21;break"
    }

 return;
}
# shortcut_main


# shortcut_toolbox:
# Setup special Keybindings for the toolbox
# XXXX currently unused
#
proc shortcut_toolbox { w } {
    global ay ayviewshortcuts aymainshortcuts

    set m $ay(editmenu)
    bind $w <[repcont $aymainshortcuts(Undo)]> "$m invoke 9"
    bind $w <[repcont $aymainshortcuts(Redo)]> "$m invoke 10"
}
# shortcut_toolbox


# shortcut_view:
# Setup menu- and main-keybindings for a 3D-View.
proc shortcut_view { w } {
    global ay ayprefs ayviewshortcuts aymainshortcuts AYWITHAQUA

    if { (! $AYWITHAQUA) || ([winfo toplevel $w] != $w) } {
	set viewm $w.fMenu.v.m
	set typem $w.fMenu.t.m
	set confm $w.fMenu.c.m
    } else {
	set viewm $w.menubar.mview
	set typem $w.menubar.mtype
	set confm $w.menubar.mconf
    }

    if { [winfo toplevel $w] != $w } {
	eval [subst "bindtags $w \{$w Frame all\}"]
	shortcut_main $w
    } else {
	# some main window shortcuts
	set m $ay(helpmenu)
	bind $w <[repcont $aymainshortcuts(Help)]> "$m invoke 0;break"

	set m $ay(editmenu)
	bind $w <[repcont $aymainshortcuts(Undo)]> "$m invoke 12;break"
	bind $w <[repcont $aymainshortcuts(Redo)]> "$m invoke 13;break"
	bind $w <[repcont $aymainshortcuts(Material)]> "$m invoke 15;break"
	bind $w <[repcont $aymainshortcuts(Master)]> "$m invoke 16;break"
    }

    # view window shortcuts
    set m $viewm
    bind $w <[repcont $ayviewshortcuts(QRender)]> "$m invoke 0;break"
    $m entryconfigure 0 -accelerator $ayviewshortcuts(QRender)
    bind $w <[repcont $ayviewshortcuts(Render)]> "$m invoke 1;break"
    $m entryconfigure 1 -accelerator $ayviewshortcuts(Render)
    bind $w <[repcont $ayviewshortcuts(Redraw)]> "$m invoke 3;break"
    $m entryconfigure 3 -accelerator $ayviewshortcuts(Redraw)

    bind $w <[repcont $aymainshortcuts(ExportRIB)]> "$m invoke 4;break"
    $m entryconfigure 4 -accelerator $aymainshortcuts(ExportRIB)
    if { [string first ".view" $w] == 0 } {
	global AYENABLEPPREV
	if { $AYENABLEPPREV == 1 } { set tmp 12 } else { set tmp 9 } 
	bind $w <[repcont $ayviewshortcuts(Close)]> "$m invoke $tmp"
	$m entryconfigure $tmp -accelerator $ayviewshortcuts(Close)
    }

    if { ([winfo toplevel $w] == $w) || $ayprefs(BindInternalViews) } {
	set m $typem
	bind $w <[repcont $ayviewshortcuts(Front)]> "$m invoke 0;break"
	$m entryconfigure 0 -accelerator $ayviewshortcuts(Front)
	bind $w <[repcont $ayviewshortcuts(Side)]> "$m invoke 1;break"
	$m entryconfigure 1 -accelerator $ayviewshortcuts(Side)
	bind $w <[repcont $ayviewshortcuts(Top)]> "$m invoke 2;break"
	$m entryconfigure 2 -accelerator $ayviewshortcuts(Top)
	bind $w <[repcont $ayviewshortcuts(Persp)]> "$m invoke 4;break"
	$m entryconfigure 4 -accelerator $ayviewshortcuts(Persp)
	bind $w <[repcont $ayviewshortcuts(Trim)]> "$m invoke 6;break"
	$m entryconfigure 6 -accelerator $ayviewshortcuts(Trim)
    }

    set m $confm
    bind $w <[repcont $ayviewshortcuts(Auto)]> "$m invoke 0;break"
    $m entryconfigure 0 -accelerator $ayviewshortcuts(Auto)
    
    #bind $w <[repcont $ayviewshortcuts(Shade)]> "$m invoke 1"
    #$m entryconfigure 1 -accelerator $ayviewshortcuts(Shade)
    bind $w <[repcont $ayviewshortcuts(Shade)]> "viewToggleDMode $w;break"

    bind $w <[repcont $ayviewshortcuts(DGrid)]> "$m invoke 10;break"
    $m entryconfigure 10 -accelerator $ayviewshortcuts(DGrid)
    bind $w <[repcont $ayviewshortcuts(UGrid)]> "$m invoke 11;break"
    $m entryconfigure 11 -accelerator $ayviewshortcuts(UGrid)
    bind $w <[repcont $ayviewshortcuts(SGrid)]> "$m invoke 12;break"
    $m entryconfigure 12 -accelerator $ayviewshortcuts(SGrid)
    if { ([winfo toplevel $w] == $w) } {
	bind $w <[repcont $ayviewshortcuts(Halve)]> "$m invoke 14;break"
	$m entryconfigure 14 -accelerator $ayviewshortcuts(Halve)
	bind $w <[repcont $ayviewshortcuts(Double)]> "$m invoke 15;break"
	$m entryconfigure 15 -accelerator $ayviewshortcuts(Double)
    }
    if { ([winfo toplevel $w] == $w) || $ayprefs(BindInternalViews) } {
	$m entryconfigure 21 -accelerator $ayviewshortcuts(ZoomTO)
	bind $w <[repcont $ayviewshortcuts(ZoomTO)]> "$m invoke 21;break"

	$m entryconfigure 22 -accelerator $ayviewshortcuts(Align)
	bind $w <[repcont $ayviewshortcuts(Align)]> "$m invoke 22;break"
    } else {
	$m entryconfigure 21 -accelerator $ayviewshortcuts(ZoomTO2)
	$m entryconfigure 22 -accelerator $ayviewshortcuts(Align2)
	$m entryconfigure 23 -accelerator $ayviewshortcuts(Local2)
    }
    
    bind $w <[repcont $ayviewshortcuts(ZoomTO2)]> "$m invoke 21;break"
    bind $w <[repcont $ayviewshortcuts(Align2)]> "$m invoke 22;break"

    #set m $ay(mmodem)
    #$m entryconfigure 1 -accelerator $ayviewshortcuts(Local)
    #bind $w <[repcont $ayviewshortcuts(Local)]> "$m invoke 1;break"
    if { ([winfo toplevel $w] == $w) || $ayprefs(BindInternalViews) } {
	bind $w <[repcont $ayviewshortcuts(Local)]> "viewToggleMMode $w;break"
	$m entryconfigure 23 -accelerator $ayviewshortcuts(Local)
    }

    bind $w <[repcont $ayviewshortcuts(Local2)]> "viewToggleMMode $w;break"

    bind $w <[repcont $ayviewshortcuts(RotL)]>\
	    "$w.f3D.togl setconf -undrotx 1 -drotx 5.0 -droty 0.0;break"
    bind $w <[repcont $ayviewshortcuts(RotR)]>\
	    "$w.f3D.togl setconf -undrotx 1 -drotx -5.0 -droty 0.0;break"
    bind $w <[repcont $ayviewshortcuts(RotU)]>\
	    "$w.f3D.togl setconf -undroty 1 -droty 5.0 -drotx 0.0;break"
    bind $w <[repcont $ayviewshortcuts(RotD)]>\
	    "$w.f3D.togl setconf -undroty 1 -droty -5.0 -drotx 0.0;break"

    bind $w <[repcont $ayviewshortcuts(ZoomI)]>\
	    "$w.f3D.togl setconf -dzoom 0.9;break"

    bind $w <[repcont $ayviewshortcuts(ZoomO)]>\
	    "$w.f3D.togl setconf -dzoom 1.1;break"

    bind $w <[repcont $ayviewshortcuts(SelAllP)]>\
	    "selPoints; rV;break"
    bind $w <[repcont $ayviewshortcuts(InvSelP)]>\
	    "invSelPoints; rV;break"

    bind $w <[repcont $ayviewshortcuts(CollP)]>\
	    "collMP; rV;break"
    bind $w <[repcont $ayviewshortcuts(ExplP)]>\
	    "explMP; rV;break"

    bind $w <[repcont $ayviewshortcuts(SnapGrid2D)]>\
	    "actionSnapToGrid2D $w.f3D.togl;break"
    bind $w <[repcont $ayviewshortcuts(SnapGrid3D)]>\
	    "actionSnapToGrid3D $w.f3D.togl;break"

    set m $ay(toolsmenu)
    bind $w <[repcont $ayviewshortcuts(Hide)]>\
	    "$m invoke 12;break"
    bind $w <[repcont $ayviewshortcuts(Show)]>\
	    "$m invoke 14;break"

    bind $w <[repcont $ayviewshortcuts(TypeUp)]> "viewCycleType $w -1;break"
    bind $w <[repcont $ayviewshortcuts(TypeDown)]> "viewCycleType $w 1;break"

    bind $w <[repcont $aymainshortcuts(Zap)]> zap
    bind $w <Map> unzap

    # bind function keys
    shortcut_fkeys $w

    if { $AYWITHAQUA == 1 } {
	bind $w <Command-q> exit
    }

 return;
}
# shortcut_view


# shortcut_viewactions:
# Setup action-keybindings for a 3D-View.
proc shortcut_viewactions { w } {
 global ay ayviewshortcuts

    set i $ayviewshortcuts(RotButton)

    bind $w <KeyPress-$ayviewshortcuts(RotModKey)> {
	if { [string first ".view" %W] != 0 } {
	    set w %W.f3D.togl
	} else {
	    set w [winfo toplevel %W].f3D.togl
	}
	$w configure -cursor exchange
    }

    bind $w <KeyRelease-$ayviewshortcuts(RotModKey)> {
	if { [string first ".view" %W] != 0 } {
	    set w %W.f3D.togl
	} else {
	    set w [winfo toplevel %W].f3D.togl
	}
	$w configure -cursor {}
    }
    bind $w.f3D.togl <${ayviewshortcuts(RotMod)}-ButtonPress-${i}> {
	set oldx %x
	set oldy %y
    }
    bind $w.f3D.togl <${ayviewshortcuts(RotMod)}-ButtonRelease-${i}> "break"
    bind $w.f3D.togl <${ayviewshortcuts(RotMod)}-B${i}-Motion> {
	%W setconf -drotx [expr ($oldx - %x)] -droty [expr ($oldy - %y)]
	set oldx %x
	set oldy %y
	update
	break
    }

    set i $ayviewshortcuts(ZoomRButton)

    bind $w <KeyPress-$ayviewshortcuts(ZoomRModKey)> {
	if { [string first ".view" %W] != 0 } {
	    set w %W.f3D.togl
	} else {
	    set w [winfo toplevel %W].f3D.togl
	}
	$w configure -cursor sizing
    }

    bind $w <KeyRelease-$ayviewshortcuts(ZoomRModKey)> {
	if { [string first ".view" %W] != 0 } {
	    set w %W.f3D.togl
	} else {
	    set w [winfo toplevel %W].f3D.togl
	}
	$w configure -cursor {}
    }

    bind $w.f3D.togl <${ayviewshortcuts(ZoomRMod)}-ButtonPress-${i}> {
	set oldx %x
	set oldy %y
    }

    bind $w.f3D.togl <${ayviewshortcuts(ZoomRMod)}-ButtonRelease-${i}> {
	undo save ZoomRView
	%W setconf -zrect 1 -rect $oldx $oldy %x %y 0
	%W redraw
	break
    }

    bind $w.f3D.togl <${ayviewshortcuts(ZoomRMod)}-B${i}-Motion> {
	%W setconf -rect $oldx $oldy %x %y 1
    }

    set i $ayviewshortcuts(ZoomVButton)
    
    bind $w.f3D.togl <ButtonPress-${i}> {
	undo save ZoomView
	%W mc
	%W zoomvac -start %y
	update
    }

    bind $w.f3D.togl <B${i}-Motion> {
	%W zoomvac -winy %y
	update
    }

    set i  $ayviewshortcuts(MoveVButton)

    bind $w.f3D.togl <ButtonPress-${i}> {
	undo save MoveView
	%W mc
	%W movevac -start %x %y
	update
    }

    bind $w.f3D.togl <B${i}-Motion> {
	%W movevac -winxy %x %y
	update
    }
    
    bind $w.f3D.togl <ButtonPress-4> {
	undo save ZoomView
	%W mc
	%W setconf -dzoom $::ayprefs(WheelZoom)
	update
	%W reshape
	%W render
	break
    }

    bind $w.f3D.togl <ButtonPress-5> {
	undo save ZoomView
	%W mc
	%W setconf -dzoom [expr 1.0/$::ayprefs(WheelZoom)]
	update
	%W reshape
	%W render
	break
    }

    if { $ay(ws) == "Win32" } {
	bind $w <MouseWheel> {
	    undo save ZoomView
	    %W.f3D.togl mc
	    if { %D < 0.0 } {
		%W.f3D.togl setconf -dzoom [expr 1.0/$::ayprefs(WheelZoom)]
	    } else {
		%W.f3D.togl setconf -dzoom $::ayprefs(WheelZoom)
	    }
	    update
	    %W.f3D.togl reshape
	    %W.f3D.togl render
	    break
	}
    }
    # if

    if { $ay(ws) == "Aqua" } {
	bind $w.f3D.togl <MouseWheel> {
	    undo save ZoomView
	    %W mc
	    if { %D < 0.0 } {
		%W setconf -dzoom [expr 1.0/$::ayprefs(WheelZoom)]
	    } else {
		%W setconf -dzoom $::ayprefs(WheelZoom)
	    }
	    update
	    %W reshape
	    %W render
	    break
	}
    }
    # if

    bind $w <$ayviewshortcuts(Break)> "actionClear $w.f3D.togl"
    bind $w <$ayviewshortcuts(MoveV)> "actionMoveView $w.f3D.togl"
    bind $w <$ayviewshortcuts(MoveZV)> "actionMoveZView $w.f3D.togl"
    bind $w <$ayviewshortcuts(RotV)> "actionRotView $w.f3D.togl"
    bind $w <$ayviewshortcuts(ZoomV)> "actionZoomView $w.f3D.togl"
    
    bind $w <$ayviewshortcuts(MoveO)> "actionMoveOb $w.f3D.togl"
    bind $w <$ayviewshortcuts(RotO)> "actionRotOb $w.f3D.togl"
    bind $w <$ayviewshortcuts(RotA)> "actionRotObA $w.f3D.togl"
    bind $w <$ayviewshortcuts(ScalX)> "actionSc1DXOb $w.f3D.togl"
    bind $w <$ayviewshortcuts(ScalY)> "actionSc1DYOb $w.f3D.togl"
    bind $w <$ayviewshortcuts(ScalZ)> "actionSc1DZOb $w.f3D.togl"
    bind $w <$ayviewshortcuts(Scal2)> "actionSc2DOb $w.f3D.togl"
    bind $w <$ayviewshortcuts(Scal3)> "actionSc3DOb $w.f3D.togl"
    bind $w <$ayviewshortcuts(Stretch)> "actionStr2DOb $w.f3D.togl"
    bind $w <$ayviewshortcuts(Edit)> "actionEditP $w.f3D.togl"
    bind $w <$ayviewshortcuts(EditD)> "actionDEditP $w.f3D.togl"
    bind $w <$ayviewshortcuts(WeightE)> "actionEditWP $w.f3D.togl"
    bind $w <$ayviewshortcuts(WeightR)> "actionResetWP $w.f3D.togl"

    bind $w <$ayviewshortcuts(Select)> "actionTagP $w.f3D.togl"
    bind $w <$ayviewshortcuts(DeSelect)> "actionDelTagP $w.f3D.togl"
    bind $w <$ayviewshortcuts(InsertP)> "actionInsertP $w.f3D.togl"
    bind $w <$ayviewshortcuts(DeleteP)> "actionDeleteP $w.f3D.togl"
    bind $w <$ayviewshortcuts(FindU)> "actionFindU $w.f3D.togl"
    bind $w <$ayviewshortcuts(SplitNC)> "actionSplitNC $w.f3D.togl"
    bind $w <$ayviewshortcuts(Pick)> "actionPick $w.f3D.togl"

 return;
}
# shortcut_viewactions


#shortcut_show:
# display all current shortcuts in a separate top level window
proc shortcut_show { } {
global ay aymainshortcuts ayviewshortcuts tcl_platform

set w .ayscw

catch {destroy $w}
toplevel $w -class ayam
wm title $w "Ayam Shortcuts"
wm iconname $w "Ayam"
if { $ay(ws) == "Aqua" } {
    winMakeFloat $w
} else {
    wm transient $w .
}
frame $w.ftext

frame $w.fbutton
pack $w.fbutton -in $w -side bottom
button $w.fbutton.b -text "Dismiss" -pady $ay(pady) -command "destroy $w"
pack $w.fbutton.b -in $w.fbutton

# Esc-key && close via window decoration == Dismiss button
bind $w <Escape> "$w.fbutton.b invoke"
wm protocol $w WM_DELETE_WINDOW "$w.fbutton.b invoke"

pack $w.ftext -in $w -side top -expand yes -fill both

set font fixed

if { $tcl_platform(platform) == "windows" } {
    set font fixedsys
}

# IRIXens "fixed" is too wide...
if { [string first "IRIX" $tcl_platform(os)] != -1 } {
    set font 9x15
}

# Also MacOSX Aqua does not deliver a proper mono-spaced "fixed"...
if { $ay(ws) == "Aqua"  } {
    set font {Courier 12}
}

text $w.ftext.text -yscrollcommand "$w.ftext.sbar set" \
	-setgrid 1 -font $font -height 20 -width 40
# -tabs {32c left}
scrollbar $w.ftext.sbar -takefocus 0 -command "$w.ftext.text yview"

pack $w.ftext.sbar -in $w.ftext -side right -fill y
pack $w.ftext.text -in $w.ftext -side left -fill both -expand yes

$w.ftext.text insert end "Modelling Actions (View Windows):

 Pick Objects:        $ayviewshortcuts(Pick)

 Move Objects:        $ayviewshortcuts(MoveO)
 Rotate Objects:      $ayviewshortcuts(RotO)
 Rotate Objects   
 around Point:        $ayviewshortcuts(RotA)
 Scale Objects (3D)   $ayviewshortcuts(Scal3)
 Scale Objects (2D)   $ayviewshortcuts(Scal2)
 Stretch Objects (2D) $ayviewshortcuts(Stretch)
 Scale Objects (1D,X) $ayviewshortcuts(ScalX)
 Scale Objects (1D,Y) $ayviewshortcuts(ScalY)
 Scale Objects (1D,Z) $ayviewshortcuts(ScalZ)

 Edit Points          $ayviewshortcuts(Edit)
 Edit Points (direct) $ayviewshortcuts(EditD)
 Edit Weight          $ayviewshortcuts(WeightE)
 Reset Weights        $ayviewshortcuts(WeightR)

 Select Points        $ayviewshortcuts(Select)
 De-Select Points     $ayviewshortcuts(DeSelect)
 Insert Points        $ayviewshortcuts(InsertP)
 Delete Points        $ayviewshortcuts(DeleteP)

 Select All Points    $ayviewshortcuts(SelAllP)
 Invert PointSel      $ayviewshortcuts(InvSelP)
 Collapse Selected    $ayviewshortcuts(CollP)
 Explode Selected     $ayviewshortcuts(ExplP)
 Snap to Grid 2D      $ayviewshortcuts(SnapGrid2D)
 Snap to Grid 3D      $ayviewshortcuts(SnapGrid3D)

 Hide Objects         $ayviewshortcuts(Hide)
 Show Objects         $ayviewshortcuts(Show)

 FindU                $ayviewshortcuts(FindU)
 Split Curve          $ayviewshortcuts(SplitNC)

View Actions (View Windows):

 Break Action        $ayviewshortcuts(Break)

 Move View           $ayviewshortcuts(MoveV)
 Move View
 (along Z)           $ayviewshortcuts(MoveZV)
 MoveViewAnytime     Mouse-$ayviewshortcuts(MoveVButton)

 Rotate View         $ayviewshortcuts(RotV)
 RotateViewAnytime   $ayviewshortcuts(RotModKey)+Mouse-1

 Rotate View (left)  $ayviewshortcuts(RotL)
 Rotate View (right) $ayviewshortcuts(RotR)
 Rotate View (up)    $ayviewshortcuts(RotU)
 Rotate View (down)  $ayviewshortcuts(RotD)

 Zoom View           $ayviewshortcuts(ZoomV)
 Zoom in View        $ayviewshortcuts(ZoomI)
 Zoom out View       $ayviewshortcuts(ZoomO)
 ZoomViewAnytime     Mouse-$ayviewshortcuts(ZoomVButton)

 ZoomRegion View     $ayviewshortcuts(ZoomRModKey)+Mouse-1

 ZoomToObject        $ayviewshortcuts(ZoomTO) / $ayviewshortcuts(ZoomTO2)

 Cycle View Type     $ayviewshortcuts(TypeUp) / $ayviewshortcuts(TypeDown)

Function Keys (View Windows):

 Adjust NURBS Sampling Rate: ${aymainshortcuts(SetSTL)} / ${aymainshortcuts(SetSTP)}
 Toggle Wire/NURBS:          ${aymainshortcuts(SwWire)} / ${aymainshortcuts(SwNURBS)}
 Toggle Lazy Notification    ${aymainshortcuts(SwLazyNotify)} 


See the reference card in the docs
folder for a printable and more
complete version.
"

# disabling the widget on Win32 prohibits selection/focus
# and thus the mouse wheel does not work, the downside is
# that now, the user is able to edit the text (on Win32)
if { $tcl_platform(platform) != "windows" } {
    $w.ftext.text configure -state disabled
}

bind $w <Next> "$w.ftext.text yview scroll 1 pages"
bind $w <Prior> "$w.ftext.text yview scroll -1 pages"

bind $w.ftext.text <ButtonPress-4>\
"$w.ftext.text yview scroll -1 pages; break"

bind $w.ftext.text <ButtonPress-5>\
"$w.ftext.text yview scroll 1 pages; break"

 return;
}
# shortcut_show


# shortcut_addescescbinding:
#
proc shortcut_addescescbinding { w } {
    bind $w <Escape> "\
        bind $w <Escape> \"\";
        resetFocus;
	cS;
        \$ay(plb) selection clear 0 end;
        plb_update;
        if \{ \$ay(lb) == 0 \} \{ tree_gotop \} else \{ goTop \};
        rV;"
    after 1000 "bind $w <Escape> \{shortcut_addescescbinding $w\}"

 return;
}
# shortcut_addescescbinding
