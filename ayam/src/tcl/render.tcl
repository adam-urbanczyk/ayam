# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2001 by Randolf Schultz
# (rschultz@informatik.uni-rostock.de) and others.
#
# All rights reserved.
#
# See the file License for details.

# render.tcl - invoke an external renderer

#XXXX does not work this way
proc render { args } {
    global ay ayprefs

    set command "|"
    append command [lindex $args 0]
    append command " 2>@ stderr &"

    set f [open $command r]
    fileevent $f readable "gotline $f"
    proc gotline f {
	if {[gets $f line]<0} {
	    # it died !
	    close $f ;      # catch to get exit status
	    return ;        # (see errorCode in tclvars.n)
	}
	# use $line !
	puts $line
    }

}
# render

