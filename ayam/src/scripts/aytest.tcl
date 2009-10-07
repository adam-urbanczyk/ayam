#
# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2009 by Randolf Schultz
# (randolf.schultz@gmail.com) and others.
#
# All rights reserved.
#
# See the file License for details.

# aytest.tcl - test Ayam

array set aytestprefs {
    KeepObjects 0
}

#
#
#
proc aytest_selectGUI { } {
    global ay

    winAutoFocusOff

    set w .testGUI
    catch {destroy $w}
    toplevel $w -class ayam
    wm title $w "Test Ayam"
    wm iconname $w "Ayam"
    if { $ay(ws) == "Aqua" } {
	winMakeFloat $w
    } else {
	wm transient $w .
    }

    set f [frame $w.f1]
    pack $f -in $w -side top -fill x

    set ay(bca) $w.f2.bca
    set ay(bok) $w.f2.bok

    # listbox for test selection
    addText $w.f1 e0 "Select Test:"

    set lb [listbox $w.f1.l1 -selectmode multiple]

    $lb insert end "Test 1 - Default Object Callbacks"
    $lb insert end "Test 2 - Valid Solid Object Variations"
    $lb insert end "Test 3 - Valid Tool Object Variations"
    $lb insert end "Test 4 - Modelling Tools"
    $lb insert end "Test 5 - All Solid Object Variations"

    # preferences
    set f [frame $w.fp]
    addCheck $f aytestprefs KeepObjects


    set f [frame $w.f2]

    button $f.bok -text "Ok" -width 5 -command "\
      aytest_runTests \[$w.f1.l1 curselection\];"

    button $f.bca -text "Cancel" -width 5 -command "focus .;destroy $w"

    pack $w.f1.l1 -side top -fill both -expand yes

    pack $w.fp -side top -fill x -expand yes

    pack $f.bok $f.bca -in $f -side left -fill x -expand yes
    pack $f -in $w -side bottom -fill x

    # Esc-key && close via window decoration == Cancel button
    bind $w <Escape> "$f.bca invoke"
    wm protocol $w WM_DELETE_WINDOW "$f.bca invoke"

    winCenter $w
    grab $w
    focus $w.f1.l1
    tkwait window $w

    winAutoFocusOn

 return;
}
# aytest_selectGUI


#
# Test 1
#
proc aytest_1 { } {
uplevel #0 {

puts $log "Testing default object callbacks...\n"

set types ""

lappend types Box Sphere Cylinder Cone Disk Hyperboloid Paraboloid Torus

lappend types NCurve ICurve ACurve NCircle

lappend types NPatch BPatch PatchMesh

lappend types Revolve Extrude Sweep Swing Skin Birail1 Birail2 Gordon

lappend types Cap Bevel ExtrNC ExtrNP OffsetNC ConcatNC Trim Text

lappend types Camera Light Material RiInc RiProc Script Select

set view1 ""
if { [winfo exists .fv.fViews.fview1.f3D.togl] } {
    set view1 .fv.fViews.fview1.f3D.togl
}
set view2 ""
if { [winfo exists .fv.fViews.fview2.f3D.togl] } {
    set view2 .fv.fViews.fview2.f3D.togl
}


foreach type $types {

    puts $log "Creating a $type ...\n"
    crtOb $type
    hSL

    puts $log "Copying a $type ...\n"
    copOb
    pasOb
    hSL

    puts $log "Deleting a $type ...\n"
    delOb
    hSL

    puts $log "Get properties of $type ...\n"
    getProp

    puts $log "Set properties of $type ...\n"
    setProp

    puts $log "Saving a $type ...\n"
    saveScene $scratchfile 1

    puts $log "Reading a $type ...\n"
    insertScene $scratchfile
    catch {file delete $scratchfile}

    puts $log "Exporting a $type ...\n"
    wrib $scratchfile -selonly
    catch {file delete $scratchfile}

    if { [winfo exists $view1] } {
	puts $log "Drawing a $type ...\n"
	$view1 mc
	$view1 redraw
    }

    if { [winfo exists $view2] } {
	puts $log "Shading a $type ...\n"
	$view2 mc
	$view2 redraw
    }

    if { [winfo exists $view1] } {
	puts $log "Bounding box calculation of $type ...\n"
	$view1 mc
	$view1 zoomob
    }

    puts $log "Notifying a $type ...\n"
    forceNot

    puts $log "Selecting points of $type ...\n"
    selPnts -all
    selPnts
    selPnts 0 2
    selPnts

    puts $log "Converting a $type ...\n"
    convOb
    convOb -inplace

    # missing tests: select points via action callback,
    # draw points/handles, (comparison - AI?)

    selOb
    goTop
}
# foreach

}
}
# aytest_1


