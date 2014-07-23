# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2005 by Randolf Schultz
# (randolf.schultz@gmail.com) and others.
#
# All rights reserved.
#
# See the file License for details.

# tgui.tcl - Tesselation GUI Tcl code

uplevel #0 { array set tgui_tessparam {
    SMethod 0
    SParamU 20
    SParamV 20

    FT1 1
    SParamU1 10
    SParamV1 10

    FT2 1
    SParamU2 0.5
    SParamV2 0.5

    FT3 1
    SParamU3 1
    SParamV3 1

    FT4 1
    SParamU4 1
    SParamV4 1

    FT5 1
    SParamU5 1
    SParamV5 1

    UseTexCoords 0
    UseVertColors 0
    UseVertNormals 0

    RefineTrims 0

    LazyUpdate 0

    MB1Down 0
    ReadTag 0
    OldSMethod -1
    NumTriangles 0
}
array set tgui_tessparamdefaults [array get tgui_tessparam]
}

# tgui_block:
#  block user interactions in main and toolbox windows that could
#  destroy objects
proc tgui_block { blockMsg } {
    global ay ayprefs

    set ay(ButtonBinding) [bind Button <1>]
    set sc "if { (\[winfo toplevel %W\] == \".\") || \
	    (\[winfo toplevel %W\] == \".tbw\") } {\
	    ayError 2 Ayam \"$blockMsg\"; bell;\
	    break } else { eval {$ay(ButtonBinding)} }"
    bind Button <1> $sc

    set ay(MenubuttonBinding) [bind Menubutton <1>]
    set sc "if { \[winfo toplevel %W\] == \".\" } {\
	    ayError 2 Ayam \"$blockMsg\"; bell;\
	    break } else { eval {$ay(MenubuttonBinding)} }"
    bind Menubutton <1> $sc

    set ay(ListboxBinding) [bind Listbox <1>]
    set sc "if { \[winfo toplevel %W\] == \".\" } {\
	    ayError 2 Ayam \"$blockMsg\"; bell;\
	    break } else { eval {$ay(ListboxBinding)} }"
    bind Listbox <1> $sc

    set ay(ListboxRBinding) [bind Listbox <ButtonRelease-1>]
    set sc "if { \[winfo toplevel %W\] == \".\" } {\
	    ayError 2 Ayam \"$blockMsg\"; bell;\
	    break } else { eval {$ay(ListboxRBinding)} }"
    bind Listbox <ButtonRelease-1> $sc

    set sc "ayError 2 Ayam \"$blockMsg\"; bell; break"

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
	    ayError 2 Ayam \"$blockMsg\"; bell;\
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

    trace vdelete tgui_tessparam(UseTexCoords) w tgui_update
    trace vdelete tgui_tessparam(UseVertColors) w tgui_update
    trace vdelete tgui_tessparam(UseVertNormals) w tgui_update
    trace vdelete tgui_tessparam(RefineTrims) w tgui_update

    .tguiw.f1.fSParamU.e delete 0 end
    .tguiw.f1.fSParamU.e insert 0 $tgui_tessparam(SParamU)
    .tguiw.f1.fSParamV.e delete 0 end
    .tguiw.f1.fSParamV.e insert 0 $tgui_tessparam(SParamV)

    if { $tgui_tessparam(SMethod) == 0 } {
	if { $tgui_tessparam(OldSMethod) != $tgui_tessparam(SMethod) } {
	    if { $tgui_tessparam(FT1) == 1 } {
		set tgui_tessparam(SParamU) $tgui_tessparam(SParamU1)
		set tgui_tessparam(SParamV) $tgui_tessparam(SParamV1)
		set tgui_tessparam(FT1) 0
	    }
	    .tguiw.f1.fSParamU.ll conf -text "0"
	    .tguiw.f1.fSParamU.lr conf -text "100"
	    .tguiw.f1.fSParamU.s conf -from 0 -to 100
	    .tguiw.f1.fSParamU.s conf -resolution 1
	}
	.tguiw.f1.fSParamV.s conf -state disabled
	.tguiw.f1.fSParamV.e conf -state disabled
    }

    if { $tgui_tessparam(SMethod) == 1 } {
	if { $tgui_tessparam(OldSMethod) != $tgui_tessparam(SMethod) } {
	    if { $tgui_tessparam(FT2) == 1 } {
		set tgui_tessparam(SParamU) $tgui_tessparam(SParamU2)
		set tgui_tessparam(SParamV) $tgui_tessparam(SParamV2)
		set tgui_tessparam(FT2) 0
	    }
	    .tguiw.f1.fSParamU.ll conf -text "0"
	    .tguiw.f1.fSParamU.lr conf -text "100"
	    .tguiw.f1.fSParamU.s conf -from 0 -to 100
	    .tguiw.f1.fSParamU.s conf -resolution 1
	}
	.tguiw.f1.fSParamV.s conf -state disabled
	.tguiw.f1.fSParamV.e conf -state disabled
    }

    if { ($tgui_tessparam(SMethod) > 1) } {
	if { $tgui_tessparam(OldSMethod) != $tgui_tessparam(SMethod) } {
	    if { ($tgui_tessparam(SMethod) == 2) &&
		 ($tgui_tessparam(FT3) == 1 ) } {
		set tgui_tessparam(SParamU) $tgui_tessparam(SParamU3)
		set tgui_tessparam(SParamV) $tgui_tessparam(SParamV3)
		set tgui_tessparam(FT3) 0
	    }
	    if { ($tgui_tessparam(SMethod) == 3) &&
		 ($tgui_tessparam(FT4) == 1 ) } {
		set tgui_tessparam(SParamU) $tgui_tessparam(SParamU4)
		set tgui_tessparam(SParamV) $tgui_tessparam(SParamV4)
		set tgui_tessparam(FT4) 0
	    }
	    .tguiw.f1.fSParamU.ll conf -text "0"
	    .tguiw.f1.fSParamU.lr conf -text "20"
	    .tguiw.f1.fSParamU.s conf -from 0 -to 20
	    .tguiw.f1.fSParamV.ll conf -text "0"
	    .tguiw.f1.fSParamV.lr conf -text "20"
	    .tguiw.f1.fSParamV.s conf -from 0 -to 20

	    if { ($tgui_tessparam(SMethod) == 4) &&
		 ($tgui_tessparam(FT5) == 1 ) } {
		.tguiw.f1.fSParamU.lr conf -text "5"
		.tguiw.f1.fSParamU.s conf -from 0 -to 5
		.tguiw.f1.fSParamV.lr conf -text "5"
		.tguiw.f1.fSParamV.s conf -from 0 -to 5
		set tgui_tessparam(SParamU) $tgui_tessparam(SParamU5)
		set tgui_tessparam(SParamV) $tgui_tessparam(SParamV5)
		set tgui_tessparam(FT5) 0
	    }

	    .tguiw.f1.fSParamU.s conf -resolution 0.1
	    .tguiw.f1.fSParamV.s conf -resolution 0.1
	}
	.tguiw.f1.fSParamV.s conf -state normal
	.tguiw.f1.fSParamV.e conf -state normal
    }

    set tgui_tessparam(OldSMethod) $tgui_tessparam(SMethod)

    if { ! $tgui_tessparam(LazyUpdate) } {
	tguiCmd up $tgui_tessparam(SMethod) $tgui_tessparam(SParamU)\
	    $tgui_tessparam(SParamV) $tgui_tessparam(UseTexCoords)\
	    $tgui_tessparam(UseVertColors) $tgui_tessparam(UseVertNormals)\
	    $tgui_tessparam(RefineTrims)
    } else {
	if { $tgui_tessparam(MB1Down) == 0 } {
	    tguiCmd up $tgui_tessparam(SMethod) $tgui_tessparam(SParamU)\
		$tgui_tessparam(SParamV) $tgui_tessparam(UseTexCoords)\
		$tgui_tessparam(UseVertColors) $tgui_tessparam(UseVertNormals)\
		$tgui_tessparam(RefineTrims)
	}
    }

    trace variable tgui_tessparam(SMethod) w tgui_update
    #trace variable tgui_tessparam(SParamU) w tgui_update
    #trace variable tgui_tessparam(SParamV) w tgui_update

    trace variable tgui_tessparam(UseTexCoords) w tgui_update
    trace variable tgui_tessparam(UseVertColors) w tgui_update
    trace variable tgui_tessparam(UseVertNormals) w tgui_update
    trace variable tgui_tessparam(RefineTrims) w tgui_update

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

    undo save AddTPTag
    set tgui_tessparam(tagval) \
	[format "%d,%g,%g,%d" [expr $tgui_tessparam(SMethod) + 1]\
	     $tgui_tessparam(SParamU) $tgui_tessparam(SParamV)\
	     $tgui_tessparam(RefineTrims)]

    forAll -recursive 0 {
	global tgui_tessparam

	set tagnames ""
	set tagvals ""
	getTags tagnames tagvals
	if { ($tagnames == "" ) || ([lsearch -exact $tagnames "TP"] == -1) } {
	    # no tags, or TP tag not present already => just add a new tag
	    addTag "TP" $tgui_tessparam(tagval)
	} else {
	    # we have already a TP tag => change its value
	    set index [lsearch -exact $tagnames "TP"]
	    setTags -index $index "TP" $tgui_tessparam(tagval)
	}
	# if
    }
    # forAll

 return;
}
# tgui_addtag


