# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2001 by Randolf Schultz
# (rschultz@informatik.uni-rostock.de) and others.
#
# All rights reserved.
#
# See the file License for details.

# plb.tcl - the properties listbox

proc plb_resizecanvas {win W H} {
   $win itemconfigure all -width $W
   set bbox [$win bbox all]
   $win configure -scrollregion $bbox -yscrollincrement 0.1i
}

#
proc plb_open { w } {
global ay ayprefs


label $w.la -text "Properties:" -padx 0 -pady 0
pack $w.la -in $w -side top -fill x -expand no

bind $w.la <Double-ButtonPress-1> {
    global ay;
    $ay(plb) selection clear 0 end;
    plb_update
}
balloon_set $w.la "Double click here\nto deselect property"

# the properties listbox
set f $w
listbox $f.li -width 14 -height 12 -yscrollcommand {global ay; $ay(pss) set}\
	-selectmode single -exportselection 0 -takefocus 0

set ay(plb) $f.li

bind $f.li <ButtonRelease-1> {
    global ay pclip_omit pclip_omit_label sel

    if { $ay(lb) == 0 } {
	after 100 {global ay; focus -force $ay(tree)}
    } else {
	after 100 {global ay; focus -force $ay(olb)}
    }

    set sel ""
    getSel sel
    if { $sel == "" } { break }

    set lb $ay(plb)
    if { [$lb size] < 1 } { return }
    set prop [$lb get [$lb curselection]]
    eval [subst "set w \$${prop}(w)"]

    set getprocp ""
    eval [subst "set getprocp \$${prop}(gproc)"]
    if { $getprocp != "" } { $getprocp } else { getProp }

    eval [subst "set arrname \$${prop}(arr)"]
    global pclip_reset $arrname
    array set pclip_reset  [array get $arrname]
    $ay(pca) itemconfigure 1 -window $ay(pca).$w

    # resize canvas
    set width [expr [winfo reqwidth $ay(pca).$w] + 10]
    set height [expr [winfo reqheight $ay(pca).$w] + 10]
    $ay(pca) configure -width $width
    $ay(pca) configure -height $height
    $ay(pca) configure -scrollregion [list 0 5 $width $height]

    # resize main?
    if { $ayprefs(AutoResize) == 1 } {
	plb_resize
    }
    if { [array exists pclip_omit] } {
	unset pclip_omit
	set labels [array names pclip_omit_label]
	foreach label $labels {
	    set oldname [$label cget -text] 
	    set name [string trimleft $oldname !]
	    $label configure -text $name
	}
	unset pclip_omit_label	    
	array set pclip_omit { }
	array set pclip_omit_label { }
    }
    # improve focus traversal (speed-wise)
    global tcl_platform AYWITHAQUA
    if { $ay(lb) == 1 } {
	bind $ay(olb) <Key-Tab>\
		"focus [tk_focusNext $ay(pca).$w];break"
	bind [tk_focusNext $ay(pca).$w] <Shift-Tab>\
		"focus $ay(olb);break"
	if { ( $tcl_platform(platform) != "windows" ) && ( ! $AYWITHAQUA ) } {
	    bind [tk_focusNext $ay(pca).$w] <ISO_Left_Tab>\
		    "focus $ay(olb);break"
	}
    } else {
	bind $ay(tree) <Key-Tab>\
		"focus [tk_focusNext $ay(pca).$w];break"
	bind [tk_focusNext $ay(pca).$w] <Shift-Tab>\
		"focus $ay(tree);break"
	if { ( $tcl_platform(platform) != "windows" ) && ( ! $AYWITHAQUA ) } {
	    bind [tk_focusNext $ay(pca).$w] <ISO_Left_Tab>\
		    "focus $ay(tree);break"
	}
    }

}
#bind

bind $f.li <ButtonPress-4> {
    global ay
    $ay(plb) yview scroll -1 pages
    break
}

bind $f.li <ButtonPress-5> {
    global ay
    $ay(plb) yview scroll 1 pages
    break
}

global tcl_platform
if { $tcl_platform(platform) == "windows" } {
    bind $f.li <MouseWheel> {
	global ay
	if { %D < 0.0 } {
	    $ay(plb) yview scroll 1 pages
	} else {
	    $ay(plb) yview scroll -1 pages 
	}
	break
    }
}

# plb context menu
set m [menu $ay(plb).popup -tearoff 0]
$m add command -label "Deselect Property" -command {
    global ay
    $ay(plb) selection clear 0 end
    plb_update
}
$m add command -label "Copy Property" -command {
    pclip_copy 0
}
$m add command -label "Copy Marked Property" -command {
    pclip_copy 1
}
$m add command -label "Paste Property" -command {
    global ay
    pclip_paste
    set ay(sc) 1
}

bind $f.li <ButtonPress-3> {
    global ay

    set xy [winfo pointerxy .]
    set x [lindex $xy 0]
    set y [lindex $xy 1]

    tk_popup $ay(plb).popup $x $y
}

# plb scrollbar
set ay(pss) $f.s
scrollbar $f.s -takefocus 0 -command {
    global ay
    $ay(plb) yview
}

pack $f.li $f.s -in $f -side left -fill y -expand no


# the arguments canvas
frame $w.fArg -bd 2 -relief sunken -highlightthickness 1
set f $w.fArg

# apply/reset buttons
frame $f.fb
set f $f.fb

button $f.b1 -text "Apply" -padx 10 -pady 0 -command {
    global ay
    undo save Apply
    set lb $ay(plb)
    set sel [$lb curselection]
    if { $sel == "" } { return }
    set prop [$lb get $sel]
    set setprocp ""
    eval [subst "set setprocp \$${prop}(sproc)"]
    if { $setprocp != "" } { $setprocp } else { setProp }
    set getprocp ""
    eval [subst "set getprocp \$${prop}(gproc)"]
    if { $getprocp != "" } { $getprocp } else { getProp }

    eval [subst "set arrname \$${prop}(arr)"]
    global pclip_reset $arrname
    array set pclip_reset  [array get $arrname]
    rV
} -takefocus 0 -highlightthickness 0

button $f.b2 -text "Reset" -padx 10 -pady 0 -command {
    global ay
    set lb $ay(plb)
    set sel [$lb curselection]
    if { $sel == "" } { return }
    set prop [$lb get $sel]
    global $prop pclip_reset
    eval [subst "set arrname \$${prop}(arr)"]
    array set $arrname [array get pclip_reset]
} -takefocus 0 -highlightthickness 0

pack $f.b1 $f.b2 -in $f -side left -fill x -expand yes 
pack $f -in $w.fArg -side bottom -fill x -expand no


set f $w.fArg
frame $f.fca
set f $f.fca

# the canvas itself

canvas $f.ca -yscrollcommand { global ay; $ay(pcas) set }\
 -highlightthickness 0
set ay(pca) $f.ca
# XXXX window for prop-guis, there should be a window for every
# prop GUI, that is just being displayed if needed, by changing
# the window as:
# $f.ca itemconfigure 1 -window .ca.f1
set ay(pw) [frame $f.ca.w -width 100 ]
$f.ca create window 5 5 -anchor nw -window $ay(pw)
set width [expr [winfo reqwidth $ay(pw)] + 10]
$ay(pca) configure -width $width

bind . <ButtonPress-4> {
    global ay
    $ay(pca) yview scroll -1 pages
}

bind . <ButtonPress-5> {
    global ay
    $ay(pca) yview scroll 1 pages
}

global tcl_platform
if { $tcl_platform(platform) == "windows" } {
    bind . <MouseWheel> {
	global ay
	if { %D < 0.0 } {
	    $ay(pca) yview scroll 1 pages
	} else {
	    $ay(pca) yview scroll -1 pages 
	}
    }
}

pack $f.ca -in $f -side left -fill both -expand yes

scrollbar $f.s -command { global ay; $ay(pca) yview } -takefocus 0
set ay(pcas) $f.s
pack $f.s -in $f -side left -fill y 


pack $w.fArg.fca -in $w.fArg -side top -fill both -expand yes
pack $w.fArg -in $w -side top -fill both -expand yes
update
}
# plb_open


