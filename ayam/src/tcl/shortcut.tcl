# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2001 by Randolf Schultz
# (rschultz@informatik.uni-rostock.de) and others.
#
# All rights reserved.
#
# See the file License for details.

# shortcut.tcl - procedures to establish key bindings


# replace control
proc repcont { string } {
    regsub -all "Ctrl" $string "Control" dummy
    return $dummy
}

#replace "KeyPress-"
proc repkp { string } {
    regsub -all "KeyPress-" $string "" dummy
    return $dummy
}

# shortcut_main:
#  Setup Keybindings for the Main Window
# 
#
proc shortcut_main { w } {
    global aymainshortcuts

    if { [winfo exists .fl.con] == 1 } {
	bind .fl.con.console <[repcont $aymainshortcuts(SwCon)]> {
	    focus [tk_focusNext %W] }

	bind .fl.con.console <ButtonPress-4> {
	.fl.con.console yview scroll -1 pages
	break
	}
	bind .fl.con.console <ButtonPress-5> {
	.fl.con.console yview scroll 1 pages
        break
	}

    }


    bind $w <[repcont $aymainshortcuts(New)]> ".fu.fMenu.fil.m invoke 0"
    .fu.fMenu.fil.m entryconfigure 0 -accelerator $aymainshortcuts(New)
    bind $w <[repcont $aymainshortcuts(Replace)]> ".fu.fMenu.fil.m invoke 2"
    .fu.fMenu.fil.m entryconfigure 2 -accelerator $aymainshortcuts(Replace)
    bind $w <[repcont $aymainshortcuts(Insert)]> ".fu.fMenu.fil.m invoke 3"
    .fu.fMenu.fil.m entryconfigure 3 -accelerator $aymainshortcuts(Insert)
    bind $w <[repcont $aymainshortcuts(SaveAs)]> ".fu.fMenu.fil.m invoke 5"
    .fu.fMenu.fil.m entryconfigure 5 -accelerator $aymainshortcuts(SaveAs)
    bind $w <[repcont $aymainshortcuts(Save)]> ".fu.fMenu.fil.m invoke 6"
    .fu.fMenu.fil.m entryconfigure 6 -accelerator $aymainshortcuts(Save)
    bind $w <[repcont $aymainshortcuts(ExportRIB)]> ".fu.fMenu.fil.m invoke 10"
    .fu.fMenu.fil.m entryconfigure 10 -accelerator $aymainshortcuts(ExportRIB)

    bind $w <[repcont $aymainshortcuts(MRU1)]> ".fu.fMenu.fil.m invoke 16"
    .fu.fMenu.fil.m entryconfigure 16\
	    -accelerator [repkp $aymainshortcuts(MRU1)]
    bind $w <[repcont $aymainshortcuts(MRU2)]> ".fu.fMenu.fil.m invoke 17"
    .fu.fMenu.fil.m entryconfigure 17\
	    -accelerator [repkp $aymainshortcuts(MRU2)]
    bind $w <[repcont $aymainshortcuts(MRU3)]> ".fu.fMenu.fil.m invoke 18"
    .fu.fMenu.fil.m entryconfigure 18\
	    -accelerator [repkp $aymainshortcuts(MRU3)]
    bind $w <[repcont $aymainshortcuts(MRU4)]> ".fu.fMenu.fil.m invoke 19"
    .fu.fMenu.fil.m entryconfigure 19\
	    -accelerator [repkp $aymainshortcuts(MRU4)]


    bind $w <[repcont $aymainshortcuts(Quit)]> ".fu.fMenu.fil.m invoke 21"
    .fu.fMenu.fil.m entryconfigure 21 -accelerator $aymainshortcuts(Quit)

    bind $w <[repcont $aymainshortcuts(Apply)]> ".fu.fMain.fProp.fArg.fb.b1 invoke"

    bind $w <[repcont $aymainshortcuts(Copy)]> ".fu.fMenu.ed.m invoke 0"
    .fu.fMenu.ed.m entryconfigure 0 -accelerator $aymainshortcuts(Copy)
    bind $w <[repcont $aymainshortcuts(Cut)]> ".fu.fMenu.ed.m invoke 1"
    .fu.fMenu.ed.m entryconfigure 1 -accelerator $aymainshortcuts(Cut)
    bind $w <[repcont $aymainshortcuts(Paste)]> ".fu.fMenu.ed.m invoke 2"
    .fu.fMenu.ed.m entryconfigure 2 -accelerator $aymainshortcuts(Paste)
    bind $w <[repcont $aymainshortcuts(CopyP)]> ".fu.fMenu.ed.m invoke 5"
    .fu.fMenu.ed.m entryconfigure 5 -accelerator $aymainshortcuts(CopyP)
    bind $w <[repcont $aymainshortcuts(ICopyP)]> ".fu.fMenu.ed.m invoke 6"
    .fu.fMenu.ed.m entryconfigure 6 -accelerator $aymainshortcuts(ICopyP)
    bind $w <[repcont $aymainshortcuts(PasteP)]> ".fu.fMenu.ed.m invoke 7"
    .fu.fMenu.ed.m entryconfigure 7 -accelerator $aymainshortcuts(PasteP)

    bind $w <[repcont $aymainshortcuts(Undo)]> ".fu.fMenu.ed.m invoke 9"
    .fu.fMenu.ed.m entryconfigure 9 -accelerator $aymainshortcuts(Undo)
    bind $w <[repcont $aymainshortcuts(Redo)]> ".fu.fMenu.ed.m invoke 10"
    .fu.fMenu.ed.m entryconfigure 10 -accelerator $aymainshortcuts(Redo)

    bind $w <[repcont $aymainshortcuts(Prefs)]> ".fu.fMenu.ed.m invoke 12"
    .fu.fMenu.ed.m entryconfigure 12 -accelerator $aymainshortcuts(Prefs)

    bind $w <[repcont $aymainshortcuts(Help)]> ".fu.fMenu.hlp.m invoke 0"
    .fu.fMenu.hlp.m entryconfigure 0 -accelerator $aymainshortcuts(Help)

    bind $w <[repcont $aymainshortcuts(Zap)]> zap
    bind $w <Map> unzap
}
# shortcut_main


