# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2005 by Randolf Schultz
# (randolf.schultz@gmail.com) and others.
#
# All rights reserved.
#
# See the file License for details.

# plb.tcl - procedures for managing the properties listbox and display

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
set ay(plbl) $w.la
balloon_set $w.la [ms plb_label]

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
	ayError 2 "Ayam" "Property \"${prop}\" does not exist!"
	$lb selection clear 0 end
	plb_update
	break
    }
    eval [subst "set ww \$${prop}(w)"]

    set getprocp ""
    eval [subst "set getprocp \$${prop}(gproc)"]
    if { $getprocp != "" } { $getprocp } else { getProp }

    # save current property data for the "Reset" button
    eval [subst "set arrname \$${prop}(arr)"]
    global pclip_reset $arrname
    catch {array unset pclip_reset}
    array set pclip_reset [array get $arrname]

    # make new property GUI visible in the canvas
    $ay(pca) itemconfigure 1 -window $ay(pca).$ww
    $ay(pca) configure -takefocus 1

    # resize canvas
    set width [expr [winfo reqwidth $ay(pca).$ww] + 10]
    set height [expr [winfo reqheight $ay(pca).$ww] + 10]
    $ay(pca) configure -width $width
    $ay(pca) configure -height $height

    # resize main window
    plb_resize

    # manage property clipboard
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
	focus -force $ay(tree)
    } else {
	focus -force $ay(olb)
    }

    # improve focus traversal (speed-wise)
    global tcl_platform AYWITHAQUA
    if { $ay(lb) == 1 } {
	bind $ay(olb) <Key-Tab>\
		"focus [tk_focusNext $ay(pca).$ww];plb_focus;break"
	bind [tk_focusNext $ay(pca).$ww] <Shift-Tab>\
		"focus $ay(olb);break"
	if { ( $tcl_platform(platform) != "windows" ) && ( ! $AYWITHAQUA ) } {
	    catch {bind [tk_focusNext $ay(pca).$ww] <ISO_Left_Tab>\
		       "focus $ay(olb);break"}
	}
    } else {
	bind $ay(tree) <Key-Tab>\
		"focus [tk_focusNext $ay(pca).$ww];plb_focus;break"
	bind [tk_focusNext $ay(pca).$ww] <Shift-Tab>\
		"focus $ay(tree);break"
	if { ( $tcl_platform(platform) != "windows" ) && ( ! $AYWITHAQUA ) } {
	    catch {bind [tk_focusNext $ay(pca).$ww] <ISO_Left_Tab>\
		       "focus $ay(tree);break"}
	}
    }
    # if

    after idle {$ay(pca) configure -scrollregion [$ay(pca) bbox all]}
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
$m add command -label "Add Property" -command "plb_addremprop"
$m add command -label "Remove Property" -command "plb_addremprop 1"

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
    eval [subst "set setprocp {\$${prop}(sproc)}"]
    if { $setprocp != "" } { eval $setprocp } else { setProp }
    set getprocp ""
    eval [subst "set getprocp {\$${prop}(gproc)}"]
    if { $getprocp != "" } { eval $getprocp } else { getProp }

    # save current property data for the "Reset" button
    eval [subst "set arrname \$${prop}(arr)"]
    global pclip_reset $arrname
    catch {array unset pclip_reset}
    array set pclip_reset [array get $arrname]
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

set ay(rstb) $f.b2

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
$f.ca create window 5 0 -anchor nw -window $ay(pw)
set width [expr [winfo reqwidth $ay(pw)] + 10]
$ay(pca) configure -width $width
$f.ca create line 0 0 2 0 -fill [$f.ca cget -background]

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
	if { ([focus] != "") && ([focus] != ".fl.con.console") &&
	     ([winfo toplevel [focus]] == ".") &&
	     ([string first "view" [focus]] == -1)} {
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
	if { ([focus] != "") && ([focus] != ".fl.con.console") &&
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

# the property GUI scrollbar
scrollbar $f.s -command { global ay; $ay(pca) yview } -takefocus 0
set ay(pcas) $f.s
pack $f.s -in $f -side right -fill y


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
    global tcl_platform AYWITHAQUA

    # protect against double updates
    if { $ay(plblock) == 1 } {
	return
    } else {
	set ay(plblock) 1
    }

    set index ""

    if { $ay(lb) == 1 } {
	set index [$ay(olb) curselection]
    } else {
	set index [$ay(tree) selection get]
    }

    if { $index == "" } {
	plb_bindtab
    }

    # avoid leaving the focus on an empty property canvas
    if { ( [string first $ay(pca) [focus]] == 0 ) &&
	 ( [llength $index] == 0 ) } {
	resetFocus
    }

    set lb $ay(plb)
    set oldsel ""
    set oldsel [$lb curselection]

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
	    set ay(plblock) 0
	    return;
	}

	if { $type != ".." } {
	    global ${type}_props

	    # get props of selected object
	    if { ! [info exists ${type}_props] } {
		# props are undefined yet, try to run the
		# object type specific init procedure...
		set ay(bok) $ay(appb)
		catch { eval init_${type} }
	    }

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

	    # add props to listbox
	    if { [llength $props] > 0 } {
		eval [subst "$lb insert end $props"]
	    }

	    # also add properties from NP tags
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
	if { $oldsel != ""} {
	    if { $oldsel <= [$lb index end] } {
		$lb selection set $oldsel
		set tmp [$lb curselection]
		if { $tmp != "" } {
		    event generate $lb <<ListboxSelect>>
		}
	    } else {
		# avoid leaving the focus on an empty property canvas
		resetFocus
	    }
	} else {
	    plb_bindtab
	}
	# if
    }
    # if

    after idle {$ay(pca) configure -scrollregion [$ay(pca) bbox all]}

    set ay(plblock) 0

 return;
}
# plb_update


