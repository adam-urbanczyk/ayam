# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2005 by Randolf Schultz
# (rschultz@informatik.uni-rostock.de) and others.
#
# All rights reserved.
#
# See the file License for details.

# tgui.tcl - Tesselation GUI Tcl code

uplevel #0 { array set tgui_tessparam {
    SMethod 0
    SParamU 20
    SParamV 20
    MB1Down 0
    SaveToTag 0
    OldSMethod -1
}
}

# tgui_block:
#  block user interactions in main and toolbox windows that could
#  destroy objects
proc tgui_block { } {
    global ay ayprefs

    set blockMsg "User interaction is restricted by tesselation dialog!"

    set ay(ButtonBinding) [bind Button <1>]
    set sc "if { (\[winfo toplevel %W\] == \".\") || \
	    (\[winfo toplevel %W\] == \".tbw\") } {\
	    ayError 2 Ayam \"$blockMsg\";\
	    break } else { eval {$ay(ButtonBinding)} }"
    bind Button <1> $sc

    set ay(MenubuttonBinding) [bind Menubutton <1>]
    set sc "if { \[winfo toplevel %W\] == \".\" } {\
	    ayError 2 Ayam \"$blockMsg\";\
	    break } else { eval {$ay(MenubuttonBinding)} }"
    bind Menubutton <1> $sc

    set ay(ListboxBinding) [bind Listbox <1>]
    set sc "if { \[winfo toplevel %W\] == \".\" } {\
	    ayError 2 Ayam \"$blockMsg\";\
	    break } else { eval {$ay(ListboxBinding)} }"
    bind Listbox <1> $sc

    set ay(ListboxRBinding) [bind Listbox <ButtonRelease-1>]
    set sc "if { \[winfo toplevel %W\] == \".\" } {\
	    ayError 2 Ayam \"$blockMsg\";\
	    break } else { eval {$ay(ListboxRBinding)} }"
    bind Listbox <ButtonRelease-1> $sc

    set sc "ayError 2 Ayam \"$blockMsg\"; break"

    # unbind Objects label and object tree or object listbox
    if { $ay(lb) == 0 } {
	set ay(oldlabelbinding) [bind .fu.fMain.fHier.ftr.la <Double-1>]
	bind .fu.fMain.fHier.ftr.la <Double-1> $sc
	set t $ay(tree)

	set ay(oldtreeb1binding) [bind $t <1>]
	bind $t <1> $sc
	set ay(oldtreeb1rbinding) [bind $t <ButtonRelease-1>]
	bind $t <ButtonRelease-1> $sc

	set ay(oldtreeb2binding) [bind $t <2>]
	bind $t <2> $sc

	set ay(oldtreeb3binding) [bind $t <3>]
	bind $t <3> $sc

	$t bindText <ButtonRelease-1> ""
	$t bindText <ButtonPress-1> ""
    } else {
	set ay(oldlabelbinding) [bind .fu.fMain.fHier.flb.la <Double-1>]
	bind .fu.fMain.fHier.flb.la <Double-1> $sc
    }

    # unbind property listbox
    set ay(oldplbb1rbinding) [bind $ay(plb) <ButtonRelease-1>]
    bind $ay(plb) <ButtonRelease-1> $sc

    # unbind toolbox and main window via a new bindtag "tguinone"
    bind tguinone <Control-KeyPress> $sc
    bind tguinone <KeyPress> $sc

    if { [winfo exists .tbw] } {
	bindtags .tbw {tguinone .tbw ayam all}
    }
    bindtags .fl.con {tguinone .fl.con . all}
    bindtags . {tguinone . Ayam all}

    set sc "if { \[winfo toplevel %W\] == \".\" } {\
	    ayError 2 Ayam \"$blockMsg\";\
	    break } else { continue }"

    bind Frame <Control-KeyPress> $sc
    bind Canvas <Control-KeyPress> $sc
    bind Console <Control-KeyPress> $sc
    bind Listbox <Control-KeyPress> $sc

    winAutoFocusOff

    # modify mouse cursor to a watch
    mouseWatch 1 {. .tbw}

 return;
}
# tgui_block


