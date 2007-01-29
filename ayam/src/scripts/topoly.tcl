#
# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2007 by Randolf Schultz
# (rschultz@informatik.uni-rostock.de) and others.
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

	if { $ayprefs(PrepDiaCap) == 1 } {
	    set m "$t\n\n$m"
	}

	tk_messageBox -title $t -type ok -icon warning -message $m
	
	return;
    }
    # if

    mouseWatch 1 {. .tbw}

    # do a "toNPatch"
    set types { Box Sphere Cylinder Cone Disk Hyperboloid Torus Paraboloid \
		BPatch PatchMesh Revolve Extrude Sweep Birail1 Birail2 Skin \
		Gordon Cap Bevel Text }
    foreach type $types {
	forAllT $type 1 { convOb -inplace }
    }

    # update scene structure
    uS

    # now convert to PolyMesh
    forAllT NPatch 1 { convOb -inplace }

    # now convert MetaBalls to PolyMeshes
    forAllT MetaObj 1 { convOb -inplace }

    mouseWatch 0 {. .tbw}

    ayError 4 "toPoly" "Done converting everything to polygons."

    return;
}
# toPoly

# attach to custom menu
global ay
$ay(cm) add command -label "Convert All to PolyMesh" -command {
    toPoly; uS; rV; }

# attach to tool window
if { ![winfo exists .tbw.f.ftopoly] } {

    # create a frame:
    set f [frame .tbw.f.ftopoly]

    # calculate the row number below the last row:
    set row [expr [lindex [grid size .tbw.f] 1] + 1]

    # now display the frame at calculated row, spanning the whole window:
    grid $f -row $row -column 0 -columnspan [lindex [grid size .tbw.f] 0]\
	-sticky we

    # create a button inside the frame:
    button $f.b1 -width 10 -text "ToPolyMesh" -command { toPoly; uS; rV; }

    # display the button
    pack $f.b1 -side left -fill x -expand yes
}
# if
