# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2005 by Randolf Schultz
# (randolf.schultz@gmail.com) and others.
#
# All rights reserved.
#
# See the file License for details.

# plb.tcl - procedures for managing the properties listbox and display

# Global Variables:

set ay(PlbUpdateSema) 0

# Procedures:

# plb_resizecanvas:
#
#
proc plb_resizecanvas { win W H } {
   $win itemconfigure all -width $W
   set bbox [$win bbox all]
   $win configure -scrollregion $bbox -yscrollincrement 0.1i
 return;
}
# plb_resizecanvas


# plb_open:
#
#
proc plb_open { w } {
global ay ayprefs

label $w.la -text "Properties:" -padx 0 -pady 0
pack $w.la -in $w -side top -fill x -expand no

bind $w.la <Double-ButtonPress-1> {
    global ay;
    $ay(plb) selection clear 0 end;
    plb_update
    resetFocus
}
balloon_set $w.la "Double click here\nto deselect property"

# the properties listbox
set f $w
listbox $f.li -width 14 -height 12 -yscrollcommand {global ay; $ay(pss) set}\
	-selectmode single -exportselection 0 -takefocus 0 -selectborderwidth 0

set ay(plb) $f.li

bind $f.li <<ListboxSelect>> {
    global ay pclip_omit pclip_omit_label sel

    set sel ""
    getSel sel
    if { ($sel == "") || ([llength $sel] > 1) } { break }

    set lb $ay(plb)
    if { [$lb size] < 1 } { break }
    set prop [$lb get [$lb curselection]]
    if { ! [info exists ${prop}] } {
	ayError 2 "Ayam" "Property '\${prop}' does not exist!"
	$lb selection clear 0 end
	plb_update
	break
    }
    eval [subst "set ww \$${prop}(w)"]

    set getprocp ""
    eval [subst "set getprocp \$${prop}(gproc)"]
    if { $getprocp != "" } { $getprocp } else { getProp }

    eval [subst "set arrname \$${prop}(arr)"]
    global pclip_reset $arrname
    array set pclip_reset  [array get $arrname]
    $ay(pca) itemconfigure 1 -window $ay(pca).$ww

    # resize canvas
    set width [expr [winfo reqwidth $ay(pca).$ww] + 10]
    set height [expr [winfo reqheight $ay(pca).$ww] + 10]
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

    if { $ay(lb) == 0 } {
	global ay; focus -force $ay(tree)
    } else {
	global ay; focus -force $ay(olb)
    }
    # improve focus traversal (speed-wise)
    global tcl_platform AYWITHAQUA
    if { $ay(lb) == 1 } {
	bind $ay(olb) <Key-Tab>\
		"focus [tk_focusNext $ay(pca).$ww];plb_focus;break"
	bind [tk_focusNext $ay(pca).$ww] <Shift-Tab>\
		"focus $ay(olb);break"
	if { ( $tcl_platform(platform) != "windows" ) && ( ! $AYWITHAQUA ) } {
	    bind [tk_focusNext $ay(pca).$ww] <ISO_Left_Tab>\
		    "focus $ay(olb);break"
	}
    } else {
	bind $ay(tree) <Key-Tab>\
		"focus [tk_focusNext $ay(pca).$ww];plb_focus;break"
	bind [tk_focusNext $ay(pca).$ww] <Shift-Tab>\
		"focus $ay(tree);break"
	if { ( $tcl_platform(platform) != "windows" ) && ( ! $AYWITHAQUA ) } {
	    bind [tk_focusNext $ay(pca).$ww] <ISO_Left_Tab>\
		    "focus $ay(tree);break"
	}
    }
    # if
}
# bind

# mouse-wheel bindings
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

global tcl_platform AYWITHAQUA
if { ($tcl_platform(platform) == "windows") || $AYWITHAQUA } {
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
    resetFocus
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
$m add separator
$m add command -label "Help on Property" -command {
    global ay
    set hm $ay(helpmenu)
    $hm invoke 2
}

if { $ay(ws) == "Aqua" && $ayprefs(SwapMB) } {
    bind $f.li <ButtonPress-2> "tk_popup $m %X %Y"
} else {
    bind $f.li <ButtonPress-3> "tk_popup $m %X %Y"
}

# plb scrollbar
set ay(pss) $f.s
scrollbar $f.s -takefocus 0 -command { global ay; $ay(plb) yview }

pack $f.li $f.s -in $f -side left -fill y -expand no


# the property arguments area
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

set ay(appb) $f.b1
set ay(bok) $ay(appb)

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

# create the canvas widget itself
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

# mouse-wheel bindings
bind . <ButtonPress-4> {
    global ay
    $ay(pca) yview scroll -1 pages
    break;
}

bind . <ButtonPress-5> {
    global ay
    $ay(pca) yview scroll 1 pages
    break;
}

global tcl_platform AYWITHAQUA
if { ($tcl_platform(platform) == "windows") } {
    bind . <MouseWheel> {
	global ay
	if { %D < 0.0 } {
	    $ay(pca) yview scroll 1 pages
	} else {
	    $ay(pca) yview scroll -1 pages
	}
	break;
    }
    # bind

    bind all <MouseWheel> {
	if { ([focus] != "") && ([focus] != ".fl.con.console") &&\
		 ([winfo toplevel [focus]] == ".") } {
	    global ay
	    if { %D < 0.0 } {
		$ay(pca) yview scroll 1 pages
	    } else {
		$ay(pca) yview scroll -1 pages
	    }
	    break;
	}
	# if
    }
    # bind
}
# if

if { $AYWITHAQUA } {
    bind . <MouseWheel> {
	global ay
	if { %D < 0.0 } {
	    $ay(pca) yview scroll 3 units
	} else {
	    $ay(pca) yview scroll -3 units
	}
	break;
    }
    # bind

    bind all <MouseWheel> {
	if { ([focus] != "") && ([focus] != ".fl.con.console") &&\
		 ([winfo toplevel [focus]] == ".") } {
	    global ay
	    if { %D < 0.0 } {
		$ay(pca) yview scroll 3 units
	    } else {
		$ay(pca) yview scroll -3 units
	    }
	    break;
	}
	# if
    }
    # bind
}
# if

# focus management bindings
bind $f.ca <1> "focus %W"
bind $f.ca <Key-Escape> "resetFocus;break"

pack $f.ca -in $f -side left -fill both -expand yes

scrollbar $f.s -command { global ay; $ay(pca) yview } -takefocus 0
set ay(pcas) $f.s
pack $f.s -in $f -side left -fill y


pack $w.fArg.fca -in $w.fArg -side top -fill both -expand yes
pack $w.fArg -in $w -side top -fill both -expand yes
update

return;
}
# plb_open


# plb_close:
#
#
proc plb_close { w } {
destroy $w.la
destroy $w.li
destroy $w.s
destroy $w.fArg
return;
}
# plb_close


# plb_update:
#
#
proc plb_update { } {
global ay ayprefs ay_error curtypes pclip_omit pclip_omit_label

# protect against double updates
if { $ay(PlbUpdateSema) == 1 } {
    return
} else {
    set ay(PlbUpdateSema) 1
}

set index ""

if { $ay(lb) == 1 } {
    set index [$ay(olb) curselection]
} else {
    set index [$ay(tree) selection get]
}

# avoid leaving the focus on an empty property canvas
if { [llength $index] == 0 } {
    resetFocus
}

set lb $ay(plb)
set oldsel ""
set oldsel [$lb curselection]

if { $oldsel == "" } {
    if { $ay(lb) == 1 } {
	set w $ay(olb)
    } else {
	set w $ay(tree)
    }
    if { $ayprefs(SingleWindow) == 1 } {
	bind $w <Key-Tab> "focus .fu.fMain.fview3;break"
	bind $w <Shift-Tab> "focus .fv.fViews.fview2;break"
    } else {
	bind $w <Key-Tab> "focus .fl.con.console;break"
    }
    $ay(pca) configure -takefocus 0
}

# delete current entries
$lb delete 0 end

# show new property GUI
$ay(pca) itemconfigure 1 -window $ay(pw)
if { [llength $index] == 1 } {
    set type ""
    if { $ay(lb) == 1 } {
	set type [lindex $curtypes $index]
    } else {
	set oldayerror $ay_error
	getType type
	set ay_error $oldayerror
    }

    if { $type == "" } {
	set ay(PlbUpdateSema) 0
	return;
    }

    if { $type != ".." } {
	global ${type}_props
	eval [subst "set props {\$${type}_props}"]
	# remove properties from RP tags
	set tn ""
	getTags tn tv
	if { ($tn != "") && ([ string first RP $tn ] != -1) } {
	    set i 0
	    foreach tag $tn {
		if { [lindex $tn $i] == "RP" } {
		    set val [lindex $tv $i]
		    set j 0
		    set nprops ""
		    foreach prop $props {
			if { $prop != $val } {
			    lappend nprops $prop
			}
			incr j
		    }
		    set props $nprops
		}
		incr i
	    }
	    # foreach
	}
	# if

	if { [llength $props] > 0 } {
	    eval [subst "$lb insert end $props"]
	}
	# insert properties from NP tags
	if { ($tn != "") && ([ string first NP $tn ] != -1) } {
	    set i 0
	    foreach tag $tn {
		if { [lindex $tn $i] == "NP" } {
		    $lb insert end [lindex $tv $i]
		}
		incr i
	    }
	    # foreach
	}
	# if
    }
    # if

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
	    $ay(pca) configure -takefocus 1

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
			"focus [tk_focusNext $ay(pca).$w];plb_focus;break"
		bind [tk_focusNext $ay(pca).$w] <Shift-Tab>\
			"focus $ay(olb);break"
		if { ( $tcl_platform(platform) != "windows" ) &&\
			( ! $AYWITHAQUA ) } {
		    bind [tk_focusNext $ay(pca).$w] <ISO_Left_Tab>\
			    "focus $ay(olb);break"
		}
	    } else {
		bind $ay(tree) <Key-Tab>\
		    "focus [tk_focusNext $ay(pca).$w];plb_focus;break"

		bind [tk_focusNext $ay(pca).$w] <Shift-Tab>\
		    "focus $ay(tree);break"
		if { ( $tcl_platform(platform) != "windows" ) &&\
			 ( ! $AYWITHAQUA ) } {
		    bind [tk_focusNext $ay(pca).$w] <ISO_Left_Tab>\
			"focus $ay(tree);break"
		}
	    }
	    # if
	}
	# if
    }
    # if

    # avoid leaving the focus on an empty property canvas
    if { $oldsel != "" && ( $oldsel > [$lb index end] ) } {
	resetFocus
    }

}
# if