# tgui_unblock:
#  unblock user interactions in main and toolbox windows that could
#  destroy objects
proc tgui_unblock { } {
    global ay ayprefs

    bind Button <1> $ay(ButtonBinding)
    bind Menubutton <1> $ay(MenubuttonBinding)
    bind Listbox <1> $ay(ListboxBinding)
    bind Listbox <ButtonRelease-1> $ay(ListboxRBinding)

    if { $ay(lb) == 0 } {
	# bind Objects label
	bind .fu.fMain.fHier.ftr.la <Double-1> $ay(oldlabelbinding)
	# re-establish tree bindings
	set t $ay(tree)
	bind $t <1> $ay(oldtreeb1binding)
	bind $t <ButtonRelease-1> $ay(oldtreeb1rbinding)
	bind $t <2> $ay(oldtreeb2binding)
	bind $t <3> $ay(oldtreeb3binding)
	$t bindText <ButtonRelease-1> ""
	$t bindText <ButtonPress-1> "tree_selectItem 1 $t"
    } else {
	bind .fu.fMain.fHier.flb.la <Double-1> $ay(oldlabelbinding)
    }

    bind $ay(plb) <ButtonRelease-1> $ay(oldplbb1rbinding)

    if { [winfo exists .tbw] } {
	bindtags .tbw {.tbw ayam all}
	bind tguinone <Control-KeyPress> ""
    }

    bindtags .fl.con {.fl.con . all}
    bindtags . {. Ayam all}

    bind Frame <Control-KeyPress> ""
    bind Canvas <Control-KeyPress> ""
    bind Console <Control-KeyPress> ""
    bind Listbox <Control-KeyPress> ""

    winAutoFocusOn

    # reset mouse cursor
    mouseWatch 0 {. .tbw}

 return;
}
# tgui_unblock


# tgui_update:
#
#
proc tgui_update args {
    global ay ayprefs tgui_tessparam

    trace vdelete tgui_tessparam(SMethod) w tgui_update
    #trace vdelete tgui_tessparam(SParamU) w tgui_update
    #trace vdelete tgui_tessparam(SParamV) w tgui_update

    if { $ayprefs(LazyNotify) == 1 } {
	if { $tgui_tessparam(MB1Down) == 0 } {
	    tguiCmd up $tgui_tessparam(SMethod) $tgui_tessparam(SParamU)\
		$tgui_tessparam(SParamV)
	}
    } else {
	tguiCmd up $tgui_tessparam(SMethod) $tgui_tessparam(SParamU)\
	    $tgui_tessparam(SParamV)
    }

    .tguiw.f1.fSParamU.e delete 0 end
    .tguiw.f1.fSParamU.e insert 0 $tgui_tessparam(SParamU)
    .tguiw.f1.fSParamV.e delete 0 end
    .tguiw.f1.fSParamV.e insert 0 $tgui_tessparam(SParamV)

    if { $tgui_tessparam(SMethod) == 0 } {
	if { $tgui_tessparam(OldSMethod) != $tgui_tessparam(SMethod) } {
	    .tguiw.f1.fSParamU.ll conf -text "0"
	    .tguiw.f1.fSParamU.lr conf -text "100"
	    .tguiw.f1.fSParamU.s conf -from 0 -to 100
	    .tguiw.f1.fSParamU.s conf -resolution 1
	}
	pack forget .tguiw.f1.fSParamV
    }

    if { $tgui_tessparam(SMethod) == 1 } {
	if { $tgui_tessparam(OldSMethod) != $tgui_tessparam(SMethod) } {
	    .tguiw.f1.fSParamU.ll conf -text "0"
	    .tguiw.f1.fSParamU.lr conf -text "100"
	    .tguiw.f1.fSParamU.s conf -from 0 -to 100
	    .tguiw.f1.fSParamU.s conf -resolution 1
	}
	pack forget .tguiw.f1.fSParamV
    }

    if { ($tgui_tessparam(SMethod) == 2) || ($tgui_tessparam(SMethod) == 3) } {

	if { $tgui_tessparam(OldSMethod) != $tgui_tessparam(SMethod) } {
	    .tguiw.f1.fSParamU.ll conf -text "1"
	    .tguiw.f1.fSParamU.lr conf -text "20"
	    .tguiw.f1.fSParamU.s conf -from 0 -to 20
	    .tguiw.f1.fSParamV.ll conf -text "1"
	    .tguiw.f1.fSParamV.lr conf -text "20"
	    .tguiw.f1.fSParamV.s conf -from 0 -to 20
	    .tguiw.f1.fSParamU.s conf -resolution 0.1
	    .tguiw.f1.fSParamV.s conf -resolution 0.1
	}
	pack .tguiw.f1.fSParamV -in .tguiw.f1 -side top
    }

    set tgui_tessparam(OldSMethod) $tgui_tessparam(SMethod)

    trace variable tgui_tessparam(SMethod) w tgui_update
    #trace variable tgui_tessparam(SParamU) w tgui_update
    #trace variable tgui_tessparam(SParamV) w tgui_update

 return;
}
# tgui_update


