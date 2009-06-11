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

set scratchfile [file join $ayprefs(TmpDir) aytestscratchfile.ay]

set logfile [file join $ayprefs(TmpDir) aytestlog]
set log [open $logfile w]

newScene
selOb

#
# Test 1
#

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


#
# Test 2
#

puts $log "Testing object variations ...\n"


# standard angles to test for the ThetaMax
# parameter found in many objects
set angles {-360 -359 -271 -270 -269 -181 -180 -179 -91 -90 -89 -1 1 89 90 91 179 180 181 269 270 271 359 360}


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
    lappend val [expr [lindex $valset 1] * 2.0 ]
    lappend val [expr [lindex $valset 2] * 2.0 ]
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
    lappend val [expr [lindex $valset 1] * 0.5 ]
    lappend val [expr [lindex $valset 2] * 0.5 ]
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


#############
# Cone
#############

# Cone Variation #1
array set Cone_1 {
    arr ConeAttrData
    freevars {Closed ThetaMax}
    Closed {0 1}
    vars {Radius ZMin ZMax}
}
set Cone_1(ThetaMax) $angles


#############
# Disk
#############

# Disk Variation #1
array set Disk_1 {
    arr DiskAttrData
    freevars {Closed ThetaMax}
    Closed {0 1}
    vars {Radius ZMin ZMax}
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
    vars {Radius ZMin ZMax}
}
set Hyperboloid_1(ThetaMax) $angles



#############
# Paraboloid
#############

# Paraboloid Variation #1
array set Paraboloid_1 {
    arr ParaboloidAttrData
    freevars {Closed ThetaMax}
    Closed {0 1}
    vars {Radius ZMin ZMax}
}
set Paraboloid_1(ThetaMax) $angles



#############
# Torus
#############

# Torus Variation #1
array set Torus_1 {
    arr TorusAttrData
    freevars {Closed ThetaMax}
    Closed {0 1}
    vars {Radius ZMin ZMax}
}
set Torus_1(ThetaMax) $angles


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

set types {}
lappend types Sphere

foreach type $types {
    test_var $type
}
# foreach


# for now, we stop
close $log
return;

#
# Test 3
#

puts $log "Testing tool objects ...\n"

set types { Revolve Extrude Sweep Swing Skin Birail1 Birail2 Gordon }

lappend types Cap Bevel ExtrNC ExtrNP OffsetNC ConcatNC Trim ConcatNP


set Revolve_prereqs { NCurve }


foreach type $types {

    

    puts $log "Creating a $type ...\n"
    level_crt $type

}
# foreach


#
# Test 4
#
# test modelling tools
puts $log "Testing modelling tools ...\n"