# shortcut_toolbox:
# Setup special Keybindings for the toolbox
# XXXX currently unused
#
proc shortcut_toolbox { w } {
    global ayviewshortcuts aymainshortcuts

    bind $w <[repcont $aymainshortcuts(Undo)]> ".fu.fMenu.ed.m invoke 9"
    bind $w <[repcont $aymainshortcuts(Redo)]> ".fu.fMenu.ed.m invoke 10"
}
# shortcut_toolbox


# shortcut_view:
# Setup Keybindings for a 3D-View
#
#
proc shortcut_view { w } {
    global ayviewshortcuts aymainshortcuts

    bind $w <[repcont $aymainshortcuts(Undo)]> ".fu.fMenu.ed.m invoke 9"
    bind $w <[repcont $aymainshortcuts(Redo)]> ".fu.fMenu.ed.m invoke 10"

    bind $w <[repcont $ayviewshortcuts(QRender)]> "$w.fMenu.v.m invoke 0"
    $w.fMenu.v.m entryconfigure 0 -accelerator $ayviewshortcuts(QRender)
    bind $w <[repcont $ayviewshortcuts(Render)]> "$w.fMenu.v.m invoke 1"
    $w.fMenu.v.m entryconfigure 1 -accelerator $ayviewshortcuts(Render)
    bind $w <[repcont $ayviewshortcuts(Redraw)]> "$w.fMenu.v.m invoke 2"
    $w.fMenu.v.m entryconfigure 2 -accelerator $ayviewshortcuts(Redraw)

    bind $w <[repcont $aymainshortcuts(ExportRIB)]> "$w.fMenu.v.m invoke 3"
    $w.fMenu.v.m entryconfigure 3 -accelerator $aymainshortcuts(ExportRIB)

    bind $w <[repcont $ayviewshortcuts(Close)]> "$w.fMenu.v.m invoke 5"
    $w.fMenu.v.m entryconfigure 5 -accelerator $ayviewshortcuts(Close)

    bind $w <[repcont $ayviewshortcuts(Front)]> "$w.fMenu.t.m invoke 0"
    $w.fMenu.t.m entryconfigure 0 -accelerator $ayviewshortcuts(Front)
    bind $w <[repcont $ayviewshortcuts(Side)]> "$w.fMenu.t.m invoke 1"
    $w.fMenu.t.m entryconfigure 1 -accelerator $ayviewshortcuts(Side)
    bind $w <[repcont $ayviewshortcuts(Top)]> "$w.fMenu.t.m invoke 2"
    $w.fMenu.t.m entryconfigure 2 -accelerator $ayviewshortcuts(Top)
    bind $w <[repcont $ayviewshortcuts(Persp)]> "$w.fMenu.t.m invoke 4"
    $w.fMenu.t.m entryconfigure 4 -accelerator $ayviewshortcuts(Persp)
    bind $w <[repcont $ayviewshortcuts(Trim)]> "$w.fMenu.t.m invoke 6"
    $w.fMenu.t.m entryconfigure 6 -accelerator $ayviewshortcuts(Trim)

    bind $w <[repcont $ayviewshortcuts(Auto)]> "$w.fMenu.c.m invoke 0"
    $w.fMenu.c.m entryconfigure 0 -accelerator $ayviewshortcuts(Auto)
    bind $w <[repcont $ayviewshortcuts(Shade)]> "$w.fMenu.c.m invoke 1"
    $w.fMenu.c.m entryconfigure 1 -accelerator $ayviewshortcuts(Shade)

    bind $w <[repcont $ayviewshortcuts(DGrid)]> "$w.fMenu.c.m invoke 5"
    $w.fMenu.c.m entryconfigure 5 -accelerator $ayviewshortcuts(DGrid)
    bind $w <[repcont $ayviewshortcuts(UGrid)]> "$w.fMenu.c.m invoke 6"
    $w.fMenu.c.m entryconfigure 6 -accelerator $ayviewshortcuts(UGrid)
    bind $w <[repcont $ayviewshortcuts(SGrid)]> "$w.fMenu.c.m invoke 7"
    $w.fMenu.c.m entryconfigure 7 -accelerator $ayviewshortcuts(SGrid)

    bind $w <[repcont $ayviewshortcuts(Halve)]> "$w.fMenu.c.m invoke 11"
    $w.fMenu.c.m entryconfigure 11 -accelerator $ayviewshortcuts(Halve)
    bind $w <[repcont $ayviewshortcuts(Double)]> "$w.fMenu.c.m invoke 12"
    $w.fMenu.c.m entryconfigure 12 -accelerator $ayviewshortcuts(Double)

    $w.fMenu.c.m entryconfigure 18 -accelerator $ayviewshortcuts(ZoomTO)
    bind $w <[repcont $ayviewshortcuts(ZoomTO)]> "$w.fMenu.c.m invoke 18"
    $w.fMenu.c.m entryconfigure 19 -accelerator $ayviewshortcuts(Align)
    bind $w <[repcont $ayviewshortcuts(Align)]> "$w.fMenu.c.m invoke 19"


    bind $w <[repcont $ayviewshortcuts(RotL)]>\
	    "$w.f3D.togl setconf -drotx 5.0 -droty 0.0"
    bind $w <[repcont $ayviewshortcuts(RotR)]>\
	    "$w.f3D.togl setconf -drotx -5.0 -droty 0.0"
    bind $w <[repcont $ayviewshortcuts(RotU)]>\
	    "$w.f3D.togl setconf -droty -5.0 -drotx 0.0"
    bind $w <[repcont $ayviewshortcuts(RotD)]>\
	    "$w.f3D.togl setconf -droty 5.0 -drotx 0.0"

    bind $w <[repcont $ayviewshortcuts(ZoomI)]> {
	%W.f3D.togl setconf -dzoom 0.9
    }

    bind $w <[repcont $ayviewshortcuts(ZoomO)]> {
	%W.f3D.togl setconf -dzoom 1.1
    }

    bind $w <[repcont $aymainshortcuts(Zap)]> zap
    bind $w <Map> unzap
return

}
# shortcut_view

