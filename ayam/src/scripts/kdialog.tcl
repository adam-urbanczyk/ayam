#
# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2008 by Randolf Schultz
# (randolf.schultz@gmail.com) and others.
#
# All rights reserved.
#
# See the file License for details.

#
# kdialog.tcl - use kdialog instead of native Tk (file) requesters
#

if { [auto_execok kdialog] == "" } {
    ayError 2 "kdialog.tcl" "Can not find/execute kdialog!"
    return;
}


# kdialog_run:
#  run kdialog, capture and return its output (from the stdout channel)
#
proc kdialog_run { cmd } {

    set newcmd [join $cmd " "]

    if { [catch {open "| $newcmd 2>@stdout"} file] } {
	return "Can't open pipe for '$cmd'"
    }

    set pipe $file
    fconfigure $pipe -buffering none

    set output ""

    while { [gets $pipe data] >= 0 } {
	append output $data
    }

    catch {close $pipe} err

 return $output
}
# kdialog_run


rename ::tk_getOpenFile ::kdialog_getOpenFile

proc ::tk_getOpenFile { args } {
    set opt ""
    set idir "."
    set ftypes ""
    set tspec ""
    foreach {key val} $args {
	switch -glob -- $key {
	    -initiald* {
		set idir $val
	    }
	    -f* {
		foreach {ftype} $val {
		    if { [lindex $ftype 1] != "*" } {
			foreach ext [lindex $ftype 1] {
			    lappend ftypes *$ext
			}
		    }
		}
	    }
	    -t* {
		lappend opt "-title=\\\"$val\\\""
	    }
	}
	# switch
    }
    # foreach

    if { $ftypes != "" } {
	set tspec "$ftypes |Supported Files"
    }

    set cmd [list kdialog --getopenfilename]
    if { $idir != "" } {
	lappend cmd "\"$idir\""
    }
    if { $tspec != "" } {
	lappend cmd "\"$tspec\""
    }
    foreach elem $opt { lappend cmd "\"$elem\"" }

 return [kdialog_run $cmd];
}
# ::tk_getOpenFile

rename ::tk_getSaveFile ::kdialog_getSaveFile

proc ::tk_getSaveFile { args } {
    set opt ""
    set idir "."
    set ftypes ""
    set tspec ""
    foreach {key val} $args {
	switch -glob -- $key {
	    -initiald* {
		set idir $val
	    }
	    -f* {
		foreach {ftype} $val {
		    if { [lindex $ftype 1] != "*" } {
			foreach ext [lindex $ftype 1] {
			    lappend ftypes *$ext
			}
		    }
		}
	    }
	    -t* {
		lappend opt "-title=\\\"$val\\\""
	    }
	}
	# switch
    }
    # foreach

    if { $ftypes != "" } {
	set tspec "\"$ftypes | Supported Files\""
    }

    set cmd [list kdialog --getsavefilename]
    if { $idir != "" } {
	lappend cmd "\"$idir\""
    }
    if { $tspec != "" } {
	lappend cmd "\"$tspec\""
    }
    foreach elem $opt { lappend cmd "\"$elem\"" }

 return [kdialog_run $cmd];
}
# ::tk_getSaveFile


rename ::tk_chooseDirectory ::kdialog_chooseDirectory

proc ::tk_chooseDirectory { args } {
    set opt ""
    set idir "."
    foreach {key val} $args {
	switch -glob -- $key {
	    -initiald* {
		set idir $val
	    }
	    -t* {
		lappend opt "-title=\\\"$val\\\""
	    }
	}
	# switch
    }
    # foreach

    set cmd [list kdialog --getexistingdirectory]
    if { $idir != "" } {
	lappend cmd $idir
    }
    foreach elem $opt { lappend cmd "\"$elem\"" }

 return [kdialog_run $cmd];
}
# ::tk_chooseDirectory


proc kdialog_revert { } {

    rename ::tk_getOpenFile ""
    rename ::kdialog_getOpenFile ::tk_getOpenFile
    rename ::tk_getSaveFile ""
    rename ::kdialog_getSaveFile ::tk_getSaveFile
    rename ::tk_chooseDirectory ""
    rename ::kdialog_chooseDirectory ::tk_chooseDirectory

 return;
}
# kdialog_revert


# attach revert procedure to custom menu
global ay
$ay(cm) add command -label "RevertKDialog" -command {
    kdialog_revert
    # XXXX also remove ourselves from the menu?
}

ayError 4 "kdialog" "Enabled."