# plb_resize:
#  resize the main window according to the currently selected property
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
    if { ($tcl_platform(platform) != "windows") &&
	 ($ayprefs(TwmCompat) != 1) } {
	set x [winfo rootx .]
	set y [winfo rooty .]

	append ng "+$x"
	append ng "+$y"
    }
    wm geometry . ""
    wm geometry . $ng
    update
    pane_constrain . .fu.fMain.__h1 .fu.fMain.fHier .fu.fMain.fProp \
	width x 1 25.0
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


# plb_setwin:
# helper for dynamic property GUIs
# set the canvas window and resize the canvas
# also adapts the scroll region and restores the focus
proc plb_setwin { w {fw ""} } {
    global ay

    if { ![winfo exists $w] } {
	return
    }

    # set new window
    $ay(pca) itemconfigure 1 -window $w
    update

    # resize canvas
    set width [expr [winfo reqwidth $w] + 10]
    set height [expr [winfo reqheight $w] + 10]
    $ay(pca) configure -width $width
    $ay(pca) configure -height $height

    # resize main window
    plb_resize

    # adapt scrollregion
    after idle {$ay(pca) configure -scrollregion [$ay(pca) bbox all]}

    # manage focus
    if { [winfo exists $fw] } {
	focus -force $fw
    }

 return;
}
# plb_setwin


# plb_addremprop:
# open a dialog where the user can add/remove properties
# (realized via NP/RP tags)
proc plb_addremprop { {rem 0} } {
    global ay ayprefs AddRemProp
    set selected ""
    getSel selected
    if { $selected == "" } { ayError 20 "addremprop" ""; return; }

    winAutoFocusOff

    set ay(addPropFocus) [focus]

    set w .addPropw
    if { $rem } {
	set title "Remove"
    } else {
	set title "Add"
    }
    append title " Property"
    winDialog $w $title

    if { ! [info exists AddRemProp] } {
	array set AddRemProp {
	    Property ""
	}
    }

    if { $rem == 1 } {
	set lb $ay(plb)
	if { [$lb size] > 1 } {
	    set AddRemProp(Property) [$lb get [$lb curselection]]
	}
    }

    set AddRemProp(Operation) $rem

    set f [frame $w.f1]
    pack $f -in $w

    set props ""
    global AllProps
    if { $AddRemProp(Operation) == 1 } {
	# remove operation, compile list of
	# candidates for removal
	array set AllProps {}
	forAll -recursive 0 {
	    global AllProps
	    getType type

	    if { ! [info exists ::${type}_props] } {
		eval ${type}_init
	    }
	    eval set props \$::${type}_props
	    foreach prop $props {
		set AllProps($prop) 1
	    }

	    set tagnames ""
	    set tagvalues ""
	    getTags tagnames tagvalues
	    set l [llength $tagnames]
	    for {set j 0} {$j < $l} {incr j} {
		set tagname [lindex $tagnames $j]
		if { $tagname == "NP" } {
		    set AllProps([lindex $tagvalues $j]) 1
		}
	    }
	    # for
	}
	# forAll
    } else {
	# add operation, compile list of
	# candidates for addition
	array set AllProps { Caps 1 Bevels 1}
	forAll -recursive 0 {
	    global AllProps
	    set tagnames ""
	    set tagvalues ""
	    getTags tagnames tagvalues
	    set l [llength $tagnames]
	    for {set j 0} {$j < $l} {incr j} {
		set tagname [lindex $tagnames $j]
		if { $tagname == "RP" } {
		    set AllProps([lindex $tagvalues $j]) 1
		}
	    }
	    # for
	}
	# forAll
    }
    # if

    set props [lsort [array names AllProps]]

    set ay(bca) $w.f3.bca
    set ay(bok) $w.f3.bok

    if { $ayprefs(FixDialogTitles) == 1 } {
	addText $f e1 $title
    }
    addString $f AddRemProp Property $props

    # buttons
    set f [frame $w.f3]
    button $f.bok -text "Ok" -pady $ay(pady) -width 5 -command {
	prop_addrem
	grab release .addPropw
	restoreFocus $ay(addPropFocus)
        destroy .addPropw
    }
    # ok

    button $f.bca -text "Cancel" -pady $ay(pady) -width 5 -command "\
	grab release $w;\
	restoreFocus $ay(addPropFocus);\
        destroy $w"

    pack $f.bok $f.bca -in $f -side left -fill x -expand yes
    pack $f -in $w -side bottom -fill x -expand no

    # Esc-key && close via window decoration == Cancel button
    bind $w <Escape> "$f.bca invoke"
    wm protocol $w WM_DELETE_WINDOW "$f.bca invoke"
    bind $w <Key-Return> "$f.bok invoke"
    catch { bind $w <Key-KP_Enter> "$f.bok invoke" }

    winRestoreOrCenter $w $title

    grab $w
    tkwait window $w

    winAutoFocusOn

 return;
}
# plb_addremprop


# plb_bindtab:
# bind <Tab>-key to skip the focus over an empty property canvas
#
proc plb_bindtab { } {
    global ay ayprefs
    if { $ay(lb) == 1 } {
	set w $ay(olb)
    } else {
	set w $ay(tree)
    }
    if { $ayprefs(SingleWindow) == 1 } {
	bind $w <Key-Tab> "focus .fu.fMain.fview3;break"
    } else {
	bind $w <Key-Tab> "focus .fl.con.console;break"
    }
    $ay(pca) configure -takefocus 0
 return;
}
# plb_bindtab
