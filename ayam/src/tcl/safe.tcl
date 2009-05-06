# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2009 by Randolf Schultz
# (randolf.schultz@gmail.com) and others.
#
# All rights reserved.
#
# See the file License for details.

# safe.tcl - manage safe interpreters

# safe_init:
#  initialize the safe interpreter <interp>
#
proc safe_init { interp } {

    # share the standard I/O channels
    catch {interp share {} stdout $interp}
    catch {interp share {} stderr $interp}

    # the safe_commands list contains all commands considered safe
    # and thus available in the safe interpreter verbatim

    # object
    set safe_commands { crtOb delOb hSL selOb withOb convOb }
    # clipboard
    lappend safe_commands cutOb copOb pasOb pasmovOb repOb
    # current level
    lappend safe_commands goUp goDown goTop
    # property
    lappend safe_commands setProp getProp setProperty getProperty
    lappend safe_commands setTrafo getTrafo setAttr getAttr setMat getMat
    lappend safe_commands nameOb shaderSet shaderGet
    # transformations
    lappend safe_commands movOb rotOb scalOb
    # tags
    lappend safe_commands addTag delTags getTags setTags
    # NURBS
    lappend safe_commands revertC revertuS revertvS swapuvS
    lappend safe_commands clampNC elevateNC insknNC remknNC refineNC coarsenNC
    lappend safe_commands rescaleknNC splitNC toXYNC trimNC estlenNC reparamNC
    lappend safe_commands centerNC makeCompNC estlenNC shiftClosedBS
    lappend safe_commands clampuNP clampvNP rescaleknNP insknuNP insknvNP
    lappend safe_commands closeuNP closevNP elevateuNP elevatevNP
    lappend safe_commands splituNP splitvNP extrNP
    lappend safe_commands crtNCircle crtNRect crtClosedBS crtNSphere
    lappend safe_commands crtNSphere2
    # PolyMesh
    lappend safe_commands mergePo optiPo splitPo
    # point editing
    lappend safe_commands setPnt getPnt selPnts invPnts centerPnts
    lappend safe_commands movPnts rotPnts scalPnts
    # enquiry
    lappend safe_commands getVersion getType getName getLevel hasChild
    # misc
    lappend safe_commands resolveIn

    # make safe commands known in safe interpreter
    foreach command $safe_commands {
	interp alias $interp $command {} $command
    }

    # property GUI
    interp alias $interp addPropertyGUI {} safe_addPropertyGUI
    interp alias $interp addParam {} addParam

    interp alias $interp puts {} safe_puts

 return;
}
# safe_init


# safe_puts:
#  a safe puts, that only writes to stdout/stderr for use in
#  the safe interpreter
proc safe_puts { args } {

    if { [llength $args] > 1 } {
	set channelId [lindex $args 0]
	if { ($channelId == "stdout") || ($channelId == "stderr") } {
	    puts $channelId [lrange $args 1 end]
	}
    }

    if { [llength $args] == 1 } {
	puts stdout $args
    }

 return;
}
# safe_puts


# safe_addPropertyGUI:
#  safe version to create a property GUI
#  first, fetches data array from safe interpreter, then calls through
#  to addPropertyGUI
proc safe_addPropertyGUI { name {sproc ""} {gproc ""} } {

    set w ""
    # create the standard array name
    set arrayname ${name}Data
    global $arrayname
    # never let the (evil?) script overwrite existing arrays
    if { ! [info exists $arrayname] } {
	# fetch the data
	array set $arrayname [aySafeInterp eval array get ::$arrayname]
	# now create the property GUI management array and frame
	# by calling through to addPropertyGUI
	set w [addPropertyGUI $name $sproc $gproc]
    }

 return $w;
}
# safe_addPropertyGUI