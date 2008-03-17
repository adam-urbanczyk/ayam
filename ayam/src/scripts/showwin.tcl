# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2008 by Randolf Schultz
# (randolf.schultz@gmail.com) and others.
#
# All rights reserved.
#
# See the file License for details.

# showwin.tcl - show all windows

proc showWindows { } {
    global ay ayprefs

    foreach child [winfo children .] {
	set toplevel [winfo toplevel $child]
	if { ![winfo ismapped $toplevel] } {
	    wm deiconify $toplevel
	}
	
    }
    # foreach
}
# showWindows

# attach to custom menu
global ay
$ay(cm) add command -label "ShowWindows" -command showWindows

# attach to tool window
if { [winfo exists .tbw] && ![winfo exists .tbw.f.fshow] } {

    # create a frame:
    set f [frame .tbw.f.fshow]

    # calculate the row number below the last row:
    set row [expr [lindex [grid size .tbw.f] 1] + 1]

    # now display the frame at calculated row, spanning the whole window:
    grid $f -row $row -column 0 -columnspan [lindex [grid size .tbw.f] 0]\
	-sticky we

    # create a button inside the frame:
    button $f.b1 -width 10 -text "ShowWindows" -command { showWindows }

    # display the button
    pack $f.b1 -side left -fill x -expand yes
}
# if