#
# Test 2
#
proc aytest_2 { } {
uplevel #0 {

puts $log "Testing solid object variations ...\n"

# standard angles to test for the ThetaMax
# parameter found in many objects
set angles {-360 -359 -271 -270 -269 -181 -180 -179 -91 -90 -89 -1 1 89 90 91 179 180 181 269 270 271 359 360}


#############
# Sphere
#############

# Sphere Variation #1
array set Sphere_1 {
    arr SphereAttrData
    freevars {Closed ThetaMax}
    Closed {0 1}
    vars {Radius ZMin ZMax}
}
set Sphere_1(ThetaMax) $angles

lappend Sphere_1(vals) { 1.0 -1.0 1.0 }
lappend Sphere_1(vals) { 1.0 -0.1 0.1 }
lappend Sphere_1(vals) { 1.0 -0.2 0.2 }
lappend Sphere_1(vals) { 1.0 -0.5 0.5 }

lappend Sphere_1(vals) { 1.0 -0.5 1.0 }
lappend Sphere_1(vals) { 1.0 0.0 1.0 }
lappend Sphere_1(vals) { 1.0 0.5 1.0 }
lappend Sphere_1(vals) { 1.0 -1.0 -0.5 }
lappend Sphere_1(vals) { 1.0 -1.0 0.0 }
lappend Sphere_1(vals) { 1.0 -1.0 0.5 }


# Sphere Variation #2
array set Sphere_2 {
    arr SphereAttrData
    precmd {scalOb 0.5 0.5 0.5}
    freevars {Closed ThetaMax}
    Closed {0 1}
    vars {Radius ZMin ZMax}
}
set Sphere_2(ThetaMax) $angles

# take all valsets from Sphere_1 but adapt the radius
foreach valset $Sphere_1(vals) {
    set val {}
    lappend val [expr [lindex $valset 0] * 2.0 ]
    lappend val [lindex $valset 1]
    lappend val [lindex $valset 2]
    lappend Sphere_2(vals) $val
}
# foreach


# Sphere Variation #3
array set Sphere_3 {
    arr SphereAttrData
    precmd {scalOb 2.0 2.0 2.0}
    freevars {Closed ThetaMax}
    Closed {0 1}
    vars {Radius ZMin ZMax}
}
set Sphere_3(ThetaMax) $angles

# take all valsets from Sphere_1 but adapt the radius
foreach valset $Sphere_1(vals) {
    set val {}
    lappend val [expr [lindex $valset 0] * 0.5 ]
    lappend val [lindex $valset 1]
    lappend val [lindex $valset 2]
    lappend Sphere_3(vals) $val
}
# foreach


#############
# Cylinder
#############

# Cylinder Variation #1
array set Cylinder_1 {
    arr CylinderAttrData
    freevars {Closed ThetaMax}
    Closed {0 1}
    vars {Radius ZMin ZMax}
}
set Cylinder_1(ThetaMax) $angles

lappend Cylinder_1(vals) { 1.0 -1.0 1.0 }
lappend Cylinder_1(vals) { 1.0 -0.1 0.1 }
lappend Cylinder_1(vals) { 1.0 -0.5 0.5 }
lappend Cylinder_1(vals) { 1.0 -2.0 2.0 }
lappend Cylinder_1(vals) { 1.0 0.0 1.0 }
lappend Cylinder_1(vals) { 1.0 -1.0 0.0 }
lappend Cylinder_1(vals) { 1.0 -2.0 -1.0 }
lappend Cylinder_1(vals) { 1.0 1.0 2.0 }
lappend Cylinder_1(vals) { 1.0 -2.1 -0.1 }
lappend Cylinder_1(vals) { 1.0 0.1 2.1 }


# Cylinder Variation #2
array set Cylinder_2 {
    arr CylinderAttrData
    freevars {Closed ThetaMax}
    Closed {0 1}
    vars {Radius ZMin ZMax}
}
set Cylinder_2(ThetaMax) $angles

# take all valsets from Cylinder_1 but adapt the radius
foreach valset $Cylinder_1(vals) {
    set val {}
    lappend val [expr [lindex $valset 0] * 0.5 ]
    lappend val [lindex $valset 1]
    lappend val [lindex $valset 2]
    lappend Cylinder_2(vals) $val
}
# foreach


# Cylinder Variation #3
array set Cylinder_3 {
    arr CylinderAttrData
    freevars {Closed ThetaMax}
    Closed {0 1}
    vars {Radius ZMin ZMax}
}
set Cylinder_3(ThetaMax) $angles

# take all valsets from Cylinder_1 but adapt the radius
foreach valset $Cylinder_1(vals) {
    set val {}
    lappend val [expr [lindex $valset 0] * 2.0 ]
    lappend val [lindex $valset 1]
    lappend val [lindex $valset 2]
    lappend Cylinder_3(vals) $val
}
# foreach



#############
# Cone
#############

# Cone Variation #1
array set Cone_1 {
    arr ConeAttrData
    freevars {Closed ThetaMax Radius Height}
    Closed {0 1}
    Radius {0.1 0.5 1.0 1.5 2.0}
    Height {-2.0 -1.5 -1.0 -0.5 -0.1 0.1 0.5 1.0 1.5 2.0}
    vars {}
    vals {1}
}
set Cone_1(ThetaMax) $angles



#############
# Disk
#############

# Disk Variation #1
array set Disk_1 {
    arr DiskAttrData
    freevars {Radius Height ThetaMax}
    Radius {0.1 0.5 1.0 1.5 2.0}
    Height {-2.0 -1.5 -1.0 -0.5 -0.1 0.1 0.5 1.0 1.5 2.0}
    vars {}
    vals {1}
}
set Disk_1(ThetaMax) $angles


#############
# Hyperboloid
#############

# Hyperboloid Variation #1
array set Hyperboloid_1 {
    arr HyperboloidAttrData
    freevars {Closed ThetaMax}
    Closed {0 1}
    vars {P1_X P1_Y P1_Z P2_X P2_Y P2_Z}
}
set Hyperboloid_1(ThetaMax) $angles

# a hyperboloid as you know it
lappend Hyperboloid_1(vals) { 0.0 -1.0 -0.5  1.0 0.0 0.5 }
lappend Hyperboloid_1(vals) { 1.0 0.0 0.5  0.0 -1.0 -0.5 }

# cylinder
lappend Hyperboloid_1(vals) { 0.0 1.0 -0.5  0.0 1.0 0.5 }
lappend Hyperboloid_1(vals) { 0.0 1.0 0.5  0.0 1.0 -0.5 }
lappend Hyperboloid_1(vals) { 1.0 0.0 -0.5  1.0 0.0 0.5 }
lappend Hyperboloid_1(vals) { 1.0 0.0 0.5  1.0 0.0 -0.5 }

# ring (disk with hole)
lappend Hyperboloid_1(vals) { 0.5 0.0 0.0  1.0 0.0 0.0 }

# frustum (cone without apex)
lappend Hyperboloid_1(vals) { 0.0 1.0 -0.5  0.0 0.5 0.5 }



#############
# Paraboloid
#############

# Paraboloid Variation #1
array set Paraboloid_1 {
    arr ParaboloidAttrData
    freevars {Closed ThetaMax}
    Closed {0 1}
    vars {RMax ZMin ZMax}
}
set Paraboloid_1(ThetaMax) $angles


lappend Paraboloid_1(vals) { 1.0 0.0 0.5 }
lappend Paraboloid_1(vals) { 1.0 0.0 1.0 }
lappend Paraboloid_1(vals) { 1.0 0.0 1.5 }
lappend Paraboloid_1(vals) { 1.0 0.0 2.0 }

lappend Paraboloid_1(vals) { 1.0 0.1 0.5 }
lappend Paraboloid_1(vals) { 1.0 0.1 1.0 }
lappend Paraboloid_1(vals) { 1.0 0.1 1.5 }
lappend Paraboloid_1(vals) { 1.0 0.1 2.0 }

lappend Paraboloid_1(vals) { 1.0 1.0 0.1 }
lappend Paraboloid_1(vals) { 1.0 1.0 0.5 }
lappend Paraboloid_1(vals) { 1.0 1.0 0.9 }

lappend Paraboloid_1(vals) { 1.0 2.0 0.1 }
lappend Paraboloid_1(vals) { 1.0 2.0 0.5 }
lappend Paraboloid_1(vals) { 1.0 2.0 0.9 }


# Paraboloid Variation #2
array set Paraboloid_2 {
    arr ParaboloidAttrData
    freevars {Closed ThetaMax}
    Closed {0 1}
    vars {RMax ZMin ZMax}
}
set Paraboloid_2(ThetaMax) $angles


# take all valsets from Paraboloid_1 but adapt RMax
foreach valset $Paraboloid_1(vals) {
    set val {}
    lappend val [expr [lindex $valset 0] * 0.5 ]
    lappend val [lindex $valset 1]
    lappend val [lindex $valset 2]
    lappend Paraboloid_2(vals) $val
}
# foreach


# Paraboloid Variation #3
array set Paraboloid_3 {
    arr ParaboloidAttrData
    freevars {Closed ThetaMax}
    Closed {0 1}
    vars {RMax ZMin ZMax}
}
set Paraboloid_3(ThetaMax) $angles

# take all valsets from Paraboloid_1 but adapt RMax
foreach valset $Paraboloid_1(vals) {
    set val {}
    lappend val [expr [lindex $valset 0] * 2.0 ]
    lappend val [lindex $valset 1]
    lappend val [lindex $valset 2]
    lappend Paraboloid_3(vals) $val
}
# foreach


#############
# Torus
#############

# Torus Variation #1
array set Torus_1 {
    arr TorusAttrData
    freevars {Closed ThetaMax}
    Closed {0 1}
    vars {MajorRad MinorRad PhiMin PhiMax}
}
set Torus_1(ThetaMax) $angles

lappend Torus_1(vals) { 0.25 0.25 0.0 360.0 }
lappend Torus_1(vals) { 0.75 0.25 0.0 360.0 }
lappend Torus_1(vals) { 1.0 0.25 0.0 360.0 }
lappend Torus_1(vals) { 1.0 0.5 0.0 360.0 }

lappend Torus_1(vals) { 0.25 0.25 90.0 360.0 }
lappend Torus_1(vals) { 0.75 0.25 90.0 360.0 }
lappend Torus_1(vals) { 1.0 0.25 90.0 360.0 }
lappend Torus_1(vals) { 1.0 0.5 90.0 360.0 }

lappend Torus_1(vals) { 0.25 0.25 180.0 360.0 }
lappend Torus_1(vals) { 0.75 0.25 180.0 360.0 }
lappend Torus_1(vals) { 1.0 0.25 180.0 360.0 }
lappend Torus_1(vals) { 1.0 0.5 180.0 360.0 }


# XXXX TODO: add torus variations for phimin/phimax


set types {}
lappend types Sphere Cylinder Disk Cone

foreach type $types {
    puts $log "Testing $type ...\n"
    test_var $type
}
# foreach

}
}
# aytest_2