# shortcut_viewactions:
# Setup Keybindings for a 3D-View
#
#
proc shortcut_viewactions { w } {
    global ayviewshortcuts

set i $ayviewshortcuts(ZoomVButton)
    
bind $w.f3D.togl <ButtonPress-${i}> {
	undo view [winfo name [winfo toplevel %W]]
	%W zoomvac -start %y
	update
}
bind $w.f3D.togl <B${i}-Motion> {
	%W zoomvac -winy %y
	update
}

set i  $ayviewshortcuts(MoveVButton)

bind $w.f3D.togl <ButtonPress-${i}> {
	undo view [winfo name [winfo toplevel %W]]
	%W movevac -start %x %y
	update
}
bind $w.f3D.togl <B${i}-Motion> {
	%W movevac -winxy %x %y
	update
}


bind $w.f3D.togl <ButtonPress-4> {
    undo view [winfo name [winfo toplevel %W]]
    $ay(currentView) setconf -dzoom 0.5
    update
    $ay(currentView) reshape
    $ay(currentView) render
}


bind $w.f3D.togl <ButtonPress-5> {
    undo view [winfo name [winfo toplevel %W]]
    $ay(currentView) setconf -dzoom 2
    update
    $ay(currentView) reshape
    $ay(currentView) render
}

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

proc altrotatebinding { w } {
    bind $w <B1-Motion> {
	%W setconf -drotx [expr ($oldx - %x)] -droty [expr ($oldy - %y)]
	set oldx %x
	set oldy %y
	update
    }
}

bind $w <Alt-ButtonPress-1> {
    undo view [winfo name [winfo toplevel %W]]
    set oldx %x
    set oldy %y
    set ay(oldbinding) [bind %W <B1-Motion>]
    altrotatebinding %W

    bind $w <Alt-KeyRelease> {
        bind %W <Alt-KeyRelease> ""
        bind %W <B1-Motion> $ay(oldbinding)
    }
}

 return;
}
# shortcut_viewactions