# tgui_recalcslider:
#  re-calculate resolution of slider according to val
#
proc tgui_recalcslider { slider val } {

    if { $slider == 0 } {
	set rmin [.tguiw.f1.fSParamU.s cget -from]
	if { $val < $rmin } {
	    .tguiw.f1.fSParamU.s conf -from $val
	    .tguiw.f1.fSParamU.ll conf -text $val
	    set rmin $val
	}
	set rmax [.tguiw.f1.fSParamU.s cget -to]
	if { $val > $rmax } {
	    .tguiw.f1.fSParamU.s conf -to $val
	    .tguiw.f1.fSParamU.lr conf -text $val
	    set rmax $val
	}
    } else {
	set rmin [.tguiw.f1.fSParamV.s cget -from]
	if { $val < $rmin } {
	    .tguiw.f1.fSParamV.s conf -from $val
	    .tguiw.f1.fSParamV.ll conf -text $val
	    set rmin $val
	}
	set rmax [.tguiw.f1.fSParamV.s cget -to]
	if { $val > $rmax } {
	    .tguiw.f1.fSParamV.s conf -to $val
	    .tguiw.f1.fSParamV.lr conf -text $val
	    set rmax $val
	}
    }

    if { [expr (abs(int($val) - $val)) != 0.0] } {
	set ta [expr abs(int($val) - $val)]
	set tb 1.0
	set done 0
	while { $done != 1 } {
	    set tb [expr $tb/10.0]
	    set ta [expr (abs(int($ta*10.0) - ($ta*10.0)))]
	    if { $ta <= 0.0 } { set done 1 }
	}
	# while
    } else {
	if { [expr abs($rmin - $rmax)] <= 1.0 } {
	    set tb 0.1
	} else {
	    set tb 1.0
	}
    }
    # if

    if { $slider == 0 } {
	.tguiw.f1.fSParamU.s configure -resolution $tb
    } else {
	.tguiw.f1.fSParamV.s configure -resolution $tb
    }

 return;
}
# tgui_recalcslider


# tgui_addtag:
#
#
proc tgui_addtag { } {
    global tgui_tessparam

    if { $tgui_tessparam(SaveToTag) == 1 } {

	undo save AddTPTag

	forAllT NPatch 0 {
	    global tgui_tessparam
	    set val [format "%d,%g,%g" $tgui_tessparam(SMethod)\
			 $tgui_tessparam(SParamU) $tgui_tessparam(SParamV)]
	    set tagnames ""
	    set tagvals ""
	    getTags tagnames tagvals
	    if { ($tagnames == "" ) || ([lsearch $tagnames "TP"] == -1) } {
		# no tags, or TP tag not present already => just add a new tag
		addTag "TP" $val
	    } else {
		# we have already a TP tag => change its value
		set index [lsearch $tagnames "TP"]
		set tagvals [lreplace $tagvals $index $index $val]
		setTags $tagnames $tagvals
	    }
	    # if
	}
	# forAllT
    }
    # if

 return;
}
# tgui_addtag


# tgui_remtag:
#  remove TP tags from all selected NURBS patch objects
#
proc tgui_remtag { } {
    global tgui_tessparam

    forAll 0 {
	set tagnames ""
	set tagvals ""
	getTags tagnames tagvals
	if { ($tagnames != "") } {
	    while { ([lsearch $tagnames "TP"] != -1) } {
		set index [lsearch $tagnames "TP"]
		set tagnames [lreplace $tagnames $index $index]
		set tagvals [lreplace $tagvals $index $index]
		setTags $tagnames $tagvals
	    }
	    # while
	}
	# if
    }
    # forAll

 return;
}
# tgui_remtag


# tgui_readtag:
#  read TP tag from first selected object and set tesselation gui
#  parameters appropriately
proc tgui_readtag { } {
    global tgui_tessparam

    set tagnames ""
    set tagvals ""
    getTags tagnames tagvals
    if { ($tagnames != "" ) } {
	set index [lsearch $tagnames "TP"]
	if { $index != -1 } {
	    set val [lindex $tagvals $index]
	    scan $val "%d,%g,%g" smethod sparamu sparamv

	    tgui_recalcslider $sparamu $sparamv

	    set tgui_tessparam(OldSMethod) $smethod
	    set tgui_tessparam(SParamU) $sparamu
	    set tgui_tessparam(SParamV) $sparamv
	    set tgui_tessparam(SMethod) $smethod
	}
        # if
    }
    # if

 return;
}
# tgui_readtag


