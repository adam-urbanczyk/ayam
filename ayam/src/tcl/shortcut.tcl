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
    global ay aymainshortcuts

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

    set m $ay(filemenu)
    bind $w <[repcont $aymainshortcuts(New)]> "$m invoke 0"
    $m entryconfigure 0 -accelerator $aymainshortcuts(New)
    bind $w <[repcont $aymainshortcuts(Replace)]> "$m invoke 2"
    $m entryconfigure 2 -accelerator $aymainshortcuts(Replace)
    bind $w <[repcont $aymainshortcuts(Insert)]> "$m invoke 3"
    $m entryconfigure 3 -accelerator $aymainshortcuts(Insert)
    bind $w <[repcont $aymainshortcuts(SaveAs)]> "$m invoke 5"
    $m entryconfigure 5 -accelerator $aymainshortcuts(SaveAs)
    bind $w <[repcont $aymainshortcuts(Save)]> "$m invoke 6"
    $m entryconfigure 6 -accelerator $aymainshortcuts(Save)
    bind $w <[repcont $aymainshortcuts(ExportRIB)]> "$m invoke 10"
    $m entryconfigure 10 -accelerator $aymainshortcuts(ExportRIB)

    bind $w <[repcont $aymainshortcuts(MRU1)]> "$m invoke 16"
    $m entryconfigure 16\
	    -accelerator [repkp $aymainshortcuts(MRU1)]
    bind $w <[repcont $aymainshortcuts(MRU2)]> "$m invoke 17"
    $m entryconfigure 17\
	    -accelerator [repkp $aymainshortcuts(MRU2)]
    bind $w <[repcont $aymainshortcuts(MRU3)]> "$m invoke 18"
    $m entryconfigure 18\
	    -accelerator [repkp $aymainshortcuts(MRU3)]
    bind $w <[repcont $aymainshortcuts(MRU4)]> "$m invoke 19"
    $m entryconfigure 19\
	    -accelerator [repkp $aymainshortcuts(MRU4)]


    bind $w <[repcont $aymainshortcuts(Quit)]> "$m invoke 21"
    $m entryconfigure 21 -accelerator $aymainshortcuts(Quit)


    bind $w <[repcont $aymainshortcuts(Apply)]> ".fu.fMain.fProp.fArg.fb.b1 invoke"

    set m $ay(editmenu)
    bind $w <[repcont $aymainshortcuts(Copy)]> "$m invoke 0"
    $m entryconfigure 0 -accelerator $aymainshortcuts(Copy)
    bind $w <[repcont $aymainshortcuts(Cut)]> "$m invoke 1"
    $m entryconfigure 1 -accelerator $aymainshortcuts(Cut)
    bind $w <[repcont $aymainshortcuts(Paste)]> "$m invoke 2"
    $m entryconfigure 2 -accelerator $aymainshortcuts(Paste)
    bind $w <[repcont $aymainshortcuts(CopyP)]> "$m invoke 5"
    $m entryconfigure 5 -accelerator $aymainshortcuts(CopyP)
    bind $w <[repcont $aymainshortcuts(ICopyP)]> "$m invoke 6"
    $m entryconfigure 6 -accelerator $aymainshortcuts(ICopyP)
    bind $w <[repcont $aymainshortcuts(PasteP)]> "$m invoke 7"
    $m entryconfigure 7 -accelerator $aymainshortcuts(PasteP)

    bind $w <[repcont $aymainshortcuts(Undo)]> "$m invoke 9"
    $m entryconfigure 9 -accelerator $aymainshortcuts(Undo)
    bind $w <[repcont $aymainshortcuts(Redo)]> "$m invoke 10"
    $m entryconfigure 10 -accelerator $aymainshortcuts(Redo)

    bind $w <[repcont $aymainshortcuts(Material)]> "$m invoke 12"
    $m entryconfigure 12 -accelerator $aymainshortcuts(Material)

    bind $w <[repcont $aymainshortcuts(Prefs)]> "$m invoke 14"
    $m entryconfigure 14 -accelerator $aymainshortcuts(Prefs)


    set m $ay(helpmenu)
    bind $w <[repcont $aymainshortcuts(Help)]> "$m invoke 0"
    $m entryconfigure 0 -accelerator $aymainshortcuts(Help)

    set m $ay(specialmenu)
    bind $w <[repcont $aymainshortcuts(Zap)]> zap
    $m entryconfigure 20 -accelerator $aymainshortcuts(Zap)

    bind $w <Map> unzap
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
# Setup Keybindings for a 3D-View
#
#
proc shortcut_view { w } {
    global ay ayviewshortcuts aymainshortcuts

    # some main window shortcuts
    set m $ay(editmenu)
    bind $w <[repcont $aymainshortcuts(Undo)]> "$m invoke 9"
    bind $w <[repcont $aymainshortcuts(Redo)]> "$m invoke 10"

    # view window shortcuts
    set m $ay(viewm)
    bind $w <[repcont $ayviewshortcuts(QRender)]> "$w.$m invoke 0"
    $w.$m entryconfigure 0 -accelerator $ayviewshortcuts(QRender)
    bind $w <[repcont $ayviewshortcuts(Render)]> "$w.$m invoke 1"
    $w.$m entryconfigure 1 -accelerator $ayviewshortcuts(Render)
    bind $w <[repcont $ayviewshortcuts(Redraw)]> "$w.$m invoke 2"
    $w.$m entryconfigure 2 -accelerator $ayviewshortcuts(Redraw)

    bind $w <[repcont $aymainshortcuts(ExportRIB)]> "$w.$m invoke 3"
    $w.$m entryconfigure 3 -accelerator $aymainshortcuts(ExportRIB)
    global AYENABLEPPREV
    if { $AYENABLEPPREV == 1 } { set tmp 10 } else { set tmp 7 } 
    bind $w <[repcont $ayviewshortcuts(Close)]> "$w.$m invoke $tmp"
    $w.$m entryconfigure $tmp -accelerator $ayviewshortcuts(Close)


    set m $ay(typem)
    bind $w <[repcont $ayviewshortcuts(Front)]> "$w.$m invoke 0"
    $w.$m entryconfigure 0 -accelerator $ayviewshortcuts(Front)
    bind $w <[repcont $ayviewshortcuts(Side)]> "$w.$m invoke 1"
    $w.$m entryconfigure 1 -accelerator $ayviewshortcuts(Side)
    bind $w <[repcont $ayviewshortcuts(Top)]> "$w.$m invoke 2"
    $w.$m entryconfigure 2 -accelerator $ayviewshortcuts(Top)
    bind $w <[repcont $ayviewshortcuts(Persp)]> "$w.$m invoke 4"
    $w.$m entryconfigure 4 -accelerator $ayviewshortcuts(Persp)
    bind $w <[repcont $ayviewshortcuts(Trim)]> "$w.$m invoke 6"
    $w.$m entryconfigure 6 -accelerator $ayviewshortcuts(Trim)

    set m $ay(confm)
    bind $w <[repcont $ayviewshortcuts(Auto)]> "$w.$m invoke 0"
    $w.$m entryconfigure 0 -accelerator $ayviewshortcuts(Auto)
    bind $w <[repcont $ayviewshortcuts(Shade)]> "$w.$m invoke 1"
    $w.$m entryconfigure 1 -accelerator $ayviewshortcuts(Shade)

    bind $w <[repcont $ayviewshortcuts(DGrid)]> "$w.$m invoke 7"
    $w.$m entryconfigure 7 -accelerator $ayviewshortcuts(DGrid)
    bind $w <[repcont $ayviewshortcuts(UGrid)]> "$w.$m invoke 8"
    $w.$m entryconfigure 8 -accelerator $ayviewshortcuts(UGrid)
    bind $w <[repcont $ayviewshortcuts(SGrid)]> "$w.$m invoke 9"
    $w.$m entryconfigure 9 -accelerator $ayviewshortcuts(SGrid)

    bind $w <[repcont $ayviewshortcuts(Halve)]> "$w.$m invoke 12"
    $w.$m entryconfigure 12 -accelerator $ayviewshortcuts(Halve)
    bind $w <[repcont $ayviewshortcuts(Double)]> "$w.$m invoke 13"
    $w.$m entryconfigure 13 -accelerator $ayviewshortcuts(Double)

    $w.$m entryconfigure 19 -accelerator $ayviewshortcuts(ZoomTO)
    bind $w <[repcont $ayviewshortcuts(ZoomTO)]> "$w.$m invoke 19"
    $w.$m entryconfigure 20 -accelerator $ayviewshortcuts(Align)
    bind $w <[repcont $ayviewshortcuts(Align)]> "$w.$m invoke 20"


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

    # this binding allows to rotate any view regardless
    # of any active action; the old active action will
    # be re established again once the mouse button is released
    bind $w.f3D.togl <$ayviewshortcuts(RotMod)-ButtonPress-1> {

	undo save

	set oldx %x
	set oldy %y
	# save old bindings
	set ay(oldbinding) [bind %W <B1-Motion>]
	set ay(oldb1binding) [bind %W <ButtonPress-1>]
	set ay(oldb1rbinding) [bind %W <ButtonRelease-1>]
	bind %W <ButtonPress-1> { #nothing }
	bind %W <ButtonRelease-1> { #nothing }
	shortcut_altrotatebinding %W

	bind %W <ButtonRelease-1> {
	    # restore old bindings
	    bind %W <ButtonRelease-1> { #nothing }
	    bind %W <B1-Motion> $ay(oldbinding)
	    bind %W <ButtonPress-1> $ay(oldb1binding)
	    bind %W <ButtonRelease-1> $ay(oldb1rbinding)
	}
	break;
    }

    bind $w <KeyPress-$ayviewshortcuts(RotModKey)> {
	[winfo toplevel %W].f3D.togl configure -cursor exchange
    }

    bind $w <KeyRelease-$ayviewshortcuts(RotModKey)> {
	[winfo toplevel %W].f3D.togl configure -cursor left_ptr
    }


    set i $ayviewshortcuts(ZoomVButton)
    
    bind $w.f3D.togl <ButtonPress-${i}> {
	undo save
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
	undo save
	%W mc
	%W movevac -start %x %y
	update
    }
    bind $w.f3D.togl <B${i}-Motion> {
	%W movevac -winxy %x %y
	update
    }

    
    bind $w.f3D.togl <ButtonPress-4> {
	undo save
	%W mc
	%W setconf -dzoom 0.5
	update
	%W reshape
	%W render
    }


    bind $w.f3D.togl <ButtonPress-5> {
	undo save
	%W mc
	%W setconf -dzoom 2
	update
	%W reshape
	%W render
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

 return;
}
# shortcut_viewactions


#shortcut_altrotatebinding:
# Setup key bindings for rotation of a 3D-View while
# the Alt key is hold down.
#
proc shortcut_altrotatebinding { w } {
    bind $w <B1-Motion> {
	%W setconf -drotx [expr ($oldx - %x)] -droty [expr ($oldy - %y)]
	set oldx %x
	set oldy %y
	update
	break;
    }
}
# shortcut_altrotatebinding

#shortcut_show:
# display all current shortcuts in a separate top level window
proc shortcut_show { } {
global ay aymainshortcuts ayviewshortcuts tcl_platform

set w .ayscw

catch {destroy $w}
toplevel $w
wm title $w "Ayam Shortcuts"
wm iconname $w "Ayam"

#XXXX make this window transient?

frame $w.ftext

frame $w.fbutton
pack $w.fbutton -in $w -side bottom
button $w.fbutton.b -text "Dismiss" -pady $ay(pady) -command "destroy $w"
pack $w.fbutton.b -in $w.fbutton

pack $w.ftext -in $w -side top -expand yes -fill both

set font fixed

if { $tcl_platform(platform) == "windows" } {
    set font fixedsys
}

# IRIXens "fixed" is too wide...
if { [string first "IRIX" $tcl_platform(os)] != -1 } {
    set font 9x15
}

text $w.ftext.text -yscrollcommand "$w.ftext.sbar set" \
	-setgrid 1 -font $font -height 20 -width 40
# -tabs {32c left}
scrollbar $w.ftext.sbar -takefocus 0 -command "$w.ftext.text yview"

pack $w.ftext.sbar -in $w.ftext -side right -fill y
pack $w.ftext.text -in $w.ftext -side left -fill both -expand yes

$w.ftext.text insert end "Modelling Actions (View Windows):

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


 FindU                $ayviewshortcuts(FindU)
 Split Curve          $ayviewshortcuts(SplitNC)

View Actions (View Windows):

 Break Action        $ayviewshortcuts(Break)

 Move View           $ayviewshortcuts(MoveV)
 Move View
 (along Z)           $ayviewshortcuts(MoveZV)
 Move View           Mouse-$ayviewshortcuts(MoveVButton)

 Rotate View         $ayviewshortcuts(RotV)
 Rotate View         $ayviewshortcuts(RotModKey)+Mouse-1

 Rotate View (left)  $ayviewshortcuts(RotL)
 Rotate View (right) $ayviewshortcuts(RotR)
 Rotate View (up)    $ayviewshortcuts(RotU)
 Rotate View (down)  $ayviewshortcuts(RotD)

 Zoom View           $ayviewshortcuts(ZoomV)
 Zoom in View        $ayviewshortcuts(ZoomI)
 Zoom out View       $ayviewshortcuts(ZoomO)
 Zoom View           Mouse-$ayviewshortcuts(ZoomVButton)

See the reference card in the docs
folder for a printable and more
complete version.
"


$w.ftext.text configure -state disabled
bind $w <Next> "$w.ftext.text yview scroll 1 pages"
bind $w <Prior> "$w.ftext.text yview scroll -1 pages"

bind $w.ftext.text <ButtonPress-4>\
"$w.ftext.text yview scroll -1 pages; break"

bind $w.ftext.text <ButtonPress-5>\
"$w.ftext.text yview scroll 1 pages; break"


}
#shortcut_show