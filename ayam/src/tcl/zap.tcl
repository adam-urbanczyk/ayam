# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2001 by Randolf Schultz
# (rschultz@informatik.uni-rostock.de) and others.
#
# All rights reserved.
#
# See the file License for details.

# zap.tcl - zap/unzap all application windows

#
proc zap { } {
    global ay
    set windows $ay(views)
    lappend windows .
    if { [winfo exists .prefsw] } { lappend windows .prefsw }
    if { [winfo exists .tbw] } { 
	if { $ayprefs(ToolBoxTrans) == 0 } {
	    lappend windows .tbw
	}
    }
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