# tgui_remtag:
#  remove TP tags from all selected NURBS patch objects
#
proc tgui_remtag { } {

    forAll -recursive 0 {
	delTags "TP"
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

    set tgui_tessparam(ReadTag) 0

    set tagnames ""
    set tagvals ""
    getTags tagnames tagvals
    if { ($tagnames != "" ) } {
	set index [lsearch -exact $tagnames "TP"]
	if { $index != -1 } {
	    set val [lindex $tagvals $index]

	    set smethod 0
	    set sparamu 20
	    set sparamv 20
	    set rtrims 0

	    scan $val "%d,%g,%g,%d" smethod sparamu sparamv rtrims

	    set tgui_tessparam(FT${smethod}) 0

	    set smethod [expr $smethod - 1]

	    tgui_recalcslider 0 $sparamu
	    tgui_recalcslider 1 $sparamv

	    set tgui_tessparam(OldSMethod) $smethod
	    set tgui_tessparam(SMethod) $smethod
	    set tgui_tessparam(SParamU) $sparamu
	    set tgui_tessparam(SParamV) $sparamv
	    set tgui_tessparam(RefineTrims) $rtrims

	    set tgui_tessparam(ReadTag) 1
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

    winAutoFocusOff

    # deselect property
    $ay(plb) selection clear 0 end
    plb_update

    undo save Tesselate

    set w .tguiw
    set t "Tesselation"
    winDialog $w $t

    set ay(bca) $w.f2.bca
    set ay(bok) $w.f2.bok

    set f [frame $w.f1]
    pack $f -in $w -side top -fill x

    set tgui_tessparam(LazyUpdate) $ayprefs(LazyNotify)

    addText $f t1 "Tesselation Parameters"
    addCommand $f c1 "Reset All!" {
	global tgui_tessparam tgui_tessparamdefaults
	set t $tgui_tessparam(ReadTag)
	array set tgui_tessparam [array get tgui_tessparamdefaults]
	set tgui_tessparam(ReadTag) $t
	tgui_update
	tgui_recalcslider 0 $tgui_tessparam(SParamU)
	tgui_recalcslider 1 $tgui_tessparam(SParamV)
    }
    addCheck $f tgui_tessparam LazyUpdate
    addCheck $f tgui_tessparam UseTexCoords
    addCheck $f tgui_tessparam UseVertColors
    addCheck $f tgui_tessparam UseVertNormals
    addParam $f tgui_tessparam RefineTrims {0 1 2 3 4 5}

    # SMethod
    addMenu $f tgui_tessparam SMethod $ay(smethods)

    # SParamU
    set f [frame $f.fSParamU -relief sunken -borderwidth 1]
    label $f.l -text "SParamU:" -width 14

    label $f.ll -text "0"
    scale $f.s -showvalue 0 -orient h -from 0 -to 100\
	-highlightthickness 1

    bind $f.s <ButtonPress-1> "set tgui_tessparam(MB1Down) 1;focus %W"
    bind $f.s <ButtonRelease-1> {
	set tgui_tessparam(MB1Down) 0;
	if { $tgui_tessparam(LazyUpdate) } {
	    tgui_update
	}
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
	-highlightthickness 1
    bind $f.s <ButtonPress-1> "set tgui_tessparam(MB1Down) 1;focus %W"
    bind $f.s <ButtonRelease-1> {
	set tgui_tessparam(MB1Down) 0;
	if { $tgui_tessparam(LazyUpdate) } {
	    tgui_update
	}
    }

    label $f.lr -text "100"
    entry $f.e -width 5

    pack $f.l -in $f -side left -fill x -expand no
    pack $f.ll -in $f -side left -expand no
    pack $f.s -in $f -side left -fill x -expand yes
    pack $f.lr -in $f -side left -expand no
    pack $f.e -in $f -side right -fill x -expand yes -padx 2

    pack $f -in $w.f1 -side top -fill x -expand yes

    set f $w.f1
    addText $f t2 "Tesselation Results"
    addInfo $f tgui_tessparam NumTriangles

    # set up undo system
    set ::ay(need_undo_clear) 0
    forAll -recursive 0 { if { [hasChild] } { set ::ay(need_undo_clear) 1 } }

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
	undo rewind; focus .; destroy .tguiw;
	return;
    }

    set f [frame $w.f2]
    button $f.bok -text "Ok" -width 5 -command {
	tguiCmd ok; focus .; destroy .tguiw;
	if { $::ay(need_undo_clear) } { undo clear }
	set ay(ul) $ay(CurrentLevel); uS 0 1; plb_update;
    }
    # button

    button $f.bsa -text "Save" -width 5 -command {
	tguiCmd ca; focus .; destroy .tguiw;
	undo rewind;
	tgui_addtag;
	uCL cl {1 1}; plb_update;
    }
    # button

    button $f.bca -text "Cancel" -width 5 -command {
	global tgui_tessparam
	tguiCmd ca; focus .; destroy .tguiw;
	if { $tgui_tessparam(ReadTag) == 1 } {
	    undo;
	} else {
	    undo rewind;
	}
	uCL cl {1 1}; plb_update;
    }
    # button

    pack $f.bok $f.bsa $f.bca -in $f -side left -fill x -expand yes
    pack $f -in $w -side bottom -fill x

    # Esc-key && close via window decoration == Cancel button
    bind $w <Escape> "$f.bca invoke"
    wm protocol $w WM_DELETE_WINDOW "$f.bca invoke"

    winRestoreOrCenter $w $t
    focus $w.f2.bok

    tgui_recalcslider 0 $tgui_tessparam(SParamU)
    tgui_recalcslider 1 $tgui_tessparam(SParamV)

    # create first tesselation
    tgui_update

    # initiate update machinery
    event add <<CommitTG>> <KeyPress-Return> <FocusOut>
    catch {event add <<CommitTG>> <Key-KP_Enter>}

    trace variable tgui_tessparam(UseTexCoords) w tgui_update
    trace variable tgui_tessparam(UseVertColors) w tgui_update
    trace variable tgui_tessparam(UseVertNormals) w tgui_update
    trace variable tgui_tessparam(RefineTrims) w tgui_update

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
			     };"
    if {$::tcl_platform(platform) == "windows" } {
	bind $f.e <<CommitTG>> "+if { \"%K\" == \"Return\" } {break};"
	bind $f.e <<CommitTG>> "+if { \"%K\" == \"KP_Enter\" } {break};"
    }

    set f $w.f1.fSParamV
    $f.s conf -variable tgui_tessparam(SParamV) -command tgui_update
    #trace variable tgui_tessparam(SParamV) w tgui_update
    bind $f.e <<CommitTG>> "if { \[$f.e get\] != \$tgui_tessparam(SParamV) } {\
	                         $f.s conf -command \"\"; \
				 tgui_recalcslider 1 \[$f.e get\]; \
				 $f.s set \[$f.e get\]; \
				 set tgui_tessparam(SParamV) \[$f.e get\]; \
				 $f.s conf -command tgui_update; \
			     };"
    if {$::tcl_platform(platform) == "windows" } {
	bind $f.e <<CommitTG>> "+if { \"%K\" == \"Return\" } {break};"
	bind $f.e <<CommitTG>> "+if { \"%K\" == \"KP_Enter\" } {break};"
    }

    # establish "Help"-binding
    shortcut_addcshelp $w ayam-5.html tesst

    tgui_block "User interaction is restricted by tesselation dialog!"

    tkwait window $w

    trace vdelete tgui_tessparam(SMethod) w tgui_update

    after idle tgui_unblock

    after idle viewMouseToCurrent

    winAutoFocusOn

 return;
}
# tgui_open