# tgui_open:
#  create tesselation gui
#
proc tgui_open { } {
    global ay ayprefs ay_error tgui_tessparam

    # deselect property
    $ay(plb) selection clear 0 end
    plb_update

    undo save Tesselate

    set w .tguiw
    catch {destroy $w}
    toplevel $w -class ayam
    wm title $w "Tesselation Parameters"
    wm iconname $w "Ayam"
    #wm transient $w .
    wm withdraw $w

    set f [frame $w.f1]
    pack $f -in $w -side top -fill x

    addCheck $f tgui_tessparam SaveToTag

    # SMethod
    addMenu $f tgui_tessparam SMethod $ay(smethods)

    # SParamU
    set f [frame $f.fSParamU -relief sunken -borderwidth 1]
    label $f.l -text "SParamU:" -width 14

    label $f.ll -text "0"
    scale $f.s -showvalue 0 -orient h -from 0 -to 100\
	-highlightthickness 0
    if { $ayprefs(LazyNotify) == 1 } {
	bind $f.s <ButtonPress-1> "set tgui_tessparam(MB1Down) 1"
	bind $f.s <ButtonRelease-1> "set tgui_tessparam(MB1Down) 0;tgui_update"
    }

    label $f.lr -text "100"
    entry $f.e -width 5

    pack $f.l -in $f -side left -fill x -expand no
    pack $f.ll -in $f -side left -expand no
    pack $f.s -in $f -side left -fill x -expand yes
    pack $f.lr -in $f -side left -expand no
    pack $f.e -in $f -side right -fill x -expand yes -padx 2

    pack $f -in $w.f1 -side top -fill x -expand yes

    # SParamV
    set f $w.f1
    set f [frame $f.fSParamV -relief sunken -borderwidth 1]
    label $f.l -text "SParamV:" -width 14

    label $f.ll -text "0"
    scale $f.s -showvalue 0 -orient h -from 0 -to 100\
	-highlightthickness 0
    if { $ayprefs(LazyNotify) == 1 } {
	bind $f.s <ButtonPress-1> "set tgui_tessparam(MB1Down) 1"
	bind $f.s <ButtonRelease-1> "set tgui_tessparam(MB1Down) 0;tgui_update"
    }

    label $f.lr -text "100"
    entry $f.e -width 5

    pack $f.l -in $f -side left -fill x -expand no
    pack $f.ll -in $f -side left -expand no
    pack $f.s -in $f -side left -fill x -expand yes
    pack $f.lr -in $f -side left -expand no
    pack $f.e -in $f -side right -fill x -expand yes -padx 2

    pack $f -in $w.f1 -side top -fill x -expand yes

    # set up undo system
    set ::ay(need_undo_clear) 0
    forAll 0 { if { [hasChild] } { set ::ay(need_undo_clear) 1 } }

    # read preferences from eventually present TP tag
    tgui_readtag

    # remove potentially present TP tags, lest they get in our way while
    # tesselating with our own method and parameter
    tgui_remtag

    # copy selected objects to internal buffer

    set ay_error ""
    tguiCmd in
    if { $ay_error > 1 } {
	# no objects copied => nothing to tesselate => bail out
	undo; focus .; destroy .tguiw;
	return;
    }


    set f [frame $w.f2]
    button $f.bok -text "Ok" -width 5 -command {
	tguiCmd op; focus .; destroy .tguiw;
	if { $::ay(need_undo_clear) } { undo clear }
	set ay(ul) $ay(CurrentLevel); uS 0 1; plb_update;
    }
    # button

    button $f.bca -text "Cancel" -width 5 -command {
	tguiCmd ca; focus .; destroy .tguiw;
	tgui_addtag;
	uCL cl {1 1}; plb_update;
	rV;
    }
    # button

    pack $f.bok $f.bca -in $f -side left -fill x -expand yes
    pack $f -in $w -side bottom -fill x

    winCenter $w
    focus $w.f2.bok

    # create first tesselation
    tgui_update

    # initiate update machinery
    event add <<CommitTG>> <KeyPress-Return> <FocusOut>
    trace variable tgui_tessparam(SMethod) w tgui_update

    set f $w.f1.fSParamU
    $f.s conf -variable tgui_tessparam(SParamU) -command tgui_update
    #trace variable tgui_tessparam(SParamU) w tgui_update
    bind $f.e <<CommitTG>> "if { \[$f.e get\] != \$tgui_tessparam(SParamU) } {\
	                         $f.s conf -command \"\"; \
				 tgui_recalcslider 0 \[$f.e get\]; \
				 $f.s set \[$f.e get\]; \
				 set tgui_tessparam(SParamU) \[$f.e get\]; \
				 $f.s conf -command tgui_update; \
			     }"

    set f $w.f1.fSParamV
    $f.s conf -variable tgui_tessparam(SParamV) -command tgui_update
    #trace variable tgui_tessparam(SParamV) w tgui_update
    bind $f.e <<CommitTG>> "if { \[$f.e get\] != \$tgui_tessparam(SParamV) } {\
	                         $f.s conf -command \"\"; \
				 tgui_recalcslider 1 \[$f.e get\]; \
				 $f.s set \[$f.e get\]; \
				 set tgui_tessparam(SParamV) \[$f.e get\]; \
				 $f.s conf -command tgui_update; \
			     }"

    wm deiconify $w

    tgui_block

    tkwait window $w

    trace vdelete tgui_tessparam(SMethod) w tgui_update

    after idle tgui_unblock

    after idle viewMouseToCurrent

 return;
}
# tgui_open
