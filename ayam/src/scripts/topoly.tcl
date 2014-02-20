#
# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2007 by Randolf Schultz
# (randolf.schultz@gmail.com) and others.
#
# All rights reserved.
#
# See the file License for details.

# topoly.tcl - convert everything to polygons

proc toPoly { } {
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

    ayError 4 "toPoly" "Starting conversion, please wait."

    # block the UI
    tgui_block "Interaction restricted, please wait..."

    # do a "toNPatch"
    forAll -r 2 { if { [convOb -check NPatch ] } { convOb -inplace } }

    # update scene structure
    uS

    # now convert all NPatch objects to PolyMeshes
    forAll -type NPatch -r 1 { convOb -inplace }

    # now convert Subdivision Surfaces to PolyMeshes
    forAll -type SDMesh -r 1 { convOb -inplace }

    # now convert MetaBalls to PolyMeshes
    forAll -type MetaObj -r 1 { convOb -inplace }

    # unblock the UI
    tgui_unblock

    ayError 4 "toPoly" "Done converting everything to polygons."

 return;
}
# toPoly

# attach to custom menu
global ay
$ay(cm) add command -label "ToPolyMesh" -command {
    toPoly; uS; rV; }

# attach to tool window

# add the frame to the toolbox
set f [toolbox_add ftopoly 3]

if { $f != "" } {
    # create a button inside the frame:
    button $f.b1 -width 10 -text "ToPolyMesh" -command { toPoly; uS; rV; }

    # display the button
    pack $f.b1 -side left -fill x -expand yes
}

# cleanup
unset f

