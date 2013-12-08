# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2005 by Randolf Schultz
# (randolf.schultz@gmail.com) and others.
#
# All rights reserved.
#
# See the file License for details.

# ai.tcl - Automatic Instancing

array set aiprefs {
    IgnoreTags 0
    IgnoreMat 1
    Scope 0
}

# ai_open:
#  open the AI Options dialogue
#
proc ai_open { resolve } {
    global ay ayprefs aiprefs

    winAutoFocusOff

    set oldFocus [focus]

    set w .aiw
    if { $resolve } {
	set t "Resolve All Instances"
    } else {
	set t "Automatic Instancing"
    }
    winDialog $w $t

    set f [frame $w.fu -bd 2 -relief sunken]

    set ay(bca) $w.fl.bca
    set ay(bok) $w.fl.bok

    if { $ayprefs(FixDialogTitles) == 1 } {
	addText $f e1 $t
    }
    addMenu $f aiprefs Scope [list "All" "Level" "Selection"]
    if { $resolve != 1 } {
	addCheck $f aiprefs IgnoreTags
	addCheck $f aiprefs IgnoreMat
    }
    pack $f -in $w -side top -fill x
    set f [frame $w.fl]
    button $f.bok -text "Start" -pady $ay(pady) -width 6\
	-command "ai $oldFocus $resolve"

    button $f.bca -text "Cancel" -pady $ay(pady) -width 6 -command "\
	restoreFocus $oldFocus;\
	destroy .aiw"

    pack $f.bok $f.bca -in $f -side left -fill x -expand yes
    pack $f -in $w -side bottom -fill x

    # Esc-key && close via window decoration == Cancel button
    bind $w <Escape> "$f.bca invoke"
    wm protocol $w WM_DELETE_WINDOW "$f.bca invoke"

    # establish "Help"-binding
    shortcut_addcshelp $w ayam-8.html ai

    winRestoreOrCenter $w $t
    focus $f.bok
    tkwait window $w

    winAutoFocusOn

    after idle viewMouseToCurrent

 return;
}
# ai_open


proc ai { oldFocus resolve } {
 global ay
    restoreFocus $oldFocus
    destroy .aiw
    if { $resolve } {
	ai_resolveInstances
    } else {
	ai_makeInstances
    }
    if { [winfo exists $ay(tree)] } {
	if { $resolve } {
	    set ay(cl) $ay(CurrentLevel)
	    uS 1 1
	} else {
	    tree_reset
	}
    } else {
	goTop; cS; uS; rV
    }
    set ay(sc) 1
    undo clear
 return;
}
# ai
