# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2001 by Randolf Schultz
# (rschultz@informatik.uni-rostock.de) and others.
#
# All rights reserved.
#
# See the file License for details.

# ai.tcl - Automatic Instancing

array set aiprefs {
    IgnoreTags 0
    IgnoreMat 1
}

proc ai_open { } {
    global ay aiprefs

    set w .aiw
    catch {destroy $w}
    toplevel $w -class ayam
    wm title $w "Ayam AI"
    wm iconname $w "Ayam"
    
    set f [frame $w.fu -bd 2 -relief sunken]

    addText $f e1 "AI Parameters:"
    addCheck $f aiprefs IgnoreTags
    addCheck $f aiprefs IgnoreMat

    pack $f -in $w -side top -fill x
    set f [frame $w.fl]
    button $f.bok -text "Start" -pady $ay(pady) -width 6 -command { 
	global ay
	focus .
	destroy .aiw
	ai_makeInstances
	if { [winfo exists $ay(tree)] } {
	    tree_reset
	} else {
	    goTop; cS; uS; rV
	}
	set ay(sc) 1
	undo clear
    }

    button $f.bca -text "Cancel" -pady $ay(pady) -width 6 -command { 
	focus .
	destroy .aiw
    }
    pack $f.bok $f.bca -in $f -side left -fill x -expand yes
    pack $f -in $w -side bottom -fill x

    winCenter $w

    focus $f.bok
}
# ai_open

proc ai_resolve { } {
    global ay
    ai_resolveInstances
    if { [winfo exists $ay(tree)] } {
	tree_reset
    } else {
	goTop; cS; uS; rV
    }
    set ay(sc) 1
    undo clear
}
# ai_resolve