set ay(PlbUpdateSema) 0

 return;
}
# plb_update


# plb_resize:
#
#
proc plb_resize { } {
    global ayprefs ay
    if { $ayprefs(AutoResize) != 1 } { return; }
    update
    set newwidth [expr [winfo width .fu.fMain.fHier] + [winfo reqwidth .fu.fMain.fProp] + [winfo reqwidth .fu.fMain.__h1] ]
   set newheight [winfo height .]

    set ng ${newwidth}x${newheight}
    set oldgeom [wm geom .]
    regexp {([0-9]+)?x?([0-9]+)?(\+)([0-9\-]+)?(\+)([0-9\-]+)?} $oldgeom blurb width height blurb2 x blurb3 y

    if { $newwidth <= $width } { return; }

    set vwidth [expr [winfo rootx .fu.fMain.fProp]]

    global tcl_platform ayprefs
    if { ($tcl_platform(platform) != "windows") &&\
	    ($ayprefs(TwmCompat) != 1) } {
	set x [winfo rootx .]
	set y [winfo rooty .]

	append ng "+$x"
	append ng "+$y"
    }
    wm geometry . ""
    wm geometry . $ng
    update
    pane_constrain . .fu.fMain.__h1 .fu.fMain.fHier .fu.fMain.fProp width x 1
    pane_motion $vwidth . .fu.fMain.__h1 width x 1

 return;
}
# plb_resize