#
# Test 3
#
proc aytest_3 { } {
uplevel #0 {

puts $log "Testing tool objects ...\n"

# standard angles to test for the ThetaMax
# parameter found in many objects
set angles {-360 -359 -271 -270 -269 -181 -180 -179 -91 -90 -89 -1 1 89 90 91 179 180 181 269 270 271 359 360}


# Revolve Variation #1
array set Revolve_1 {
    precmd {
	goDown -1;
	switch $l {
	    0 {crtOb NCurve}
	    1 {crtOb NCurve -length 2}
	    2 {crtOb NCurve -length 3}
	    3 {
		crtOb NCurve; hSL;
		getProp; set ::NCurveAttrData(Knot-Type) 1; setProp
	    }
	    4 {
		crtClosedBS 4; hSL; movOb 1.25 0 0;
	    }
	};
	forceNot;
	goUp;
	hSL
    }
    arr RevolveAttrData
    freevars {ThetaMax}
    vars {Sections Order}
}
set Revolve_1(ThetaMax) $angles

# repeating the same value set leads to multiple calls
# of the precmd, thus enabling it to check the value of
# <l> (set by test_var) and creating a different parameter
# curve for each iteration
lappend Revolve_1(vals) { 0 0 }
lappend Revolve_1(vals) { 0 0 }
lappend Revolve_1(vals) { 0 0 }
lappend Revolve_1(vals) { 0 0 }
lappend Revolve_1(vals) { 0 0 }


# Revolve Variation #2
array set Revolve_2 {
    precmd {goDown -1;crtOb NCurve;hSL;movOb 1 0 0;rotOb 0 0 90;
	forceNot;goUp;hSL}
    arr RevolveAttrData
    freevars {UpperCap LowerCap StartCap EndCap ThetaMax}
    UpperCap {0 1}
    LowerCap {0 1}
    StartCap {0 1}
    EndCap {0 1}
    vars {Sections Order}
}
set Revolve_2(ThetaMax) $angles

lappend Revolve_2(vals) { 0 0 }


# Extrude Variation #1
array set Extrude_1 {
    precmd {
	goDown -1;
	switch $l {
	    0 {crtOb NCurve}
	    1 {crtOb NCurve -length 2}
	    2 {crtOb NCurve -length 3}
	    3 {
		crtOb NCurve; hSL;
		getProp; set ::NCurveAttrData(Knot-Type) 1; setProp
	    }
	    4 {
		crtClosedBS 4; hSL; movOb 1.25 0 0;
	    }
	    5 {	crtOb NCircle }
	    6 {
		crtOb NCircle; hSL; movOb 1.25 0 0;
	    }
	};
	forceNot;
	goUp;
	hSL
    }
    arr ExtrudeAttrData
    vars {Height}
}

lappend Extrude_1(vals) { 1.0 }
lappend Extrude_1(vals) { 1.0 }
lappend Extrude_1(vals) { 1.0 }
lappend Extrude_1(vals) { 1.0 }
lappend Extrude_1(vals) { 1.0 }
lappend Extrude_1(vals) { 1.0 }
lappend Extrude_1(vals) { 1.0 }


# Extrude Variation #2
array set Extrude_2 {
    precmd {goDown -1;crtClosedBS 4;hSL movOb 1.25 0.0 0.0;forceNot;goUp;hSL}
    arr ExtrudeAttrData
    freevars {StartCap EndCap}
    StartCap {0 1}
    EndCap {0 1}
    vars {Height}
}

lappend Extrude_2(vals) { 0.1 }
lappend Extrude_2(vals) { 0.5 }
lappend Extrude_2(vals) { 1.0 }
lappend Extrude_2(vals) { 2.0 }
lappend Extrude_2(vals) { 10.0 }


# Extrude Variation #3
array set Extrude_3 {
    precmd {goDown -1;crtOb NCircle;hSL movOb 1.25 0.0 0.0;forceNot;goUp;hSL}
    arr ExtrudeAttrData
    freevars {StartCap EndCap}
    StartCap {0 1}
    EndCap {0 1}
    vars {Height}
}

lappend Extrude_3(vals) { 0.1 }
lappend Extrude_3(vals) { 0.5 }
lappend Extrude_3(vals) { 1.0 }
lappend Extrude_3(vals) { 2.0 }
lappend Extrude_3(vals) { 10.0 }


# XXXX TODO: add bevels, add holes



# Sweep Variation #1
array set Sweep_1 {
    precmd {
	goDown -1;
	crtOb NCurve; hSL; rotOb 0 0 90 ;
	crtOb NCurve; forceNot; goUp; hSL
    }
    arr SweepAttrData
    freevars {Rotate Interpolate}
    Rotate {0 1}
    Interpolate {0 1}
    vars { dummy }
    vals { {0} }
}


# Sweep Variation #2
array set Sweep_2 {
    precmd {
	goDown -1;
	crtClosedBS 4; hSL; rotOb 0 90 0;
	crtOb NCurve; forceNot; goUp; hSL
    }
    arr SweepAttrData
    freevars {Rotate Interpolate StartCap EndCap}
    Rotate {0 1}
    Interpolate {0 1}
    StartCap {0 1}
    EndCap {0 1}
    vars { dummy }
    vals { {0} }
}


# Sweep Variation #3
array set Sweep_3 {
    precmd {
	goDown -1;
	crtOb NCircle; hSL; rotOb 0 90 0;
	crtOb NCurve; forceNot; goUp; hSL
    }
    arr SweepAttrData
    freevars {Rotate Interpolate StartCap EndCap}
    Rotate {0 1}
    Interpolate {0 1}
    StartCap {0 1}
    EndCap {0 1}
    vars { dummy }
    vals { {0} }
}

# XXXX TODO: add bevels





# Birail1 Variation #1
array set Birail1_1 {
    precmd {
	goDown -1;
	crtOb NCurve; hSL; rotOb 0 0 -90;
	crtOb NCurve; crtOb NCurve; hSL; movOb 0 0.75 0;
	forceNot; goUp; hSL
    }
    arr Birail1AttrData
    vars { dummy }
    vals { {0} }
}




# Birail2 Variation #1
array set Birail2_1 {
    precmd {
	goDown -1;
	crtOb NCurve; hSL; rotOb 0 0 -90;
	crtOb NCurve; crtOb NCurve; hSL; movOb 0 0.75 0;
	crtOb NCurve; hSL; rotOb 0 0 -90; movOb 0.75 0 0;
	forceNot; goUp; hSL
    }
    arr Birail2AttrData
    vars { dummy }
    vals { {0} }
}




# Gordon Variation #1
array set Gordon_1 {
    precmd {
	goDown -1;
	crtOb NCurve; hSL; rotOb 0 0 90 ;
	crtOb NCurve; forceNot; goUp; hSL
    }
    arr GordonAttrData
    vars { dummy }
    vals { {0} }
}



# Skin Variation #1
array set Skin_1 {
    precmd {
	goDown -1;
	crtOb NCurve; hSL; movOb 0 1 0;
	crtOb NCurve; forceNot; goUp; hSL
    }
    arr SkinAttrData
    vars { dummy }
    vals { {0} }
}

# Skin Variation #2
array set Skin_2 {
    precmd {
	goDown -1;
	crtClosedBS 4; hSL; movOb 0 1 1;
	crtClosedBS 4; forceNot; goUp; hSL
    }
    arr SkinAttrData
    freevars {StartCap EndCap}
    Interpolate {0 1}
    StartCap {0 1}
    EndCap {0 1}
    vars { dummy }
    vals { {0} }
}

# Skin Variation #3
array set Skin_3 {
    precmd {
	goDown -1;
	crtClosedBS 4; hSL; movOb 0 4 4;
	crtClosedBS 4; hSL; movOb 0 3 3;
	crtClosedBS 4; hSL; movOb 0 1 2;
	crtClosedBS 4; hSL; movOb 0 1 0;
	crtClosedBS 4; forceNot; goUp; hSL
    }
    arr SkinAttrData
    freevars {Interpolate StartCap EndCap Knot-Type_U Order_U}
    Interpolate {0 1}
    StartCap {0 1}
    EndCap {0 1}
    Knot-Type_U { 1 2 3 }
    Order_U { 2 3 4 5 }
    vars { dummy }
    vals { 0 }
}

# Skin Variation #4
array set Skin_4 {
    precmd {
	goDown -1;
	switch $l {
	    0 {
		crtClosedBS 4; hSL; movOb 0 4 4;
		crtClosedBS 4; hSL; movOb 0 3 3;
		crtClosedBS 4; hSL; movOb 0 1 2;
		crtClosedBS 4; hSL; movOb 0 1 0;
		crtClosedBS 4;
	    }
	    1 {
		crtClosedBS 4; hSL; movOb 0 4 4;
		crtClosedBS 4; hSL; movOb 0 3 3;
		crtClosedBS 4; hSL; movOb 0 1 0;
		crtClosedBS 4;
	    }
	    2 {
		crtClosedBS 4; hSL; movOb 0 4 4;
		crtClosedBS 4; hSL; movOb 0 1 0;
		crtClosedBS 4;
	    }
	}
	forceNot; goUp; hSL
    }
    arr SkinAttrData
    freevars {StartCap EndCap}
    StartCap {0 1}
    EndCap {0 1}
    vars { Knot-Type_U Order_U }
}

lappend Skin_4(vals) {0 5}
lappend Skin_4(vals) {0 4}
lappend Skin_4(vals) {0 3}


# Skin Variation #5
array set Skin_5 {
    precmd {
	goDown -1;
	switch $l {
	    0 {
		crtClosedBS 4; hSL; movOb 0 3 3;
		crtClosedBS 5; hSL; movOb 0 1.5 0;
		crtClosedBS 4;
	    }
	    1 {
		crtClosedBS 4; hSL; movOb 0 3 3;
		crtOb NCurve; hSL; movOb 0 1.5 0;
		crtClosedBS 4;
	    }
	    2 {
		crtClosedBS 4; hSL; movOb 0 3 3;
		crtOb NCircle; hSL; movOb 0 1.5 0;
		crtClosedBS 4;
	    }
	    3 {
		crtOb NCircle; hSL; movOb 0 3 3; rotOb 45 0 0;
		crtOb NCircle; hSL; movOb 0 1.5 0;
		crtClosedBS 4;hSL; rotOb -45 0 0;
	    }
	}
	forceNot; goUp; hSL
    }
    arr SkinAttrData
    freevars {Interpolate StartCap EndCap Knot-Type_U Order_U}
    Interpolate {0 1}
    StartCap {0 1}
    EndCap {0 1}
    Knot-Type_U { 1 2 3 }
    Order_U { 2 3 4 5 }
    vars { dummy }

}

lappend Skin_5(vals) { 0 }
lappend Skin_5(vals) { 0 }
lappend Skin_5(vals) { 0 }
lappend Skin_5(vals) { 0 }


# XXXX TODO: add bevels




# Cap Variation #1
array set Cap_1 {
    precmd {
	goDown -1;
	switch $l {
	    0 {	crtClosedBS 4; }
	    1 {	crtOb NCircle }
	    2 {
		crtClosedBS 4; hSL; movOb 1.25 0 0;
	    }
	    3 {
		crtOb NCircle; hSL; movOb 1.25 0 0;
	    }
	};
	forceNot;
	goUp;
	hSL
    }
    arr CapAttrData
    vars {dummy}
}

lappend Cap_1(vals) { 0 }
lappend Cap_1(vals) { 0 }
lappend Cap_1(vals) { 0 }
lappend Cap_1(vals) { 0 }


# Bevel Variation #1
array set Bevel_1 {
    precmd {
	goDown -1;
	switch $l {
	    0 {	crtClosedBS 4; }
	    1 {	crtOb NCircle }
	    2 {
		crtClosedBS 4; hSL; movOb 1.25 0 0;
	    }
	    3 {
		crtOb NCircle; hSL; movOb 1.25 0 0;
	    }
	};
	forceNot;
	goUp;
	hSL
    }
    arr BevelAttrData
    vars {dummy}
}

lappend Bevel_1(vals) { 0 }
lappend Bevel_1(vals) { 0 }
lappend Bevel_1(vals) { 0 }
lappend Bevel_1(vals) { 0 }



# ExtrNC Variation #1
array set ExtrNC_1 {
    precmd {
	goDown -1;
	crtOb NPatch;
	forceNot;
	goUp;
	hSL
    }
    arr ExtrNCAttrData
    vars {dummy}
}

lappend ExtrNC_1(vals) { 0 }
lappend ExtrNC_1(vals) { 0 }
lappend ExtrNC_1(vals) { 0 }
lappend ExtrNC_1(vals) { 0 }



# ExtrNP Variation #1
array set ExtrNP_1 {
    precmd {
	goDown -1;
	crtOb NPatch;
	forceNot;
	goUp;
	hSL
    }
    arr ExtrNPAttrData
    vars { UMin UMax VMin VMax }
}

lappend ExtrNP_1(vals) { 0.1 0.9 0.1 0.9 }
lappend ExtrNP_1(vals) { 0.5 1.0 0.1 0.9 }
lappend ExtrNP_1(vals) { 0.5 1.0 0.5 1.0 }



# OffsetNC Variation #1
array set OffsetNC_1 {
    precmd {
	goDown -1;
	switch $l {
	    0 {crtOb NCurve}
	    1 {crtOb NCurve -length 2}
	    2 {crtOb NCurve -length 3}
	    3 {
		crtOb NCurve; hSL;
		getProp; set ::NCurveAttrData(Knot-Type) 1; setProp
	    }
	    4 {
		crtClosedBS 4; hSL; movOb 1.25 0 0;
	    }
	    5 {	crtOb NCircle }
	    6 {
		crtOb NCircle; hSL; movOb 1.25 0 0;
	    }
	};
	forceNot;
	goUp;
	hSL
    }
    freevars { Mode Revert }
    Mode { 0 1 }
    Revert { 0 1 }
    arr OffsetNCAttrData
    vars {Offset}
}

lappend OffsetNC_1(vals) { 0.1 }
lappend OffsetNC_1(vals) { 0.1 }
lappend OffsetNC_1(vals) { 0.1 }
lappend OffsetNC_1(vals) { 0.1 }
lappend OffsetNC_1(vals) { 0.1 }
lappend OffsetNC_1(vals) { 0.1 }
lappend OffsetNC_1(vals) { 0.1 }



# ConcatNC Variation #1
array set ConcatNC_1 {
    precmd {
	goDown -1;
	crtOb NCurve; crtOb NCurve; hSL; movOb 1 0 0;
	forceNot; goUp; hSL
    }
    arr ConcatNCAttrData
    vars { dummy }
    vals { {0} }
}


# Trim Variation #1
array set Trim_1 {
    precmd {
	goDown -1;
	crtOb NPatch; crtOb NCircle; hSL; scalOb 0.5 0.5 1.0; movOb 0.5 0.5 0;
	forceNot; goUp; hSL
    }
    arr TrimAttrData
    vars { dummy }
    vals { {0} }
}


# ConcatNP Variation #1
array set ConcatNP_1 {
    precmd {
	goDown -1;
	crtOb NPatch; crtOb NPatch; hSL; movOb 1 0 0;
	forceNot; goUp; hSL
    }
    arr ConcatNPAttrData
    vars { dummy }
    vals { {0} }
}




set types { Revolve Extrude Sweep Swing Skin Birail1 Birail2 Gordon }

lappend types Cap Bevel ExtrNC ExtrNP OffsetNC ConcatNC Trim ConcatNP

set types {Revolve Skin Sweep Swing Skin Birail1 Birail2 Gordon}

puts -nonewline "Testing "
foreach type $types {

    puts $log "Testing $type ...\n"

    puts -nonewline "${type}, "

    test_var $type

}
# foreach
puts -nonewline "\n"
}
}
# aytest_3


