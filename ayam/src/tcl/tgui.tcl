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
    SParam 20
    MB1Down 0
    SaveToTag 0
    OldSMethod -1
}
}

# tgui_block:
#  block user interactions in main and toolbox windows that could
#  destroy objects
proc tgui_block { } {
    global ay

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

    # modify mouse cursor to a watch
    mouseWatch 1 {. .tbw}

 return;
}
# tgui_block


# tgui_unblock:
#  unblock user interactions in main and toolbox windows that could
#  destroy objects
proc tgui_unblock { } {
    global ay

    bind Button <1> $ay(ButtonBinding)
    bind Menubutton <1> $ay(MenubuttonBinding)
    bind Listbox <1> $ay(ListboxBinding)
    bind Listbox <ButtonRelease-1> $ay(ListboxRBinding)

    # bind Objects label
    if { $ay(lb) == 0 } {
	bind .fu.fMain.fHier.ftr.la <Double-1> $ay(oldlabelbinding)
	set t $ay(tree)
	bind $t <1> $ay(oldtreeb1binding)
	bind $t <ButtonRelease-1> $ay(oldtreeb1rbinding)
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

    if { $ayprefs(LazyNotify) == 1 } {
	if { $tgui_tessparam(MB1Down) == 0 } {
	    tguiCmd up $tgui_tessparam(SMethod) $tgui_tessparam(SParam)
	}
    } else {
	tguiCmd up $tgui_tessparam(SMethod) $tgui_tessparam(SParam)
    }

    .tguiw.f1.fSParam.e delete 0 end
    .tguiw.f1.fSParam.e insert 0 $tgui_tessparam(SParam)

    if { $tgui_tessparam(SMethod) == 0 } {
	.tguiw.f1.fSParam.ll conf -text "0"
	.tguiw.f1.fSParam.lr conf -text "100"
	.tguiw.f1.fSParam.s conf -from 0 -to 100
	if { $tgui_tessparam(OldSMethod) != $tgui_tessparam(SMethod) } {
	    .tguiw.f1.fSParam.s conf -resolution 1
	}
    }

    if { $tgui_tessparam(SMethod) == 1 } {
	.tguiw.f1.fSParam.ll conf -text "0"
	.tguiw.f1.fSParam.lr conf -text "100"
	.tguiw.f1.fSParam.s conf -from 0 -to 100
	if { $tgui_tessparam(OldSMethod) != $tgui_tessparam(SMethod) } {
	    .tguiw.f1.fSParam.s conf -resolution 1
	}
    }

    if { $tgui_tessparam(SMethod) == 2 } {
	.tguiw.f1.fSParam.ll conf -text "1"
	.tguiw.f1.fSParam.lr conf -text "20"
	.tguiw.f1.fSParam.s conf -from 0 -to 20
	if { $tgui_tessparam(OldSMethod) != $tgui_tessparam(SMethod) } {
	    .tguiw.f1.fSParam.s conf -resolution 0.1
	}
    }

    set tgui_tessparam(OldSMethod) $tgui_tessparam(SMethod)

 return;
}
# tgui_update


# tgui_recalcslider:
#  re-calculate resolution of slider according to val
#
proc tgui_recalcslider { val } {

    set rmin [.tguiw.f1.fSParam.s cget -from]
    set rmax [.tguiw.f1.fSParam.s cget -to]

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

    .tguiw.f1.fSParam.s configure -resolution $tb

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
	    set val [format "%d,%g" $tgui_tessparam(SMethod)\
			 $tgui_tessparam(SParam)]
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

    forAllT NPatch 0 {

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
    # forAllT

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
	    scan $val "%d,%g" smethod sparam

	    tgui_recalcslider $sparam

	    set tgui_tessparam(OldSMethod) $smethod
	    set tgui_tessparam(SParam) $sparam
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

    undo save Tesselate

    set w .tguiw
    catch {destroy $w}
    toplevel $w -class ayam
    wm title $w "Tesselation Parameters"
    wm iconname $w "Ayam"
    #wm transient $w .

    set f [frame $w.f1]
    pack $f -in $w -side top -fill x
    addMenu $f tgui_tessparam SMethod $ay(smethods)
    set f [frame $f.fSParam -relief sunken -borderwidth 1]
    label $f.l -text "SParam:" -width 14
    label $f.ll -text "0"
    scale $f.s -showvalue 0 -orient h -from 0 -to 100\
	-highlightthickness 0
    if { $ayprefs(LazyNotify) == 1 } {
	bind $f.s <ButtonPress-1> "set tgui_tessparam(MB1Down) 1"
	bind $f.s <ButtonRelease-1> "set tgui_tessparam(MB1Down) 0;tgui_update"
    }

    label $f.lr -text "100"
    entry $f.e -width 5

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

    pack $f.l -in $f -side left -fill x -expand no
    pack $f.ll -in $f -side left -expand no
    pack $f.s -in $f -side left -fill x -expand yes
    pack $f.lr -in $f -side left -expand no
    pack $f.e -in $f -side right -fill x -expand yes -padx 2

    pack $f -in $w.f1 -side top -fill x -expand yes

    # create first tesselation
    #tgui_update

    set f $w.f1
    addCheck $f tgui_tessparam SaveToTag

    set f [frame $w.f2]
    button $f.bok -text "Ok" -width 5 -command {
	tguiCmd op; focus .; destroy .tguiw;
	uCL cl; plb_update
    }
    # button

    button $f.bca -text "Cancel" -width 5 -command {
	tguiCmd ca; focus .; destroy .tguiw; undo;
	tgui_addtag;
	uCL cl {1 1};
	rV
    }
    # button

    pack $f.bok $f.bca -in $f -side left -fill x -expand yes
    pack $f -in $w -side bottom -fill x

    winCenter $w
    focus $w.f2.bok

    # create first tesselation
    tgui_update

    # initiate update machinery
    set f $w.f1.fSParam
    $f.s conf -variable tgui_tessparam(SParam) -command tgui_update
    trace variable tgui_tessparam(SMethod) w tgui_update
    #trace variable tgui_tessparam(SParam) w tgui_update

    event add <<CommitTG>> <KeyPress-Return> <FocusOut>
    bind $f.e <<CommitTG>> "if { \[$f.e get\] != \$tgui_tessparam(SParam) } { \
	                         $f.s conf -command \"\"; \
				 tgui_recalcslider \[$f.e get\]; \
				 $f.s set \[$f.e get\]; \
				 set tgui_tessparam(SParam) \[$f.e get\]; \
				 $f.s conf -command tgui_update; \
			     }"

    tgui_block

    tkwait window $w

    tgui_unblock

    after idle viewMouseToCurrent

 return;
}
# tgui_open
