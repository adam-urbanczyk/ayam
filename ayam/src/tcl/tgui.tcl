# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2004 by Randolf Schultz
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
}
}

# tgui_update:
#
#
proc tgui_update args {
    global ay ayprefs tgui_tessparam

    if { $ayprefs(LazyNotify) == 1 } {
	if { $tgui_tessparam(MB1Down) == 0 } {
	    puts "update $tgui_tessparam(SParam)"
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
	.tguiw.f1.fSParam.s conf -from 0 -to 100 -resolution 1
    }

    if { $tgui_tessparam(SMethod) == 1 } {
	.tguiw.f1.fSParam.ll conf -text "0"
	.tguiw.f1.fSParam.lr conf -text "100"
	.tguiw.f1.fSParam.s conf -from 0 -to 100 -resolution 1
    }

    if { $tgui_tessparam(SMethod) == 2 } {
	.tguiw.f1.fSParam.ll conf -text "1"
	.tguiw.f1.fSParam.lr conf -text "20"
	.tguiw.f1.fSParam.s conf -from 0 -to 20 -resolution 0.1
    }

 return;
}
# tgui_update


# tgui_addtag:
#
#
proc tgui_addtag { } {
    global tgui_tessparam
    
    if { $tgui_tessparam(SaveToTag) == 1 } {

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


# tgui_open:
#
#
proc tgui_open { } {
    global ay ayprefs ay_error tgui_tessparam

    undo save Tesselate

    set ay_error ""
    tguiCmd in
    if { $ay_error > 1 } {
	undo;
	return;
    }

    set w .tguiw
    catch {destroy $w}
    toplevel $w -class ayam
    wm title $w "Tesselation Parameters"
    wm iconname $w "Ayam"
    #wm transient $w .

    set f [frame $w.f1]
    pack $f -in $w -side top -fill x
    addMenu $f tgui_tessparam SMethod $ay(smethods)
    trace variable tgui_tessparam(SMethod) w tgui_update
    set f [frame $f.fSParam -relief sunken -borderwidth 1]
    label $f.l -text "SParam:" -width 14
    label $f.ll -text "0"
    scale $f.s -showvalue 0 -orient h -from 0 -to 100\
	-variable tgui_tessparam(SParam)\
	-highlightthickness 0
    if { $ayprefs(LazyNotify) == 1 } {
	bind $f.s <ButtonPress-1> "set tgui_tessparam(MB1Down) 1"
	bind $f.s <ButtonRelease-1> "set tgui_tessparam(MB1Down) 0;tgui_update"
    }

    label $f.lr -text "100"
    entry $f.e -width 5
    trace variable tgui_tessparam(SParam) w tgui_update
    pack $f.l -in $f -side left -fill x -expand no
    pack $f.ll -in $f -side left -expand no
    pack $f.s -in $f -side left -fill x -expand yes
    pack $f.lr -in $f -side left -expand no
    pack $f.e -in $f -side right -fill x -expand yes -padx 2

    pack $f -in $w.f1 -side top -fill x -expand yes
    tgui_update
    $f.s conf -command tgui_update
    event add <<CommitTG>> <KeyPress-Return> <FocusOut>
    bind $f.e <<CommitTG>> "$f.s conf -command \"\"; \
                            $f.s set \[$f.e get\]; \
                            $f.s conf -command tgui_update"

    set f $w.f1
    addCheck $f tgui_tessparam SaveToTag

    set f [frame $w.f2]
    button $f.bok -text "Ok" -width 5 -command {
	tguiCmd op; grab release .tguiw; focus .; destroy .tguiw;
	uCL cl; plb_update
    }
    # button

    button $f.bca -text "Cancel" -width 5 -command {
	tguiCmd ca; grab release .tguiw; focus .; destroy .tguiw; undo;
	tgui_addtag
    }
    # button

    pack $f.bok $f.bca -in $f -side left -fill x -expand yes
    pack $f -in $w -side bottom -fill x

    winCenter $w
    grab $w
    focus $w.f2.bok
    tkwait window $w

    after idle viewMouseToCurrent

 return;
}
# tgui_open