#
# Test 4
#
proc aytest_4 { } {
uplevel #0 {

# test modelling tools
puts $log "Testing modelling tools ...\n"

}
}
# aytest_4


#
# Test 5
#
proc aytest_5 { } {
uplevel #0 {

puts $log "Testing all Solid Variations (Errors expected!) ...\n"

# XXXX add NaN, Inf?
set angles {-360 -359 -271 -270 -269 -181 -180 -179 -91 -90 -89 -1 -0.1 -0 0 0.1 1 89 90 91 179 180 181 269 270 271 359 360}
set floatvals {-1000 -100 -20 -2.5 -2 -1.5 -1.0 -0.9 -0.1 -0 0 0.1 0.9 1.0 1.5 2 2.5 20 100 1000}



#############
# Sphere
#############

array set Sphere_1 {
    arr SphereAttrData
    freevars {Closed ThetaMax Radius ZMin ZMax}
    Closed {0 1}
    vars {dummy}
    vals { {0} }
}
set Sphere_1(ThetaMax) $angles
set Sphere_1(Radius) $floatvals
set Sphere_1(ZMin) $floatvals
set Sphere_1(ZMax) $floatvals


#############
# Cylinder
#############

array set Cylinder_1 {
    arr CylinderAttrData
    freevars {Closed ThetaMax Radius ZMin ZMax}
    Closed {0 1}
    vars {dummy}
    vals { {0} }
}

set Cylinder_1(ThetaMax) $angles
set Cylinder_1(Radius) $floatvals
set Cylinder_1(ZMin) $floatvals
set Cylinder_1(ZMax) $floatvals


#############
# Cone
#############

# Cone Variation #1
array set Cone_1 {
    arr ConeAttrData
    freevars {Closed ThetaMax Radius Height}
    Closed {0 1}
    vars {dummy}
    vals { {0} }
}
set Cone_1(ThetaMax) $angles
set Cone_1(Radius) $floatvals
set Cone_1(Height) $floatvals


#############
# Disk
#############

array set Disk_1 {
    arr DiskAttrData
    freevars {Radius Height ThetaMax}

    vars {dummy}
    vals { {0} }
}
set Disk_1(ThetaMax) $angles
set Disk_1(Radius) $floatvals
set Disk_1(Height) $floatvals


#############
# Hyperboloid
#############

array set Hyperboloid_1 {
    arr HyperboloidAttrData
    freevars {Closed ThetaMax P1_X P1_Y P1_Z P2_X P2_Y P2_Z}
    Closed {0 1}
    vars {dummy}
    vals { {0} }
}
set Hyperboloid_1(ThetaMax) $angles
set Hyperboloid_1(P1_X) $floatvals
set Hyperboloid_1(P1_Y) $floatvals
set Hyperboloid_1(P1_Z) $floatvals
set Hyperboloid_1(P2_X) $floatvals
set Hyperboloid_1(P2_Y) $floatvals
set Hyperboloid_1(P2_Z) $floatvals


#############
# Paraboloid
#############

array set Paraboloid_1 {
    arr ParaboloidAttrData
    freevars {Closed ThetaMax RMax ZMin ZMax}
    Closed {0 1}
    vars {dummy}
    vals { {0} }
}
set Paraboloid_1(ThetaMax) $angles
set Paraboloid_1(RMax) $floatvals
set Paraboloid_1(ZMin) $floatvals
set Paraboloid_1(ZMax) $floatvals


#############
# Torus
#############

array set Torus_1 {
    arr TorusAttrData
    freevars {Closed ThetaMax MajorRad MinorRad PhiMin PhiMax}
    Closed {0 1}
    vars {dummy}
    vals { {0} }
}
set Torus_1(ThetaMax) $angles
set Torus_1(MajorRad) $floatvals
set Torus_1(MinorRad) $floatvals
set Torus_1(PhiMin) $angles
set Torus_1(PhiMax) $angles


set types { Sphere Cylinder Cone Disk Hyperboloid Paraboloid Torus }

foreach type $types {

    puts $log "Testing $type ...\n"
    puts "Testing $type ..."
    test_var $type

}
# foreach


}
}
# aytest_5


