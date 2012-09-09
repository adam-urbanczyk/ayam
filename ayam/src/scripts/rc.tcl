#
# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2010 by Randolf Schultz
# (randolf.schultz@gmail.com) and others.
#
# All rights reserved.
#
# See the file License for details.

# rc.tcl - revert cursor key bindings for parallel (modelling) views

# vars:

# set this to 0 if you do not want to enable the modified behavior
# upon loading of the script:
set rc_enableonload 1

# set the keyboard shortcut to enable/disable the modified behavior:
set rc_hotkey "<Shift-F11>"

# procs:

proc rc_rebind { view } {
    global ay ayviewshortcuts

    ${view}.f3D.togl mc

    set w $view

    if { $ay(cVType) != 3 } {
	# reverse binding

	bind $w <[repctrl $ayviewshortcuts(RotL)]>\
	    "viewPan $w.f3D.togl 0;break"
	bind $w <[repctrl $ayviewshortcuts(RotR)]>\
	    "viewPan $w.f3D.togl 1;break"
	bind $w <[repctrl $ayviewshortcuts(RotU)]>\
	    "viewPan $w.f3D.togl 2;break"
	bind $w <[repctrl $ayviewshortcuts(RotD)]>\
	    "viewPan $w.f3D.togl 3;break"

	bind $w <[repctrl $ayviewshortcuts(PanL)]>\
	    "$w.f3D.togl setconf -undokb ViewRotX \
             -drotx $::ayprefs(KbdRot) -droty 0.0;break"

	bind $w <[repctrl $ayviewshortcuts(PanR)]>\
	    "$w.f3D.togl setconf -undokb ViewRotX \
             -drotx -$::ayprefs(KbdRot) -droty 0.0;break"

	bind $w <[repctrl $ayviewshortcuts(PanU)]>\
	    "$w.f3D.togl setconf -undokb ViewRotY \
             -droty $::ayprefs(KbdRot) -drotx 0.0;break"

	bind $w <[repctrl $ayviewshortcuts(PanD)]>\
	    "$w.f3D.togl setconf -undokb ViewRotY \
             -droty -$::ayprefs(KbdRot) -drotx 0.0;break"

    } else {
	# normal binding

	bind $w <[repctrl $ayviewshortcuts(RotL)]>\
	    "$w.f3D.togl setconf -undokb ViewRotX \
             -drotx $::ayprefs(KbdRot) -droty 0.0;break"
	bind $w <[repctrl $ayviewshortcuts(RotR)]>\
	    "$w.f3D.togl setconf -undokb ViewRotX \
             -drotx -$::ayprefs(KbdRot) -droty 0.0;break"
	bind $w <[repctrl $ayviewshortcuts(RotU)]>\
	    "$w.f3D.togl setconf -undokb ViewRotY \
             -droty $::ayprefs(KbdRot) -drotx 0.0;break"
	bind $w <[repctrl $ayviewshortcuts(RotD)]>\
	    "$w.f3D.togl setconf -undokb ViewRotY \
             -droty -$::ayprefs(KbdRot) -drotx 0.0;break"

	bind $w <[repctrl $ayviewshortcuts(PanL)]>\
	    "viewPan $w.f3D.togl 0;break"

	bind $w <[repctrl $ayviewshortcuts(PanR)]>\
	    "viewPan $w.f3D.togl 1;break"

	bind $w <[repctrl $ayviewshortcuts(PanU)]>\
	    "viewPan $w.f3D.togl 2;break"

	bind $w <[repctrl $ayviewshortcuts(PanD)]>\
	    "viewPan $w.f3D.togl 3;break"

    }

    $ay(currentView) mc

 return;
}
# rc_rebind


# rc_enable:
#  enable the rc functionality
proc rc_enable { } {
    global ay

    foreach view $ay(views) {
	rc_rebind $view
    }

    addToProc viewCycleType "rc_rebind \$w"
    addToProc viewSetType "rc_rebind \$w"
    addToProc setViewAttr "rc_rebind \[winfo parent \[winfo parent \$togl\]\]"

    set ay(rc_enabled) 1

    ayError 4 "rc" "Enabled."

 return;
}
# rc_enable


# rc_disable:
#  disable the rc functionality
proc rc_disable { } {
    global ay

    #set ay(rc_enabled) 0

    ayError 2 "rc" "Disabling rc is currently unsupported."

 return;
}
# rc_disable

proc rc_toggle { } {
    global ay
    if { $ay(rc_enabled) } {
	rc_disable
    } else {
	rc_enable
    }
}
# rc_toggle


if { $rc_enableonload } {
    rc_enable
    addToProc viewBind "rc_rebind \$w"
} else {
    set ::ay(rc_enabled) 0
}

catch { unset rc_enableonload }

catch { bind . $rc_hotkey rc_toggle }

catch { shortcut_addviewbinding $rc_hotkey rc_toggle }

# EOF
