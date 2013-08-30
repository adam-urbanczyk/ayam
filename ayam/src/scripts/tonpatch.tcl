#
# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2007 by Randolf Schultz
# (randolf.schultz@gmail.com) and others.
#
# All rights reserved.
#
# See the file License for details.

# tonpatch.tcl - convert everything to NURBS patches (remove parametrics)

proc toNPatch { } {
    global ay ayprefs

    # going to change many objects, better let the user save changes first
    if { $ay(sc) == 1 } {
	set t "Warning!"
	set m "Save your scene first!"

	if { $ayprefs(FixDialogTitles) == 1 } {
	    set m "$t\n\n$m"
	}

	tk_messageBox -title $t -type ok -icon warning -message $m

	return;
    }
    # if

    $ay(plb) selection clear 0 end
    update

    ayError 4 "toNPatch" "Starting conversion, please wait."

    # block the UI
    tgui_block "Interaction restricted, please wait..."

    # do a "toNPatch"
    forAll -r 2 { if { [convOb -check NPatch ] } { convOb -inplace } }

    # update scene structure
    uS

    # unblock the UI
    tgui_unblock

    ayError 4 "toNPatch" "Done converting everything to NURBS patches."

    return;
}
# toNPatch

# attach to custom menu
global ay
$ay(cm) add command -label "ToNPatch" -command {
    toNPatch; uS; rV; }

# attach to tool window

# add the frame to the toolbox
set f [toolbox_add ftonpatch 3]

if { $f != "" } {
    # create a button inside the frame:
    button $f.b1 -width 10 -text "ToNPatch" -command { toNPatch; uS; rV; }

    # display the button
    pack $f.b1 -side left -fill x -expand yes
}

# cleanup
unset f