#
proc pgui_close { w } {
destroy $w.la
destroy $w.li
destroy $w.s
destroy $w.fArg

}
# plb_close

set ay(PlbUpdateSema) 0
#
proc plb_update { } {
global ay curtypes pclip_omit pclip_omit_label

# protect against double updates
if { $ay(PlbUpdateSema) == 1 } {
    return
} else {
    set ay(PlbUpdateSema) 1
}

if { $ay(lb) == 1 } {
    set index [$ay(olb) curselection]
} else {
    set index [$ay(tree) selection get]
}

set lb $ay(plb)
set oldsel ""
set oldsel [$lb curselection]

if { $oldsel == "" } {
    if { $ay(lb) == 1 } {
	bind $ay(olb) <Key-Tab> "focus .fl.con.console;break"
    } else {
	bind $ay(tree) <Key-Tab> "focus .fl.con.console;break"
    }
}

# delete current entries
$lb delete 0 end

# show new property GUI
$ay(pca) itemconfigure 1 -window $ay(pw)
if { [llength $index] == 1 } {
    if { $ay(lb) == 1 } {
	set type [lindex $curtypes $index]
    } else {
	getType type
    }
    if { $type != ".." } {
	global ${type}_props
	eval [subst "set props {\$${type}_props}"]
	eval [subst "$lb insert end $props"]
    }

    # re-create old propgui?
    if { $oldsel != "" && $oldsel <= [$lb index end] } {
	$lb selection set $oldsel
	set tmp [$lb curselection]
	if { $tmp != "" } {
	    set prop [$lb get $tmp]
	    global $prop ayprefs
	    eval [subst "set w \$${prop}(w)"]

	    set getprocp ""
	    eval [subst "set getprocp \$${prop}(gproc)"]
	    if { $getprocp != "" } { $getprocp } else { getProp }

	    $ay(pca) itemconfigure 1 -window $ay(pca).$w

	    # resize canvas
	    set width [expr [winfo reqwidth $ay(pca).$w] + 10]
	    set height [expr [winfo reqheight $ay(pca).$w] + 10]
	    $ay(pca) configure -width $width
	    $ay(pca) configure -scrollregion [list 0 5 $width $height]

	    # resize main?
	    if { $ayprefs(AutoResize) == 1 } {
		plb_resize
	    }
	    # update omit array
	    if { [array exists pclip_omit] } {
		unset pclip_omit
		set labels [array names pclip_omit_label]
		foreach label $labels {
		    set oldname [$label cget -text] 
		    set name [string trimleft $oldname !]
		    $label configure -text $name
		}
		unset pclip_omit_label	    
		array set pclip_omit { }
		array set pclip_omit_label { }
	    }
	    # improve focus traversal (speed-wise)
	    global tcl_platform AYWITHAQUA
	    if { $ay(lb) == 1 } {
		bind $ay(olb) <Key-Tab>\
			"focus [tk_focusNext $ay(pca).$w];break"
		bind [tk_focusNext $ay(pca).$w] <Shift-Tab>\
			"focus $ay(olb);break"
		if { ( $tcl_platform(platform) != "windows" ) &&\
			( ! $AYWITHAQUA ) } {
		    bind [tk_focusNext $ay(pca).$w] <ISO_Left_Tab>\
			    "focus $ay(olb);break"
		}
	    } else {
		bind $ay(tree) <Key-Tab>\
			"focus [tk_focusNext $ay(pca).$w];break"
		bind [tk_focusNext $ay(pca).$w] <Shift-Tab>\
			"focus $ay(tree);break"
		if { ( $tcl_platform(platform) != "windows" ) &&\
			( ! $AYWITHAQUA ) } {
		    bind [tk_focusNext $ay(pca).$w] <ISO_Left_Tab>\
			    "focus $ay(tree);break"
		}
	    }
	}
	#if
    }
    #if
}
#if

set ay(PlbUpdateSema) 0

 return;
}
# plb_update


