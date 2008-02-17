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

    $ay(plb) selection clear 0 end
    update

    ayError 4 "toPoly" "Starting conversion. Please wait."

    # block the UI
    tgui_block "Interaction restricted, please wait..."

    # do a "toNPatch"
    set types { Box Sphere Cylinder Cone Disk Hyperboloid Torus Paraboloid \
		BPatch PatchMesh Revolve Extrude Sweep Birail1 Birail2 Skin \
		Gordon Cap Bevel Text Swing }
    foreach type $types {
	forAllT $type 1 { convOb -inplace }
    }

    # update scene structure
    uS

    # now convert to PolyMesh
    forAllT NPatch 1 { convOb -inplace }

    # now convert MetaBalls to PolyMeshes
    forAllT MetaObj 1 { convOb -inplace }

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
set tf ""
if { [winfo exists .tbw] && ![winfo exists .tbw.f.ftopoly] } {
    set tf .tbw.f
}

if { [winfo exists .fv.fTools] && ![winfo exists .fv.fTools.f.ftopoly] } {
    set tf .fv.fTools.f
}

if { $tf != "" } {
    # create a frame:
    set f [frame $tf.ftopoly]

    if { [string first .tbw $tf] == 0 } {
	# calculate the row number below the last row:
	set row [expr [lindex [grid size $tf] 1] + 1]
	# now display the frame at calculated row, spanning the whole window:
	grid $f -row $row -column 0 -columnspan [lindex [grid size $tf] 0]\
	    -sticky we
    } else {
	# calculate the row number of the last row
	set row [expr [lindex [grid size $tf] 1] - 1]
	# calculate the number of columns in the last row
	set col [llength [grid slaves .fv.fTools.f -row $row]]
	# now display the frame at calculated row/column, spanning three cells
	grid $f -row $row -column $col -columnspan 3 -sticky we
    }

    # create a button inside the frame:
    button $f.b1 -width 10 -text "ToPolyMesh" -command { toPoly; uS; rV; }

    # display the button
    pack $f.b1 -side left -fill x -expand yes


    # cleanup
    unset f tf row col
}
# if
