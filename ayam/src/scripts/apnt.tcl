#
# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2011 by Randolf Schultz
# (randolf.schultz@gmail.com) and others.
#
# All rights reserved.
#
# See the file License for details.

# apnt.tcl - auto point, switch to point modelling mode after point selection

# vars:

# set this to 0 if you do not want to enable auto point
# upon loading of the script:
set apnt_enableonload 1

# set this to 0 if you do not want to create Custom menu entries
# upon loading of the script:
set apnt_enablemenu 1


# set this to 1 if you want all views to switch to point mode
# (instead of just the current view)
set apnt_scope 1

# set the keyboard shortcut to enable/disable auto point:
set apnt_hotkey "<F12>"

# procs:

# apnt_enable:
#  enable the apnt functionality by replacing some action procs
proc apnt_enable { } {
    global ay ayviewshortcuts

    # rename original procs to safety
    rename actionTagP o_actionTagP

    # establish modified procs
    rename apnt_actionTagP actionTagP

    # also bind to select all/none points shortcuts
    shortcut_addviewbinding <[repctrl $ayviewshortcuts(SelAllP)]>\
	"\{selPnts -all; rV; apnt_setmode; break\}"
    shortcut_addviewbinding <[repctrl $ayviewshortcuts(DeSelect)]>\
	"\{selPnts; rV; apnt_setmode 0; break\}"

    set ay(apnt_enabled) 1

    ayError 4 "apnt" "Enabled."

 return;
}
# apnt_enable


# apnt_disable:
#  disable the apnt functionality by replacing some action procs
proc apnt_disable { } {
    global ay ayviewshortcuts

    # rename modified procs to safety
    rename actionTagP apnt_actionTagP

    # establish original procs
    rename o_actionTagP actionTagP

    # also establish old bindings to select all/none points shortcuts
    shortcut_addviewbinding <[repctrl $ayviewshortcuts(SelAllP)]>\
	"selPnts -all; rV;break"
    shortcut_addviewbinding <[repctrl $ayviewshortcuts(DeSelect)]>\
	"selPnts; rV;break"

    set ay(apnt_enabled) 0

    ayError 4 "apnt" "Disabled."

 return;
}
# apnt_disable


# apnt_setmode:
#  set point modelling mode
proc apnt_setmode { {onoff 1} } {

    if { $::apnt_scope == 0 } {
	set w [winfo parent [winfo parent $::ay(currentView)]]

	viewSetPMode $w $onoff
    } else {
	set oldcw $::ay(currentView)

	foreach view $::ay(views) {
	    ${view}.f3D.togl mc
	    viewSetPMode $view $onoff
	}

	$oldcw mc
    }

 return;
}
# apnt_setmode

#
proc apnt_actionTagP { w } {
    global ayprefs ayviewshortcuts

    viewTitle $w "" "Select_Points"
    viewSetMAIcon $w ay_Tag_img "Select_Points"

    actionClearB1 $w

    bind $w <ButtonPress-1> {
	# undo save TagP
	%W mc
	set oldx %x
	set oldy %y
    }

    bind $w <ButtonRelease-1> {
	if { ($oldx != %x) || ($oldy != %y) } {
	    %W selpac %x %y $oldx $oldy
	} else {
	    %W selpac %x %y
	}
	%W setconf -rect $oldx $oldy %x %y 0
	rV
	update
	focus %W
	apnt_setmode
    }

    bind $w <${ayviewshortcuts(TagMod)}-ButtonRelease-1> {
	if { ($oldx != %x) || ($oldy != %y) } {
	    %W selpac %x %y $oldx $oldy 1
	} else {
	    %W selpac %x %y
	}
	%W setconf -rect $oldx $oldy %x %y 0
	rV
	update
	focus %W
	apnt_setmode
    }

    bind $w <B1-Motion> {
	%W setconf -rect $oldx $oldy %x %y 1
    }

    bind $w <Motion> ""

    if { $ayprefs(FlashPoints) == 1 } {
	bind $w <Motion> {
	    %W startpepac %x %y -readonly -flash
	}
	if { $ayprefs(FixFlashPoints) == 1 } {
	    bind $w <ButtonRelease-1> "+\
          %W startpepac %x %y -readonly -flash -ignoreold;\
          %W startpepac %x %y -readonly -flash -ignoreold"
	    bind $w <Shift-ButtonRelease-1> "+\
          %W startpepac %x %y -readonly -flash -ignoreold;\
          %W startpepac %x %y -readonly -flash -ignoreold"
	} else {
	    bind $w <ButtonRelease-1> "+\
          %W startpepac %x %y -readonly -flash -ignoreold;"
	    bind $w <Shift-ButtonRelease-1> "+\
          %W startpepac %x %y -readonly -flash -ignoreold;"
	}
    }
    $w setconf -drawh 1

 return;
}
# apnt_actionTagP


proc apnt_toggle { } {
    global ay
    if { $ay(apnt_enabled) } {
	apnt_disable
    } else {
	apnt_enable
    }
}
# apnt_toggle


if { $apnt_enableonload } {
    apnt_enable
} else {
    set ::ay(apnt_enabled) 0
}


catch { unset apnt_enableonload }

catch { bind . $apnt_hotkey apnt_toggle }

catch { shortcut_addviewbinding $apnt_hotkey apnt_toggle }

if { $apnt_enablemenu } {
    set m $ay(cm)
    $m add command -label "Toggle APNT" -command apnt_toggle\
	-accelerator [remkpkr $apnt_hotkey]
}

# EOF