# forall:
#  taken from http://wiki.tcl.tk/2546
#  (Cartesian product of a list of lists)
#  original author: Eric Boudaillier;
#  create cartesian product of a list of lists,
#  run cmd for every combination, example:
#  forall var1 list1 var2 list2 [varn listn] cmd
#  forall a {1 2 3} b {3 4} {puts "$a $b"}
#  =>
#  1 3
#  1 4
#  2 3
#  2 4
#  3 3...
proc forall {args} {
    if {[llength $args] < 3 || [llength $args] % 2 == 0} {
	return -code error "wrong \# args: should be \"forall varList list ?varList list ...? body\""
    }
    set body [lindex $args end]
    set args [lrange $args 0 end-1]
    while {[llength $args]} {
	set varName [lindex $args end-1]
	set list    [lindex $args end]
	set args    [lrange $args 0 end-2]
	set body    [list foreach $varName $list $body]
    }
    uplevel 1 $body
}
# forall


# test_var:
#  test object variations
#  ToDo: make final body command configurable
proc test_var { type } {

# Every object variation array contains the following components:
#  precmd - commands to run after object creation but before variation
#  postcmd - commands to run
#  arr - array to put all variable data into
#  vars - list of variable names in array arr
#  vals - list of value sets for all variables in vars
#  freevars - list of free variables (their values will be automatically
#             varied by a cartesian product!)
# for every free variable, another entry (named like the variable itself)
# exists, that contains the valid variable values

  set i 1
  while {[info exists ::${type}_$i]} {
      eval set arr \$::${type}_${i}(arr)
      eval set vars \$::${type}_${i}(vars)
      eval set vals \$::${type}_${i}(vals)
      set l 0
      foreach valset $vals {

	  crtOb $type
	  hSL

	  # execute prerequisites
	  if { [info exists ::${type}_${i}(precmd)] } {
	      eval set precmd \$::${type}_${i}(precmd)
	      eval $precmd
	  }

	  # set un-free variables from the valset
	  getProp
	  set j 0
	  while { $j < [llength $vars] } {
	      set ::${arr}([lindex $vars $j]) [lindex $valset $j]
	      incr j
	  }
	  setProp

	  # set free variables
	  if { [info exists ::${type}_${i}(freevars)] } {
	      copOb
	      set body "forall "
	      eval set freevars \$::${type}_${i}(freevars)
	      foreach freevar $freevars {
		  append body ::${arr}($freevar)
		  append body " "
		  eval set freevals \$::${type}_${i}($freevar)
		  lappend body $freevals
		  append body " "
	      }
	      set k 0
	      lappend body {
		  pasOb;hSL;
		  setProp;
		  movOb $k $l $i;

		  if { ! $::aytestprefs(KeepObjects) } {
		      delOb
		  }
		  incr k 2
	      }
	      eval $body
	  } else {
	      #
	      movOb $j $i 0.0;

	      if { ! $::aytestprefs(KeepObjects) } {
		  delOb
	      }
	  }
	  incr l
      }
      # foreach
      rV
      incr i
  }
  # while

 return;
}
# test_var


#
#
#
proc aytest_runTests { tests } {
    global ayprefs

    . configure -cursor watch
    .testGUI configure -cursor watch
    update

    foreach test $tests {
	set ::scratchfile [file join $ayprefs(TmpDir) aytestscratchfile.ay]

	set ::logfile [file join $ayprefs(TmpDir) aytestlog]
	set ::log [open $::logfile a]

	newScene
	selOb

	incr test

	puts "Running Test $test..."

	catch [aytest_$test]

	close $::log

	puts "Finished Test $test..."
    }
    # foreach

    . configure -cursor {}
    .testGUI configure -cursor {}
    update

 return;
}
# aytest_runTests


# start the GUI
aytest_selectGUI
