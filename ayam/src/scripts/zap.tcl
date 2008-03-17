#
# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2007 by Randolf Schultz
# (randolf.schultz@gmail.com) and others.
#
# All rights reserved.
#
# See the file License for details.

# zap.tcl - example script to demonstrate how to bind core functionality
# to the Ayam toolbox

set f [toolbox_add fzap 3]

if { $f != "" } {
    # create a button inside the frame:
    button $f.b1 -width 10 -text "Zap Ayam" -command { after idle zap }

    # display the button
    pack $f.b1 -side left -fill x -expand yes
}

# cleanup
unset f
