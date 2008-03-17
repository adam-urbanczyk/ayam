# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2001 by Randolf Schultz
# (randolf.schultz@gmail.com) and others.
#
# All rights reserved.
#
# See the file License for details.

# zap.tcl - zap/unzap all application windows

#
proc zap { } {
    global ay

    foreach view $ay(views) {
	if {[winfo toplevel $view] == $view} {
	    lappend windows $view
	}
    }

    lappend windows .

    if { [winfo exists .prefsw] } { lappend windows .prefsw }
    if { [winfo exists .tbw] } { 
	if { [wm transient .tbw] == "" } {
	    lappend windows .tbw
	}
    }
    if { [winfo exists .ayscw] } { lappend windows .ayscw }
    set ay(zapped) ""
    foreach w $windows {

	if { [winfo ismapped $w] } {

	    bind $w <Map> unzap
	    wm iconify $w 
	    lappend ay(zapped) $w
	}
    }

 return;
}
# zap

#
proc unzap { } { 
    global ay
    foreach w $ay(zapped) {
	if { [winfo exists $w ] } {
	    if { [string first iew $w] != -1 } { bind $w <Map> "" }
	    wm deiconify $w 
	}
    }
    set ay(zapped) ""

 return;
}
# unzap