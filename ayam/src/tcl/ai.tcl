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
}

# ai_open:
#  open the AI Options dialogue
#
proc ai_open { } {
    global ay aiprefs

    winAutoFocusOff

    set oldFocus [focus]

    set w .aiw
    winDialog $w "AI Options"

    set f [frame $w.fu -bd 2 -relief sunken]

    set ay(bca) $w.fl.bca
    set ay(bok) $w.fl.bok

    addText $f e1 "AI Options:"
    addCheck $f aiprefs IgnoreTags
    addCheck $f aiprefs IgnoreMat

    pack $f -in $w -side top -fill x
    set f [frame $w.fl]
    button $f.bok -text "Start" -pady $ay(pady) -width 6 -command "\
	global ay;\
	restoreFocus $oldFocus;\
	destroy .aiw;\
	ai_makeInstances;\
	if \{ \[winfo exists \$ay(tree)\] \} \{\
	    tree_reset\
	\} else \{\
	    goTop; cS; uS; rV\
	\};\
	set ay(sc) 1;\
	undo clear"

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

    winCenter $w
    focus $f.bok
    tkwait window $w

    winAutoFocusOn

    after idle viewMouseToCurrent

 return;
}
# ai_open

# ai_resolve:
#  resolve all instances
#
proc ai_resolve { } {
    global ay
    ai_resolveInstances
    if { [winfo exists $ay(tree)] } {
	set ay(cl) $ay(CurrentLevel)
	uS 1 1
    } else {
	goTop; cS; uS; rV
    }
    set ay(sc) 1
    undo clear
 return;
}
# ai_resolve