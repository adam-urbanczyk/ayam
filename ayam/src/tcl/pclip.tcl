# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2001 by Randolf Schultz
# (rschultz@informatik.uni-rostock.de) and others.
#
# All rights reserved.
#
# See the file License for details.

# pclip.tcl - property clipboard

array set pclip_reset {}

array set pclip_omit {}

array set pclip_omit_label {}

array set pclip_clipboard {}

set pclip_prop ""


# pclip_toggleomit:
# arranges for all variables in varnames
# to be omitted by the next copy property action
proc pclip_toggleomit { label varnames } {
    global pclip_omit pclip_omit_label
    set oldlabel [$label cget -text] 

    if { [string first "!" $oldlabel ] == -1 } {
	set name "!$oldlabel"
	$label configure -text $name
	set pclip_omit_label($label) ""
	foreach i $varnames {
	    set pclip_omit($i) ""
	}
    } else {
	set name [string trimleft $oldlabel !]
	$label configure -text $name
	unset pclip_omit_label($label)
	foreach i $varnames {
	    unset pclip_omit($i)
	}
    }

}
# pclip_toggleomit


# pclip_copy:
# copy current property to property clipboard
# if mode == 0: _omitting_ all marked entities
# else copying _just_ the marked entities
proc pclip_copy { mode } {
    global ay pclip_prop pclip_omit
    upvar #0 pclip_clipboard clipboard

    set lb $ay(plb)
    set index [$lb curselection]
    
    if { $index == "" } {
	puts stderr "pclip_copy: No property selected!"
	return;
    }

    set pclip_prop [$lb get $index]

    catch {unset clipboard}

    global $pclip_prop
    eval [subst "set arr \$${pclip_prop}(arr)"]
    global $arr
    set avnames [array names $arr]

    if { $mode == 0 } {
	foreach i $avnames {
	    if { ! [info exists pclip_omit($i)] } {
		eval [subst "set clipboard\(\$i\) \{\$$arr\(\$i\)\}"]
	    }
	}   
    } else {
	foreach i $avnames {
	    if { [info exists pclip_omit($i)] } {
		eval [subst "set clipboard\(\$i\) \{\$$arr\(\$i\)\}"]
	    } 
	}
    }
}
# pclip_copy


# pclip_paste:
# paste content of property clipboard to currently
# selected object(s)
proc pclip_paste { } {

    global pclip_prop
    if { $pclip_prop != "" } {
	global ay
	set ay(pasteProp) 1

	undo save "PasteProp"

	forAll 0 {
	    global ay pclip_prop curtypes
	    if { $ay(lb) == 1 } {
		set olb $ay(olb)
		set o [$ay(olb) curselection]
		set type [lindex $curtypes $o]
	    } else {
		getType type
	    }

	    global ${type}_props
	    eval [subst "set list \{\$${type}_props\}"]

	    set index [lsearch -exact $list $pclip_prop ]
	    if { $index >= 0 } {
		global $pclip_prop
		eval [subst "set arr \$${pclip_prop}(arr)"]
		set gproc ""
		eval [subst "set gproc \$${pclip_prop}(gproc)"]
		set sproc ""
		eval [subst "set sproc \$${pclip_prop}(sproc)"]

		global $arr pclip_clipboard
		if { $gproc != "" } { $gproc } else { getProp }
		set avnames [array names pclip_clipboard]
		foreach j $avnames {

		    eval [subst "set $arr\(\$j\) {\$pclip_clipboard\(\$j\)}"]
		}
		if { $sproc != "" } { $sproc } else { setProp }
	    } else {

	   puts stderr "pclip_paste: Unable to paste property: \"$pclip_prop\""
	   puts stderr "to objects of type: \"$type\"!"

	    }

	}

	set ay(pasteProp) 0
    } else {
	puts stderr "pclip_paste: Property clipboard is empty!"
    }
}
# pclip_paste


# pclip_pastetosel:
# paste content of property clipboard to currently
# selected property of selected object
# use at own risk!
proc pclip_pastetosel { } {
    global pclip_prop ayprefs ay

    if { $ayprefs(Wpclip_pastetosel) == 1 } {
	set t "Warning!"
	set m "This operation may destroy the current property. Proceed if you know what you are doing!"

	if { $ayprefs(PrepDiaCap) == 1 } {
	    set m "$t\n\n$m"
	}

	set c [tk_messageBox -title $t -type okcancel -icon warning\
		-message $m]

     if { $c == "cancel" } { return; }

    }

    if { $pclip_prop != "" } {
	set ay(pasteProp) 1

	if { $ay(lb) == 1 } {
	    set olb $ay(olb)
	    set o [$ay(olb) curselection]
	    set type [lindex $curtypes $o]
	} else {
	    getType type
	}
	global ${type}_props
	eval [subst "set list \{\$${type}_props\}"]

	set index [lsearch -exact $list $pclip_prop ]
	set lb $ay(plb)
	set sel ""
	set sel [$lb curselection]
	if { $sel != "" } {
	    set pclip_prop [$lb get $sel ]
	    global $pclip_prop

	    # get name of property array
	    eval [subst "set arr \$${pclip_prop}(arr)"]

	    # get names of get/set procedures
	    set gproc ""
	    eval [subst "set gproc \$${pclip_prop}(gproc)"]
	    set sproc ""
	    eval [subst "set sproc \$${pclip_prop}(sproc)"]

	    # get old values of property to paste to
	    global $arr pclip_clipboard
	    if { $gproc != "" } { $gproc } else { getProp }

	    # copy values from property clipboard to property, possibly
	    # mixing old and new values (depending on clipboard content)
	    set avnames [array names pclip_clipboard]
	    foreach j $avnames {
		eval [subst "set $arr\(\$j\) \{\$pclip_clipboard\(\$j\)\}"]
	    }

	    # copy the new values of the property to C-context
	    if { $sproc != "" } { $sproc } else { setProp }

	    # immediately read them back for proper display in
	    # the property GUI and update the property GUI
	    plb_update
	} else {
	    puts stderr "pclip_pastetosel: No Property selected!"
	}
	set ay(pasteProp) 0
    } else {
	puts stderr "pclip_pastetosel: Property clipboard is empty!"
    }
}
# pclip_pastetosel

# shortcut to pclip_copy
proc copyProp { mode } {

    pclip_copy $mode

 return;
}

# shortcut to pclip_paste
proc pasteProp { } {

    pclip_paste

 return;
}