# plb_focus:
#  this proc is bound to the Tab-key and scrolls the property
#  canvas to display the new item that gets the focus if it is
#  outside the current visible region of the property canvas
proc plb_focus { } {
    global ay
    set w [focus]
    if { $w != "" } {
	if { [string first $ay(pca) $w] != -1 } {
	    set ca $ay(pca)
	    set height [$ca cget -height]
	    set visible [$ca yview]
	    set ww $w
	    set wypos 0
	    incr wypos [winfo y $ww]
	    set increment 0
	    while { ($ww != "") && ([winfo parent $ww] != $ay(pca)) } {
		incr wypos $increment
		set ww [winfo parent $ww]
		if { $ww != "" } {
		    set increment [winfo y $ww]
		}
	    }
	    set fraction [expr (double($wypos)+[winfo reqheight $w])/\
	                       double($height)]
	    if { ($fraction < [lindex $visible 0]) ||
	         ($fraction > [lindex $visible 1]) } {
		     set fraction [expr double($wypos)/double($height)]
		     $ca yview moveto $fraction
	    }
	}
	# if
    }
    # if
 return;
}
# plb_focus


# plb_showprop:
#
#
proc plb_showprop { prop } {
    global ay

    set l $ay(plb)

    set len [$l size ]

    if { ($len < 1) || ($prop > $len) } {
	return;
    }

    $l selection clear 0 end

    update

    if { $prop == 0 } {
	$l selection set [expr $len - 1]
    } else {
	$l selection set [expr $prop - 1]
    }

    update

    event generate $l <<ListboxSelect>>

    if { $ay(lb) == 1 } {
	focus $ay(olb)
    } else {
	focus $ay(tree)
    }

 return;
}
# plb_showprop
