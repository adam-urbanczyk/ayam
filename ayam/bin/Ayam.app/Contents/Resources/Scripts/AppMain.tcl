# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2005 by Randolf Schultz
# (rschultz@informatik.uni-rostock.de) and others.
#
# All rights reserved.
#
# See the file License for details.

# AppMain.tcl - this file implements the startup script for Ayam.app
# (the Mac OS X/Aqua  Ayam application bundle)

# OpenDocument:
#  some document(s) are dropped onto the application (on Mac OS X/Aqua),
#  load them (replace the first, insert the remaining)
proc ::tk::mac::OpenDocument { args } {
    grab .fu
    set j 0
    foreach arg $args {
	if { ([file extension $arg] == ".ay") || \
		 ([file extension $arg] == ".AY") } {

	    regsub -all {\\} $arg {/} filename

	    if { $j == 0 } {
		# close all views
		viewCloseAll

		set ay_error ""
		
		replaceScene $filename
		if { $ay_error < 2 } {
		    set ay(filename) $filename
		    set windowfilename [file tail [file rootname $newfilename]]
		    wm title . "Ayam - Main - $windowfilename"
		    ayError 4 "replaceScene" "Done reading scene from:"
		    ayError 4 "replaceScene" "$filename"
		    if { [file exists $filename] } {
			set dirname [file dirname $filename]
			cd $dirname
			update_prompt ay uc w
		    }
		    io_mruAdd $filename
		} else {
		    ayError 2 "replaceScene" "There were errors while loading:"
		    ayError 2 "replaceScene" "$filename"
		}
		set j 1
	    } else {
		set ay_error ""
		insertScene $filename
		if { $ay_error < 2 } {
		    ayError 4 "insertScene" "Done inserting scene from:"
		    ayError 4 "insertScene" "$filename"
		} else {
		    ayError 2 "insertScene" "There were errors while loading:"
		    ayError 2 "insertScene" "$filename"
		}
		# if
	    }
	    # if
	    uS; rV
	}
	# if
    }
    # foreach
    grab release .fu
 return;
}
# OpenDocument

if {[string first "-psn" [lindex $argv 0]] == 0} { set argv [lrange $argv 1 end]}
console show

if [catch {source [file dirname [info script]]/tcl/ayam.tcl}] { puts $errorInfo}
# EOF
