# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2001 by Randolf Schultz
# (rschultz@informatik.uni-rostock.de) and others.
#
# All rights reserved.
#
# See the file License for details.

# tmp.tcl - tmp files management

# tmp_clean:
#
proc tmp_clean { all } {
    global ayprefs ay

    if { $all == 1 } {
	puts "Removing all temporary files..."
	foreach i $ay(tmpfiles) {
	    cd [file dirname $i]
	    set files [glob [file tail $i]*]
	    foreach j $files { file delete $j }
	}
    } else {

	set new_tempfiles ""
	set count 0
	set todelete [expr [llength $ay(tmpfiles)]-$ayprefs(KeepNTmpFiles)]
	
	foreach i $ay(tmpfiles) {
	    if { $count < $todelete } {
                #puts "Removing some temporary files..."
		set oldcd [pwd]
		cd [file dirname $i]
		set files [glob [file tail $i]*]
		foreach j $files { file delete $j }
		cd $oldcd
	    } else {
		lappend new_tempfiles $i
	    }

	    incr count
	}
	# foreach
	set ay(tmpfiles) $new_tempfiles
    }
    # if

}
# tmp_clean