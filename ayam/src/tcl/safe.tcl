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

    # make some Tcl procedures known in the safe interpreter

    # property GUI
    interp alias $interp addPropertyGUI {} safe_addPropertyGUI
    interp alias $interp addParam {} addParam
    interp alias $interp addParamB {} addParamB
    interp alias $interp addParamPair {} addParamPair

    interp alias $interp addMatrix {} addMatrix
    interp alias $interp addMatrixB {} addMatrixB

    interp alias $interp addColor {} addColor
    interp alias $interp addColorB {} addColorB

    interp alias $interp addCheck {} addCheck
    interp alias $interp addCheckB {} addCheckB

    interp alias $interp addMenu {} addMenu
    interp alias $interp addMenuB {} addMenuB

    interp alias $interp addString {} addString
    interp alias $interp addStringB {} addStringB

    interp alias $interp addCommand {} save_addCommand
    interp alias $interp addCommandB {} addCommandB

    interp alias $interp addText {} addText

    interp alias $interp addInfo {} addInfo
    interp alias $interp addInfoB {} addInfoB

    # tree
    interp alias $interp treeSelect {} treeSelect

    # puts
    interp alias $interp puts {} safe_puts

    # property management
    interp alias $interp setProperty {} setProperty
    interp alias $interp getProperty {} safe_getProperty

    # redraw
    interp alias $interp rV {} rV

    #safe_transfer setProperty $interp

    aySafeInterp eval {set ::cancelled 0}
    safe_transfer cancelled $interp
    safe_transfer renameWhileFor $interp
    aySafeInterp eval renameWhileFor

 return;
}
# safe_init


# safe_transfer:
#  transfer the Tcl procedure designated by <procname> verbatim
#  to the target interpreter <interp>
proc safe_transfer { procname interp } {

    set args [info args $procname]
    set body [info body $procname]

    set procspec proc
    lappend procspec $procname
    lappend procspec $args
    lappend procspec $body

    $interp eval $procspec

 return;
}
# safe_transfer


# safe_puts:
#  a safe puts, that only writes to stdout/stderr for use in
#  the safe interpreter
proc safe_puts { args } {

    if { [llength $args] > 1 } {
	set channelId [lindex $args 0]
	if { ($channelId == "stdout") || ($channelId == "stderr") } {
	    puts $channelId [lrange $args 1 end]
	} else {
	    puts stderr "Access to channel $channelId blocked!"
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
    }

    # now create the property GUI management array and frame
    # by calling through to addPropertyGUI
    set w [addPropertyGUI $name $sproc $gproc]
    # inform the safe context about this
    aySafeInterp eval set ::${name}GUI 1

 return $w;
}
# safe_addPropertyGUI


# save_addCommand:
#  safe version of addCommand; makes sure, the command runs in
#  the safe interpreter
proc save_addCommand { w name text command } {
    set newcmd "aySafeInterp eval { "
    append newcmd $command
    append newcmd " }"
    addCommand $w $name $text $newcmd
 return;
}
# save_addCommand


# safe_getProperty:
#  safe version of getProperty
#  it first fetches the data, then hands it over to the
#  safe interpreter
#  XXXX add mechanism to prevent even attempts of
#  accessing env, ay, ayprefs arrays (even if getProperty
#  already checks presence of requested property)?
proc safe_getProperty { property varName } {

    getProperty $property val

    aySafeInterp eval set $varName $val

 return;
}
# safe_getProperty