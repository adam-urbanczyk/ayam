#
# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2007 by Randolf Schultz
# (rschultz@informatik.uni-rostock.de) and others.
#
# All rights reserved.
#
# See the file License for details.

# zap.tcl - example script to demonstrate how to bind core functionality
# to the Ayam toolbox

set w .tbw.f.fzap

# attach to tool window
if { [winfo exists .tbw] && ![winfo exists $w] } {

    # create a frame:
    set f [frame $w]

    # calculate the row number below the last row:
    set row [expr [lindex [grid size .tbw.f] 1] + 1]

    # now display the frame at calculated row, spanning the whole window:
    grid $f -row $row -column 0 -columnspan [lindex [grid size .tbw.f] 0]\
	-sticky we

    # create a button inside the frame:
    button $f.b1 -width 10 -text "Zap Ayam" -command { after idle zap }

    # display the button
    pack $f.b1 -side left -fill x -expand yes
}
# if
