#
# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2009 by Randolf Schultz
# (randolf.schultz@gmail.com) and others.
#
# All rights reserved.
#
# See the file License for details.

# easysweep.tcl - create sweep from just one selected curve (the trajectory)

# easySweep:
#  create sweep from just one selected curve (the trajectory)
#
proc easySweep { } {
 # first, we create a sweep object
 crtOb Sweep

 # now, we need to move the selected curve (path) to
 # the sweep and create a cross-section curve there too
 # for that, we move the currently selected curve to the clipboard
 cutOb

 # enter the Sweep (the last object in the current level)
 goDown -1

 # now, we create a new curve (a closed B-Spline suitable as cross section)
 crtClosedBS -s 8

 # select the new object
 selOb 0

 # now, we rotate and scale the curve
 rotOb 0 90 0
 scalOb 0.25 0.25 1.0

 # move trajectory back (we use "pasmovOb" and _not_ "pasOb", because we
 # really want to move (and not copy) the curve object
 pasmovOb

 # go up to where we came from
 goUp

 # finally, update the GUI...
 uS
 sL

 # ...and redraw all views
 rV
}
# easySweep


# attach to custom menu
global ay
$ay(cm) add command -label "EasySweep" -command {
    easySweep; }

# add the frame to the toolbox
set f [toolbox_add feasysweep 3]

if { $f != "" } {
    # create a button inside the frame:
    button $f.b1 -width 10 -text "EasySweep" -command { easySweep; }

    # display the button
    pack $f.b1 -side left -fill x -expand yes
}

# cleanup
unset f
