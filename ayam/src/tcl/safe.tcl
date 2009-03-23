# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2009 by Randolf Schultz
# (randolf.schultz@gmail.com) and others.
#
# All rights reserved.
#
# See the file License for details.

# safe.tcl - manage safe interpreters

proc safe_init { interp } {
    interp share {} stdout $interp
    interp share {} stderr $interp

    interp alias $interp crtOb {} crtOb

    interp alias $interp setProperty {} setProperty
    interp alias $interp getProperty {} getProperty

    interp alias $interp setPoint {} setPoint
    interp alias $interp getPoint {} getPoint

    interp alias $interp puts {} safe_puts

 return;
}
# safe_init

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