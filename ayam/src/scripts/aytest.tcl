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

set scratchfile /tmp/aytestscratchfile

set logfile /tmp/aytestlog
set log [open $logfile w]

newScene
selOb

#
# Test 1
#
puts $log "Testing object callbacks...\n"

set types { Box Sphere Cylinder Cone Disk Hyperboloid Paraboloid Torus }

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
close $log
return;

#
# Test 2
#

puts $log "Testing tool objects ...\n"

set types { Revolve Extrude Sweep Swing Skin Birail1 Birail2 Gordon }

lappend types Cap Bevel ExtrNC ExtrNP OffsetNC ConcatNC Trim


set Revolve_prereqs { NCurve }


foreach type $types {

    

    puts $log "Creating a $type ...\n"
    level_crt $type

}
# foreach


#
# Test 3
#
# test modelling tools
puts $log "Testing modelling tools ...\n"
