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
# zdialog.tcl - use zenity instead of native Tk (file) requesters
#

if { [auto_execok zenity] == "" } {
    ayError 2 "zdialog.tcl" "Can not find/execute zenity!"
    return;
}

# zdialog_run:
#  run zenity, capture and return its output (from the stdout channel)
#
proc zdialog_run { cmd } {

    set newcmd  [join $cmd " "]

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
# zdialog_run


rename ::tk_getOpenFile ::zdialog_getOpenFile

# ::tk_getOpenFile
#  the new getOpenFile procedure using zenity
#
proc ::tk_getOpenFile { args } {
    set opt ""
    foreach {key val} $args {
	switch -glob -- $key {
	    -t* {
		lappend opt --title=$val
	    }
	}
	# switch
    }
    # foreach

    set cmd [list zenity --file-selection]
    foreach elem $opt { lappend cmd "\"$elem\"" }

 return [zdialog_run $cmd];
}
# ::tk_getOpenFile


rename ::tk_getSaveFile ::zdialog_getSaveFile

# ::tk_getSaveFile
#  the new getSaveFile procedure using zenity
#
proc ::tk_getSaveFile { args } {
    set opt ""
    foreach {key val} $args {
	switch -glob -- $key {
	    -initialf* {
		if { $val != "" } {
		    lappend opt --filename=$val
		}
	    }
	    -t* {
		lappend opt --title=$val
	    }
	}
	# switch
    }
    # foreach

    set cmd [list zenity --file-selection --save]
    foreach elem $opt { lappend cmd "\"$elem\"" }

 return [zdialog_run $cmd];
}
# ::tk_getSaveFile


rename ::tk_chooseDirectory ::zdialog_chooseDirectory

# ::tk_chooseDirectory
#  the new chooseDirectory procedure using zenity
#
proc ::tk_chooseDirectory { args } {
    set opt ""
    foreach {key val} $args {
	switch -glob -- $key {
	    -t* {
		lappend opt --title=$val
	    }
	}
	# switch
    }
    # foreach

    set cmd [list zenity --file-selection --directory]
    foreach elem $opt { lappend cmd "\"$elem\"" }

 return [zdialog_run $cmd];
}
# ::tk_chooseDirectory


# zdialog_revert:
#  undo all changes
#
proc zdialog_revert { } {

    rename ::tk_getOpenFile ""
    rename ::zdialog_getOpenFile ::tk_getOpenFile
    rename ::tk_getSaveFile ""
    rename ::zdialog_getSaveFile ::tk_getSaveFile
    rename ::tk_chooseDirectory ""
    rename ::zdialog_chooseDirectory ::tk_chooseDirectory

 return;
}
# zdialog_revert


# attach revert procedure to custom menu
global ay
$ay(cm) add command -label "RevertZDialog" -command {
    zdialog_revert
    # XXXX remove ourselves from the menu?
}

ayError 4 "zdialog" "Enabled."