#
proc plb_resize { } {
    global ayprefs ay
    if { $ayprefs(AutoResize) != 1 } { return; }
    update
    set newwidth [expr [winfo width .fu.fMain.fHier] + [winfo reqwidth .fu.fMain.fProp] + [winfo reqwidth .fu.fMain.__h1] ]
   set newheight [winfo height .]
    
    set ng ${newwidth}x${newheight}
    set oldgeom [wm geom .]
    regexp {([0-9]+)?x?([0-9]+)?(\+|\-)?([0-9]+)?(\+|\-)?([0-9]+)?} $oldgeom blurb width height blurb2 x blurb3 y

    if { $newwidth <= $width } { return; }

    set vwidth [expr [winfo rootx .fu.fMain.fProp]]
 
    global tcl_platform ayprefs
    if { ($tcl_platform(platform) != "windows") &&\
	    ($ayprefs(TwmCompat) != 1) } {
	set x [winfo rootx .]
	set y [winfo rooty .]

	if { $x >= 0 } { append ng "+$x" } else { append ng "$x" }
	if { $y >= 0 } { append ng "+$y" } else { append ng "$y" }
    }
    wm geometry . ""
    wm geometry . $ng
    update
    pane_constrain . .fu.fMain.__h1 .fu.fMain.fHier .fu.fMain.fProp width x 1
    pane_motion $vwidth . .fu.fMain.__h1 width x 1

}
# plb_resize


# plb_focus:
#  this proc is bound to the Tab-key and scrolls the property
#  canvas to display the new item that gets the focus if it is
#  outside the current visible region of the canvas
proc plb_focus { } {
    global ay
    set w [focus]
    if { $w != "" } {
	if { [string first $ay(pca) $w] != -1} {
	    set ca $ay(pca)
	    set height [$ca cget -height]
	    set visible [$ca yview]
	    set wypos [winfo y [winfo parent $w]]
	    set fraction [expr (double($wypos) +\
	    double([winfo reqheight $w]))/double($height)]
	    if { ($fraction < [lindex $visible 0]) ||
	         ($fraction > [lindex $visible 1]) } {
		     set fraction [expr double($wypos)/double($height)]
		     $ca yview moveto $fraction
	    }
	
	}
    }
}
# plb_focus
