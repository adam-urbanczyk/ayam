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

    addText $w.f1 e0 "Select Test:"

    set lb [listbox $w.f1.l1 -selectmode multiple]

    $lb insert end "Test 1 - Default Object Callbacks"
    $lb insert end "Test 2 - Solid Object Variations"
    $lb insert end "Test 3 - Object Variations"
    $lb insert end "Test 4 - Tool Object Variations"

    set f [frame $w.f2]
    button $f.bok -text "Ok" -width 5 -command "\
      aytest_runTests \[$w.f1.l1 curselection\];"

    button $f.bca -text "Cancel" -width 5 -command "focus .;destroy $w"

    pack $w.f1.l1 -side top -fill both -expand yes

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

puts $log "Testing object callbacks...\n"

lappend types Box Sphere Cylinder Cone Disk Hyperboloid Paraboloid Torus

lappend types NCurve ICurve ACurve NCircle

lappend types NPatch BPatch PatchMesh

lappend types Revolve Extrude Sweep Swing Skin Birail1 Birail2 Gordon

lappend types Cap Bevel ExtrNC ExtrNP OffsetNC ConcatNC Trim Text

lappend types Camera Light Material RiInc RiProc Script Select

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

    # missing tests: drawing, shading, bounding box calc, select points,
    # conversion/notification, RIB export

    # clean up
    catch {file delete $scratchfile}
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

puts $log "Testing object variations ...\n"

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
    prereqs {scalOb 0.5 0.5 0.5}
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
    prereqs {scalOb 2.0 2.0 2.0}
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

set types {}
lappend types Sphere Cylinder Disk Cone

foreach type $types {
    test_var $type
}
# foreach

}
# aytest_2


#
# Test 3
#
proc aytest_3 { } {

puts $log "Testing tool objects ...\n"

set types { Revolve Extrude Sweep Swing Skin Birail1 Birail2 Gordon }

lappend types Cap Bevel ExtrNC ExtrNP OffsetNC ConcatNC Trim ConcatNP

# Revolve Variation #1
array set Revolve_1 {
    postcmd {goDown -1;crtOb NCurve;forceNot}
    arr RevolveAttrData
    freevars {Closed ThetaMax}
    Closed {0 1}
    vars {Radius ZMin ZMax}
}
set Revolve_1(ThetaMax) $angles



foreach type $types {

    

    puts $log "Creating a $type ...\n"
    level_crt $type

}
# foreach
}
# aytest_3


#
# Test 4
#
proc aytest_4 { } {
# test modelling tools
puts $log "Testing modelling tools ...\n"
}
# aytest_4


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
	  if { [info exists ::${type}_${i}(prereqs)] } {
	      eval set prereq \$::${type}_${i}(prereqs)
	      eval $prereq
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
	      lappend body {pasOb;hSL;setProp;movOb $k $l $i;incr k 2}
	      eval $body
	  } else {
	      #
	      movOb $j $i 0.0
	  }
	  incr l 2
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

    foreach test $tests {
	set ::scratchfile [file join $ayprefs(TmpDir) aytestscratchfile.ay]

	set ::logfile [file join $ayprefs(TmpDir) aytestlog]
	set ::log [open $::logfile a]

	newScene
	selOb

	puts $test

	incr test
	catch [aytest_$test]

	close $::log
    }
    # foreach

 return;
}
# aytest_runTests


# start the GUI
aytest_selectGUI
