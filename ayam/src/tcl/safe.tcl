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
    catch {interp share {} stdout $interp}
    catch {interp share {} stderr $interp}

    # object management
    set safe_commands { crtOb delOb hSL withOb }
    # property management
    lappend safe_commands setProp getProp setProperty getProperty
    lappend safe_commands setTrafo getTrafo setAttr getAttr setMat getMat
    # transformations
    lappend safe_commands movOb rotOb scalOb movSel rotSel scalSel
    # point editing
    lappend safe_commands setPnt getPnt

    # make safe commands known in safe interpreter
    foreach command $safe_commands {
	interp alias $interp $command {} $command
    }

    # property GUI
    interp alias $interp addPropertyGUI {} addPropertyGUI
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

 return;
}
# safe_puts