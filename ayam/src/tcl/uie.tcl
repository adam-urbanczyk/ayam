# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2005 by Randolf Schultz
# (randolf.schultz@gmail.com) and others.
#
# All rights reserved.
#
# See the file License for details.

# User Interface Elements for Property GUIs, Preferences, Tool Dialogs,
# and Import/Export Dialogs

# uie_callhelp:
#  call context sensitive help of toplevel window
#  for the provided window
proc uie_callhelp { w } {
    global aymainshortcuts
    set w [winfo toplevel $w]
    set b ""
    catch {set b [bind $w [repctrl $aymainshortcuts(Help)]]}
    # use while to protect from 'called break outside of a loop'-error
    while { 1 } {
	if { $b != "" } {
	    eval $b
	}
	break
    }
 return;
}
# uie_callhelp


# uie_fixEntry:
#
#
proc uie_fixEntry { w } {
    global tcl_version tcl_platform aymainshortcuts

    if { ( $tcl_platform(platform) == "windows" ) && \
	 ( $tcl_version > 8.2 ) } {
	bind $w <Tab> {focus [tk_focusNext %W];plb_focus;break}
	bind $w <Shift-Tab> {focus [tk_focusPrev %W];plb_focus;break}
    }

    bind $w [repctrl $aymainshortcuts(Help)] "uie_callhelp %W"

 return;
}
# uie_fixEntry

#
#
#
proc updateParam { w prop name op } {
    global $prop

    set f $w.f${name}

    set oldval [$f.e get]

    if { [string is integer $oldval] || [string is double $oldval] } {

	set newval $oldval
	switch $op {
	    "*2" {
		set newval [expr $oldval * 2]
	    }
	    "/2" {
		set newval [expr $oldval / 2]
	    }
	    "p1" {
		set newval [expr $oldval + 1]
	    }
	    "m1" {
		set newval [expr $oldval - 1]
	    }
	    "p01" {
		set ta $oldval
		set tb 1.0
		set done 0
		if { [expr abs($oldval)] >= 1.0 &&
		     [expr abs($oldval)] <= 10.0 } {
		    if { [string is integer $oldval] } {
			set tb 1
		    } else {
			set tb 0.1
		    }
		    set done 1
		}
		while { ! $done } {
		    if { [expr abs($oldval)] <= 1.0 } {
			if { [expr abs(int($ta) - $ta)] == 0.0 } {
			    set done 1
			} else {
			    set tb [expr $tb/10.0]
			    set ta [expr $ta*10.0]
			}
		    } else {
			if { [expr abs($ta)] < 100.0 } {
			    set done 1
			} else {
			    set tb [expr $tb*10.0]
			    set ta [expr $ta/10.0]
			}
		    }
		}
		if { [string is integer $oldval] } {
		    set newval [expr int($oldval + $tb)]
		} else {
		    set newval [expr $oldval + $tb]
		}
	    }
	    "m01" {
		set ta $oldval
		set tb 1.0
		set done 0
		if { [expr abs($oldval)] >= 1.0 &&
		     [expr abs($oldval)] <= 10.0 } {
		    if { [string is integer $oldval] } {
			set tb 1
		    } else {
			set tb 0.1
		    }
		    set done 1
		}
		while { ! $done } {
		    if { [expr abs($oldval)] <= 1.0 } {
			if { [expr abs(int($ta) - $ta)] == 0.0 } {
			    set done 1
			} else {
			    set tb [expr $tb/10.0]
			    set ta [expr $ta*10.0]
			}
		    } else {
			if { [expr abs($ta)] < 100.0 } {
			    set done 1
			} else {
			    set tb [expr $tb*10.0]
			    set ta [expr $ta/10.0]
			}
		    }
		}
		if { [string is integer $oldval] } {
		    set newval [expr int($oldval - $tb)]
		} else {
		    set newval [expr $oldval - $tb]
		}
	    }
	}
	# switch op
	set ${prop}(${name}) $newval
    }
    # if isinteger || isdouble
 return;
}
# updateParam

#
#
#
proc addParamB { w prop name help {def {}} } {

    addParam $w $prop $name $def

    balloon_set $w.f${name}.l "${name}:\n${help}"

 return;
}
# addParamB

#
#
#
proc addParam { w prop name {def {}} } {
    global $prop ay ayprefs

    if { [winfo toplevel $w] == "." } {
	set escapecmd "resetFocus;break"
    } else {
	set escapecmd "after idle {$ay(bca) invoke}"
    }

    set bw 1

    set f [frame $w.f${name} -relief sunken -bd $bw]

    label $f.l -width 14 -text ${name}:

    if {[string length ${name}] > 12} {
	balloon_set $f.l ${name}
    }

    bind $f.l <Double-ButtonPress-1> "pclip_toggleomit $f.l $name"

    set e [entry $f.e -width 8 -textvariable ${prop}(${name}) -bd $bw]
    eval [subst "bindtags $f.e \{$f.e pge Entry all\}"]
    bind $f.e <Key-Escape> $escapecmd

    uie_fixEntry $f.e
    bind $f.e <Double-ButtonPress-1> "$f.e selection range 0 end; break"

    set font [$e cget -font]
    set wi [expr [font measure $font "WW"]]
    set he [expr [winfo reqheight $e] - 6]
    if { $ay(ws) == "Aqua" } {
	incr he -5
    }
    button $f.b1 -bd $bw -wi $wi -he $he -image ay_TriangleL_img\
	-command "updateParam $w $prop $name /2"\
	-takefocus 0 -highlightthickness 0

    button $f.b2 -bd $bw -wi $wi -he $he -image ay_TriangleR_img\
	-command "updateParam $w $prop $name *2"\
	-takefocus 0 -highlightthickness 0

    set mb ""
    if { $def != {} } {
	set mbs [expr [winfo reqheight $f.b2] - 2]
	if { $ay(ws) == "Aqua" } {
	    incr mbs -8
	}
	if { $ay(ws) == "Win32" } {
	    incr mbs 4
	}

	set mb [menubutton $f.b3 -height $mbs -width $mbs -bd $bw\
		    -image ay_Triangle_img -takefocus 0\
		    -highlightthickness 0 -relief raised -menu $f.b3.m]

	if { $ay(ws) == "Aqua" } {
	    $mb conf -height [$f.b2 cget -height]
	}

	set m [menu $mb.m -tearoff 0]
	foreach val $def {
	    $m add command -label $val\
		-command "global $prop; $e delete 0 end; $e insert end $val;"
	}
	bind $f.e <Key-F10> "$f.b3.m post \[winfo rootx $mb\]\
         \[winfo rooty $mb\];tk_menuSetFocus $f.b3.m;break"
    }

    bind $f.b1 <Control-ButtonPress-1> "updateParam $w $prop $name m01;break"
    bind $f.b2 <Control-ButtonPress-1> "updateParam $w $prop $name p01;break"

    bind $f.b1 <Alt-ButtonPress-1> "updateParam $w $prop $name m1;break"
    bind $f.b2 <Alt-ButtonPress-1> "updateParam $w $prop $name p1;break"

    if { ! $ay(iapplydisable) } {
	global aymainshortcuts
	bind $f.b1 <${aymainshortcuts(IApplyMod)}-ButtonRelease-1>\
	    "after idle {$ay(bok) invoke}"
	bind $f.b2 <${aymainshortcuts(IApplyMod)}-ButtonRelease-1>\
	    "after idle {$ay(bok) invoke}"
	if { $mb != "" } {
	    bind $mb <${aymainshortcuts(IApplyMod)}-ButtonRelease-1>\
		"after idle {$ay(bok) invoke}"
	}
    }

    bind $f.e <Key-Return> "$::ay(bok) invoke;break"
    catch {bind $f.e <Key-KP_Enter> "$::ay(bok) invoke;break"}

    if { $ay(ws) == "Win32" } {
	pack $f.l -in $f -side left
	pack $f.b1 -in $f -side left -pady 1 -fill both -expand no
	pack $f.e -in $f -side left -pady 1 -fill both -expand yes
	pack $f.b2 -in $f -side left -pady 1 -fill both -expand no
	if { $mb != "" } { pack $mb -side left -pady 1 -fill both -expand no}
    } else {
	if { $ay(ws) != "Aqua" } {
	    $f.b1 configure -highlightthickness 1
	    $f.b2 configure -highlightthickness 1
	    if { $mb != "" } { $mb configure -highlightthickness 1 }
	}
	pack $f.l -in $f -side left -fill x -expand no
	pack $f.b1 -in $f -side left -fill x -expand no
	pack $f.e -in $f -side left -fill x -expand yes
	pack $f.b2 -in $f -side left -fill x -expand no
	if { $mb != "" } {
	    pack $mb -side left -fill x -expand no;
	    if { $ay(ws) == "Aqua" } {
		pack conf $mb -anchor n
	    }
	}
    }
    pack $f -in $w -side top -fill x

 return;
}
# addParam

#
#
#
proc addParamPair { w prop name {def {}} } {
    set fdef {}
    set sdef {}
    foreach i $def {
	lappend fdef [lindex $i 0]
	lappend sdef [lindex $i 1]
    }
    addParam $w $prop ${name}_0 $fdef
    addParam $w $prop ${name}_1 $sdef
 return;
}
# addParamPair

#
#
#
proc addMatrixB { w prop name help } {

    addMatrix $w $prop $name

    balloon_set $w.f${name}.l "${name}:\n${help}"

 return;
}
# addMatrixB

#
#
#
proc addMatrix { w prop name } {
    global $prop ay ayprefs

    if { [winfo toplevel $w] == "." } {
	set escapecmd "resetFocus;break"
    } else {
	set escapecmd "after idle {$ay(bca) invoke}"
    }

    set bw 1

    set f [frame $w.f${name} -relief sunken -bd $bw]

    label $f.l -text ${name}:

    for { set i 0 } { $i < 16 } { incr i } {
	lappend omitl ${name}_$i
    }

    bind $f.l <Double-ButtonPress-1> "pclip_toggleomit $f.l { $omitl }"
    pack $f.l -in $f
    set f1 [frame $w.f${name}1 -relief sunken -bd $bw]
    for { set i 0 } { $i < 4 } { incr i } {
	entry $f1.e$i -width 6 -textvariable ${prop}(${name}_${i}) -bd $bw
	eval [subst "bindtags $f1.e$i \{$f1.e$i pge Entry all\}"]
	bind $f1.e$i <Key-Escape> $escapecmd
	uie_fixEntry $f1.e$i
	pack $f1.e$i -in $f1 -side left -fill both -expand yes
    }
    set f2 [frame $w.f${name}2 -relief sunken -bd $bw]
    for { set i 4 } { $i < 8 } { incr i } {
	entry $f2.e$i -width 6 -textvariable ${prop}(${name}_${i}) -bd $bw
	eval [subst "bindtags $f2.e$i \{$f2.e$i pge Entry all\}"]
	bind $f2.e$i <Key-Escape> $escapecmd
	uie_fixEntry $f2.e$i
	pack $f2.e$i -in $f2 -side left -fill both -expand yes
    }
    set f3 [frame $w.f${name}3 -relief sunken -bd $bw]
    for { set i 8 } { $i < 12 } { incr i } {
	entry $f3.e$i -width 6 -textvariable ${prop}(${name}_${i}) -bd $bw
	eval [subst "bindtags $f3.e$i \{$f3.e$i pge Entry all\}"]
	bind $f3.e$i <Key-Escape> $escapecmd
	uie_fixEntry $f3.e$i
	pack $f3.e$i -in $f3 -side left -fill both -expand yes
    }
    set f4 [frame $w.f${name}4 -relief sunken -bd $bw]
    for { set i 12 } { $i < 16 } { incr i } {
	entry $f4.e$i -width 6 -textvariable ${prop}(${name}_${i}) -bd $bw
	eval [subst "bindtags $f4.e$i \{$f4.e$i pge Entry all\}"]
	bind $f4.e$i <Key-Escape> $escapecmd
	uie_fixEntry $f4.e$i
	pack $f4.e$i -in $f4 -side left -fill both -expand yes
    }

    pack $f $f1 $f2 $f3 $f4 -in $w -side top -fill x -expand yes

 return;
}
# addMatrix

#
#
#
proc updateColor { w prop name button } {
    global $prop

    winAutoFocusOff

    set rname ${prop}(${name}_R)
    set gname ${prop}(${name}_G)
    set bname ${prop}(${name}_B)

    set newcolor [tk_chooseColor -parent [winfo toplevel $w]\
		      -initialcolor [$button cget -background]]
    if { $newcolor != "" } {
	$button configure -background $newcolor

	set label [string replace $button end-1 end l2]
	if { [winfo exists $label] } {
	    $label configure -background $newcolor
	}

	scan $newcolor "#%2x%2x%2x" r g b
	set $rname $r
	set $gname $g
	set $bname $b
    }

    winAutoFocusOn

 return;
}
# updateColor

#
#
#
proc updateColorFromE { w prop name button } {
    global $prop

    set rname ${prop}(${name}_R)
    set gname ${prop}(${name}_G)
    set bname ${prop}(${name}_B)

    set red [subst "\$$rname"]
    set green [subst "\$$gname"]
    set blue [subst "\$$bname"]

    # clamp colorvalues to correct range
    if { $red < -1 } { set $rname 0; set red 0 }
    if { $red > 255 } { set $rname 255; set red 255 }

    if { $green < -1 } { set $gname 0; set green 0 }
    if { $green > 255 } { set $gname 255; set green 255 }

    if { $blue < -1 } { set $bname 0; set blue 0 }
    if { $blue > 255 } { set $bname 255; set blue 255 }
    if { ($red == -1) || ($green == -1) || ($blue == -1) } {
	set newcolor black
    } else {
	set newcolor [format "#%02x%02x%02x" $red $green $blue]
    }
    $button configure -background $newcolor

    set label [string replace $button end-1 end l2]
    if { [winfo exists $label] } {
	$label configure -background $newcolor
    }

 return;
}
# proc updateColorFromE

#
#
#
proc addColorB { w prop name help {def {}}} {

    addColor $w $prop $name $def

    balloon_set $w.fl${name}.l "${name}:\n${help}"

 return;
}
# addColorB

#
#
#
proc addColor { w prop name {def {}}} {
    global $prop ay ayprefs aymainshortcuts

    if { [winfo toplevel $w] == "." } {
	set escapecmd "resetFocus;break"
    } else {
	set escapecmd "after idle {$ay(bca) invoke}"
    }

    set bw 1

    # first, we create a label on its own line
    set f [frame $w.fl${name} -relief sunken -bd $bw]

    label $f.l -text $name
    bind $f.l <Double-ButtonPress-1> "pclip_toggleomit $f.l {${name}_R\
	    ${name}_G ${name}_B }"
    pack $f.l -in $f -side top
    pack $f -in $w -side top -fill x -expand yes

    # now the color entries
    set f [frame $w.f${name}]

    set e1 [entry $f.er -width 4 -textvariable ${prop}(${name}_R) -bd $bw]
    eval [subst "bindtags $f.er \{$f.er pge Entry all\}"]
    bind $e1 <Key-Escape> $escapecmd
    uie_fixEntry $e1
    set e2 [entry $f.eg -width 4 -textvariable ${prop}(${name}_G) -bd $bw]
    eval [subst "bindtags $f.eg \{$f.eg pge Entry all\}"]
    bind $e2 <Key-Escape> $escapecmd
    uie_fixEntry $e2
    set e3 [entry $f.eb -width 4 -textvariable ${prop}(${name}_B) -bd $bw]
    eval [subst "bindtags $f.eb \{$f.eb pge Entry all\}"]
    bind $e3 <Key-Escape> $escapecmd
    uie_fixEntry $e3

    bind $e1 <FocusOut> "updateColorFromE $w $prop $name $f.b1"
    bind $e2 <FocusOut> "updateColorFromE $w $prop $name $f.b1"
    bind $e3 <FocusOut> "updateColorFromE $w $prop $name $f.b1"

    set red [subst \$${prop}(${name}_R)]
    set green [subst \$${prop}(${name}_G)]
    set blue [subst \$${prop}(${name}_B)]

    if { ($red <= 255) && ($red >= 0) && ($green <= 255) && ($green >= 0)\
	    && ($blue <= 255) && ($blue >= 0)} {
	set bcolor [format "#%02x%02x%02x" $red $green $blue]
    } else {
	set bcolor "#000000"
    }

    if { $ay(ws) == "Win32" } {
	button $f.b1 -pady 1 -background $bcolor\
		-command "updateColor $w $prop $name $f.b1"\
		-bd $bw -width 3
    } else {
	if { $ay(ws) == "Aqua" } {
	    label $f.l2 -background $bcolor -text "   "
	    button $f.b1 -pady 1 -padx 6 -text Set\
		-command "updateColor $w $prop $name $f.b1"\
		-bd $bw
	} else {
	    # X11
	    button $f.b1 -pady 1 -background $bcolor\
		-command "updateColor $w $prop $name $f.b1"\
		-bd $bw
	}
    }
    # if

    eval [subst "bindtags $f.b1 \{$f.b1 pge Button all\}"]
    bind $f.b1 <Key-Escape> $escapecmd
    bind $f.b1 [repctrl $aymainshortcuts(Help)] "uie_callhelp %W"
    bind $f.b1 <Visibility> "updateColorFromE $w $prop $name $f.b1"

    set mb ""
    if { $def != {} } {
	set mbs [expr [winfo reqheight $f.b1] - 2]
	set mb [menubutton $f.b3 -height $mbs -width $mbs -bd $bw\
		    -image ay_Triangle_img -takefocus 0\
		    -highlightthickness 0 -relief raised -menu $f.b3.m]
	if { $ay(ws) == "Win32" } {
	    $mb configure -pady 0
	}
	if { $ay(ws) == "Aqua" } {
	    $mb configure -height 14
	}
	set m [menu $mb.m -tearoff 0]
	foreach val $def {
	    $m add command -label "$val"\
	     -command "global $prop;\
	     $e1 delete 0 end;\
	     $e1 insert end [lindex $val 0];\
	     $e2 delete 0 end;\
	     $e2 insert end [lindex $val 1];\
	     $e3 delete 0 end;\
	     $e3 insert end [lindex $val 2];\
	     updateColorFromE $w $prop $name $f.b1"
	}
	# foreach
	bind $f <Key-F10> "$f.b3.m post \[winfo rootx $mb\]\
         \[winfo rooty $mb\];tk_menuSetFocus $f.b3.m;break"
    }
    # if

    if { ! $ay(iapplydisable) } {
	global aymainshortcuts
	if { $mb != "" } {
	    bind $mb <${aymainshortcuts(IApplyMod)}-ButtonRelease-1>\
		"after idle {$ay(bok) invoke}"
	}
	bind $f.b1 <${aymainshortcuts(IApplyMod)}-ButtonPress-1>\
		"updateColor $w $prop $name $f.b1;\
		 after idle {$ay(bok) invoke};break;"
    }

    set okcmd "updateColorFromE $w $prop $name $f.b1;$ay(bok) invoke;break"
    bind $e1 <Key-Return> $okcmd
    catch {bind $e1 <Key-KP_Enter> $okcmd}
    bind $e2 <Key-Return> $okcmd
    catch {bind $e2 <Key-KP_Enter> $okcmd}
    bind $e3 <Key-Return> $okcmd
    catch {bind $e3 <Key-KP_Enter> $okcmd}

    if { [winfo exists $f.l2] } {
	pack $f.er $f.eg $f.eb $f.l2 $f.b1 -in $f -fill both -expand yes\
		-side left -padx 2
    } else {
	pack $f.er $f.eg $f.eb $f.b1 -in $f -fill both -expand yes\
		-side left -padx 2
    }

    if { $mb != "" } { pack $mb -side left -fill both -expand yes}

    pack $f -in $w.fl${name} -side bottom -fill x

 return;
}
# addColor

#
#
#
proc addCheckB { w prop name help } {

    addCheck $w $prop $name

    balloon_set $w.f${name}.l "${name}:\n${help}"

 return;
}
# addCheckB

#
#
#
proc addCheck { w prop name } {
    global $prop ay ayprefs aymainshortcuts

    if { [winfo toplevel $w] == "." } {
	set escapecmd "resetFocus;break"
    } else {
	set escapecmd "after idle {$ay(bca) invoke}"
    }

    set bw 1
    set ws ""

    set f [frame $w.f${name} -relief sunken -bd $bw]

    label $f.l -width 14 -text ${name}:
    bind $f.l <Double-ButtonPress-1> "pclip_toggleomit $f.l $name"

    if {[string length ${name}] > 12} {
	balloon_set $f.l ${name}
    }

    if { $ay(ws) == "Win32" } {
	# damn windows
	set ff [frame $f.fcb -highlightthickness 1]
	set cb [checkbutton $ff.cb -image emptyimg -variable ${prop}(${name})\
		    -bd $bw -indicatoron 0 -selectcolor #b03060]

	bind $ff <Enter> { %W configure -background #ececec }
	bind $ff <Leave> { %W configure -background SystemButtonFace }
	bind $ff <1> { %W.cb invoke }

	pack $f.l -in $f -side left
	pack $f.fcb -in $f -side left -expand yes -fill both
	pack $ff.cb -in $ff -side top -padx 30 -pady 3

	eval [subst "bindtags $ff.cb \{$ff.cb pge Checkbutton all\}"]
	bind $ff.cb <Key-Escape> $escapecmd
	bind $ff.cb [repctrl $aymainshortcuts(Help)] "uie_callhelp %W"
    } else {
	if { $ay(ws) == "Aqua" } {
	    # also Aqua gets its "Extrawurst"
	    set ff [frame $f.fcb -highlightthickness 1]
	    set cb [checkbutton $ff.cb -variable ${prop}(${name}) -bd $bw]
	    bind $ff <1> { %W.cb invoke }
	    pack $f.l -in $f -side left
	    pack $f.fcb -in $f -side left -expand yes -fill both
	    pack $ff.cb -in $ff -side top -padx 30 -pady 0

	    eval [subst "bindtags $ff.cb \{$ff.cb pge Checkbutton all\}"]
	    bind $ff.cb <Key-Escape> $escapecmd
	    bind $ff.cb [repctrl $aymainshortcuts(Help)] "uie_callhelp %W"
	} else {
	    # generic (X11) implementation
	    set cb [checkbutton $f.cb -variable ${prop}(${name}) -bd $bw\
		    -pady 1 -padx 30]
	    pack $f.l -in $f -side left
	    pack $f.cb -in $f -side left -fill x -expand yes

	    eval [subst "bindtags $f.cb \{$f.cb pge Checkbutton all\}"]
	    bind $f.cb <Key-Escape> $escapecmd
	    bind $f.cb [repctrl $aymainshortcuts(Help)] "uie_callhelp %W"
	}
    }

    if { ! $ay(iapplydisable) } {
	global aymainshortcuts
	bind $cb <${aymainshortcuts(IApplyMod)}-ButtonRelease-1> "after idle {\
	    $ay(bok) invoke}"

	if { $ay(ws) == "Win32" || $ay(ws) == "Aqua" } {
	    bind $ff <${aymainshortcuts(IApplyMod)}-ButtonRelease-1>\
		"after idle {$ay(bok) invoke}"
	}
    }

    bind $cb <Key-Return> "$ay(bok) invoke;break"
    catch {bind $cb <Key-KP_Enter> "$ay(bok) invoke;break"}

    pack $f -in $w -side top -fill x

 return;
}
# addCheck

#
#
#
proc updateMenu { m name1 name2 op } {
    global ${name1}
    $m invoke [subst \$${name1}($name2)]
 return;
}
# updateMenu

#
#
#
proc addMenuB { w prop name help elist } {

    addMenu $w $prop $name $elist

    balloon_set $w.f${name}.l "${name}:\n${help}"

 return;
}
# addMenuB

#
#
#
proc addMenu { w prop name elist } {
    global $prop ay ayprefs aymainshortcuts

    if { [winfo toplevel $w] == "." } {
	set escapecmd "resetFocus;break"
    } else {
	set escapecmd "after idle {$ay(bca) invoke}"
    }

    set bw 1

    set f [frame $w.f${name} -relief sunken -bd $bw]

    label $f.l -width 14 -text ${name}:
    bind $f.l <Double-ButtonPress-1> "pclip_toggleomit $f.l $name"

    if {[string length ${name}] > 12} {
	balloon_set $f.l ${name}
    }

    set l 0
    foreach i $elist {
	if { [string length $i] > $l } {
	    set l [string length $i]
	}
    }

    menubutton $f.mb -text Eimer -menu $f.mb.m -relief raised -bd $bw\
	-padx 0 -pady 1 -takefocus 1 -width $l -highlightthickness 1\
	-indicatoron 1

    eval [subst "bindtags $f.mb \{$f.mb pge Menubutton all\}"]
    bind $f.mb <Key-Escape> $escapecmd
    bind $f.mb <Key-Return> "$::ay(bok) invoke;break"
    catch {bind $f.mb <Key-KP_Enter> "$::ay(bok) invoke;break"}

    bind $f.mb [repctrl $aymainshortcuts(Help)] "uie_callhelp %W"

    if { $ay(ws) == "Win32" } {
	$f.mb configure -pady 1
    }

    if { $ay(ws) == "Aqua" } {
	$f.mb configure -pady 2 -width 1
    }

    set m [menu $f.mb.m -tearoff 0]

    set val 0

    foreach i $elist {
	$m add command -label $i -command "global ${prop};\
		catch {set ${prop}($name) $val};\
		$f.mb configure -text {$i}"
	incr val
    }

    $m invoke [subst \$${prop}($name)]

    trace variable ${prop}($name) w "updateMenu $m"

    bind $f.mb <Destroy> "trace vdelete ${prop}($name) w \"updateMenu $m\""

    pack $f.l -in $f -side left -fill x
    pack $f.mb -in $f -side left -fill x -expand yes -pady 0
    pack $f -in $w -side top -fill x

    if { ! $ay(iapplydisable) } {
	global aymainshortcuts
	bind $f.mb <${aymainshortcuts(IApplyMod)}-ButtonRelease-1>\
	    "after idle {$ay(bok) invoke}"
    }

 return;
}
# addMenu

#
#
#
proc addStringB { w prop name help {def {}} } {

    addString $w $prop $name $def

    balloon_set $w.f${name}.l "${name}:\n${help}"

 return;
}
# addStringB

#
#
#
proc addString { w prop name  {def {}}} {
    global $prop ay ayprefs

    if { [winfo toplevel $w] == "." } {
	set escapecmd "resetFocus;break"
    } else {
	set escapecmd "after idle {$ay(bca) invoke}"
    }

    set bw 1

    set f [frame $w.f${name} -relief sunken -bd $bw]

    label $f.l -width 14 -text ${name}:
    bind $f.l <Double-ButtonPress-1> "pclip_toggleomit $f.l $name"

    if {[string length ${name}] > 12} {
	balloon_set $f.l ${name}
    }

    set e [entry $f.e -textvariable ${prop}(${name}) -width 15 -bd $bw]
    eval [subst "bindtags $f.e \{$f.e pge Entry all\}"]
    bind $f.e <Key-Escape> $escapecmd
    uie_fixEntry $f.e

    bind $f.e <Key-Return> "$ay(bok) invoke;break"
    catch {bind $f.e <Key-KP_Enter> "$ay(bok) invoke;break"}

    set mb ""
    if { $def != {} } {
	set mbs [expr [winfo reqheight $f.e] - 2]
	set mb [menubutton $f.b3 -height $mbs -width $mbs -bd $bw\
		    -image ay_Triangle_img -takefocus 0\
		-highlightthickness 0 -relief raised -menu $f.b3.m]

	if { $ay(ws) == "Win32" } {
	    $mb configure -pady 0
	}

	if { $ay(ws) == "Aqua" } {
	    $mb configure -height 14
	}

	set m [menu $mb.m -tearoff 0]
	foreach val $def {
	    $m add command -label $val\
	      -command "global $prop; $e delete 0 end; $e insert end \{$val\};"
	}

	if { ! $ay(iapplydisable) } {
	    global aymainshortcuts
	    bind $mb <${aymainshortcuts(IApplyMod)}-ButtonRelease-1>\
		"after idle {$ay(bok) invoke}"
	}
	bind $f.e <Key-F10> "$f.b3.m post \[winfo rootx $mb\]\
         \[winfo rooty $mb\];tk_menuSetFocus $f.b3.m;break"
    }

    pack $f.l -in $f -side left -fill x
    pack $f.e -in $f -side left -fill both -expand yes
    if { $mb != "" } { pack $mb -side left -fill both -expand no}

    pack $f -in $w -side top -fill x

 return;
}
# addString

#
#
#
proc entryViewEnd { w } {
    $w icursor end;
    set c [$w index insert]
    set left [$w index @0]
    if {$left > $c} {
	$w xview $c
	return
    }
    set x [winfo width $w]
    if {$c > [$w index @[winfo width $w]]} {
	$w xview insert
    }
 return;
}
# entryViewEnd

#
#
#
proc addFileTB { w prop name ftypes help {def {}} } {

    addFileT $w $prop $name $ftypes $def

    balloon_set $w.f${name}.l "${name}:\n${help}"

 return;
}
# addFileTB

#
#
#
proc addFileT { w prop name ftypes {def {}} } {

    addFile $w $prop $name $def
    set f $w.f${name}
    $f.b configure -command "
	    global $prop ay;
            winAutoFocusOff;
	    set filen \[$f.e get\];
            if { \$ay(ws) != \"Aqua\" } {
	      set filen \[tk_getOpenFile -filetypes {$ftypes}\
                          -parent [winfo toplevel $w]\
	                  -title \"Set File:\"];
            } else {
	      set filen \[tk_getOpenFile -filetypes {$ftypes}\
	                  -title \"Set File:\"];
            };
            winAutoFocusOn;
	    if { \$filen != \"\" } {
		$f.e delete 0 end;
		$f.e insert 0 \$filen;
		entryViewEnd $f.e;
		set ${prop}($name) \$filen;
	    }"

 return;
}
# addFileT

#
#
#
proc addSFileT { w prop name ftypes {def {}} } {

    addFile $w $prop $name $def
    set f $w.f${name}
    $f.b configure -command "\
	    global $prop ay;
            winAutoFocusOff;
	    set filen \[$f.e get\];
            if { \$ay(ws) != \"Aqua\" } {
              set filen \[tk_getSaveFile -filetypes {$ftypes}\
                          -parent [winfo toplevel $w]\
		          -title \"Set File:\"];
            } else {
              set filen \[tk_getSaveFile -filetypes {$ftypes}\
		          -title \"Set File:\"];
            };
            winAutoFocusOn;
	    if { \$filen != \"\" } {
		$f.e delete 0 end;
		$f.e insert 0 \$filen;
		entryViewEnd $f.e;
		set ${prop}($name) \$filen;
	    }"

 return;
}
# addSFileT

#
#
#
proc addSFile { w prop name {def {}} } {

    addFile $w $prop $name $def
    set f $w.f${name}
    $f.b configure -command "\
	    global $prop ay;
            winAutoFocusOff;
	    set filen \[$f.e get\];
            if { \$ay(ws) != \"Aqua\" } {
              set filen \[tk_getSaveFile -parent [winfo toplevel $w]\
		          -title \"Set File:\"];
            } else {
              set filen \[tk_getSaveFile -title \"Set File:\"];
            };
            winAutoFocusOn;
	    if { \$filen != \"\" } {
		$f.e delete 0 end;
		$f.e insert 0 \$filen;
		entryViewEnd $f.e;
		set ${prop}($name) \$filen;
	    }"

 return;
}
# addSFile

#
#
#
proc addSFileB { w prop name help {def {}} } {

    addSFile $w $prop $name $def

    balloon_set $w.f${name}.l "${name}:\n${help}"

 return;
}
# addSFileB

#
#
#
proc addFileB { w prop name help {def {}} } {

    addFile $w $prop $name $def

    balloon_set $w.f${name}.l "${name}:\n${help}"

 return;
}
# addFileB

#
#
#
proc addFile { w prop name {def {}} } {
    global $prop ay ayprefs

    if { [winfo toplevel $w] == "." } {
	set escapecmd "resetFocus;break"
    } else {
	set escapecmd "after idle {$ay(bca) invoke}"
    }

    set bw 1

    set f [frame $w.f${name} -relief sunken -bd $bw]

    label $f.l -width 14 -text ${name}:
    bind $f.l <Double-ButtonPress-1> "pclip_toggleomit $f.l $name"

    if {[string length ${name}] > 12} {
	balloon_set $f.l ${name}
    }

    set e [entry $f.e -textvariable ${prop}(${name}) -width 15 -bd $bw]
    eval [subst "bindtags $f.e \{$f.e pge Entry all\}"]
    bind $f.e <Key-Escape> $escapecmd
    uie_fixEntry $f.e
    after idle "entryViewEnd $f.e"

    bind $f.e <Key-Return> "$::ay(bok) invoke;break"
    catch {bind $f.e <Key-KP_Enter> "$::ay(bok) invoke;break"}

    button $f.b -text "Set" -width 4 -bd $bw -padx 0 -pady 0 -takefocus 0\
     -command "\
	global $prop ay;
        winAutoFocusOff;
	set filen \[$f.e get\];
        if { \$ay(ws) != \"Aqua\" } {
  	  set filen \[tk_getOpenFile -parent [winfo toplevel $w]\
                       -title \"Set File:\"];
        } else {
  	  set filen \[tk_getOpenFile -title \"Set File:\"];
        };
        winAutoFocusOn;
	if { \$filen != \"\" } {
	    $f.e delete 0 end;
	    $f.e insert 0 \$filen;
	    entryViewEnd $f.e;
	    set ${prop}($name) \$filen;
        }"

    if { $ay(ws) == "Aqua" } {
	$f.b configure -padx 6
    }

    set mb ""
    if { $def != {} } {
	set mbs [expr [winfo reqheight $f.b] - 2]
	set mb [menubutton $f.b3 -height $mbs -width $mbs -bd $bw\
		    -image ay_Triangle_img -takefocus 0\
		    -highlightthickness 0 -relief raised -menu $f.b3.m]
	if { $ay(ws) == "Win32" } {
	    $mb configure -pady 0
	}
	if { $ay(ws) == "Aqua" } {
	    $f.b3 configure -pady 2
	    $mb configure -height 14
	}
	set m [menu $mb.m -tearoff 0]
	foreach val $def {
	    $m add command -label $val\
		    -command "global $prop; $e delete 0 end;
	                      $e insert end \{$val\};"
	}
	# foreach
	bind $f.e <Key-F10> "$f.b3.m post \[winfo rootx $mb\]\
         \[winfo rooty $mb\];tk_menuSetFocus $f.b3.m;break"
    }
    # if

    pack $f.l -in $f -side left -fill x -expand no
    pack $f.e -in $f -side left -fill both -expand yes
    pack $f.b -in $f -side left -fill x -expand no

    if { $mb != "" } {
	if { $ay(ws) == "Win32" } {
	    pack $mb -side right -fill both -expand no
	} else {
	    pack $mb -side right -fill x -expand no
	}
    }

    pack $f -in $w -side top -fill x

 return;
}
# addFile

#
#
#
proc addMDirB { w prop name help } {

    addMDir $w $prop $name

    balloon_set $w.f${name}.l "${name}:\n${help}"

 return;
}
# addMDirB

#
#
#
proc addMDir { w prop name } {
    global $prop ay ayprefs

    if { [winfo toplevel $w] == "." } {
	set escapecmd "resetFocus;break"
    } else {
	set escapecmd "after idle {$ay(bca) invoke}"
    }

    set bw 1

    set f [frame $w.f${name} -relief sunken -bd $bw]

    label $f.l -width 14 -text ${name}:
    bind $f.l <Double-ButtonPress-1> "pclip_toggleomit $f.l $name"

    if {[string length ${name}] > 12} {
	balloon_set $f.l ${name}
    }

    entry $f.e -textvariable ${prop}(${name}) -width 15 -bd $bw
    eval [subst "bindtags $f.e \{$f.e pge Entry all\}"]
    bind $f.e <Key-Escape> $escapecmd
    uie_fixEntry $f.e

    bind $f.e <Key-Return> "$::ay(bok) invoke;break"
    catch {bind $f.e <Key-KP_Enter> "$::ay(bok) invoke;break"}

    bind $f.e <1> "+balloon_setsplit $f.e \[$f.e get\] 15"
    eval balloon_setsplit $f.e  \$${prop}(${name}) 15

    button $f.b -text "Add" -width 4 -bd $bw -padx 0 -pady 0 -takefocus 0\
     -command "\
	global $prop ay;
        winAutoFocusOff;
        if { \$ay(ws) != \"Aqua\" } {
	  set dir \[tk_chooseDirectory -parent [winfo toplevel $w]\
                     -initialdir \[pwd\]\];
        } else {
	  set dir \[tk_chooseDirectory\];
        };
        winAutoFocusOn;
	if { \${dir} != \"\" } {
	  if { \$${prop}($name) != \"\" } {
	      set sep \$ay(separator);
	      set ${prop}($name) \$${prop}($name)\${sep}\${dir};
	  } else {
	      set ${prop}($name) \${dir};
	  };
	  entryViewEnd $f.e;
	  update;
	  eval balloon_setsplit $f.e \[list \$${prop}($name)\] 15;
	};"

    if { $ay(ws) == "Aqua" } {
	$f.b configure -padx 6
    }

    pack $f.l -in $f -side left -fill x -expand no
    pack $f.e -in $f -side left -fill both -expand yes
    pack $f.b -in $f -side left -fill x -expand no
    pack $f -in $w -side top -fill x

 return;
}
# addMDir

#
#
#
proc addMFileB { w prop name help } {

    addMFile $w $prop $name

    balloon_set $w.f${name}.l "${name}:\n${help}"

 return;
}
# addMFileB

#
#
#
proc addMFile { w prop name } {
    global $prop ay ayprefs

    if { [winfo toplevel $w] == "." } {
	set escapecmd "resetFocus;break"
    } else {
	set escapecmd "after idle {$ay(bca) invoke}"
    }

    set bw 1

    set f [frame $w.f${name} -relief sunken -bd $bw]

    label $f.l -width 14 -text ${name}:
    bind $f.l <Double-ButtonPress-1> "pclip_toggleomit $f.l $name"

    if {[string length ${name}] > 12} {
	balloon_set $f.l ${name}
    }

    entry $f.e -textvariable ${prop}(${name}) -width 15 -bd $bw
    eval [subst "bindtags $f.e \{$f.e pge Entry all\}"]
    bind $f.e <Key-Escape> $escapecmd
    uie_fixEntry $f.e

    bind $f.e <Key-Return> "$::ay(bok) invoke;break"
    catch {bind $f.e <Key-KP_Enter> "$::ay(bok) invoke;break"}

    bind $f.e <1> "+balloon_setsplit $f.e \[$f.e get\] 15"
    eval balloon_setsplit $f.e \$${prop}(${name}) 15

    button $f.b -text "Add" -width 4 -bd $bw -padx 0 -pady 0 -takefocus 0\
     -command "\
	global $prop ay;
        winAutoFocusOff;
	set filen \[$f.e get\];
	global ay;
	set sep \$ay(separator);
        if { \$ay(ws) != \"Aqua\" } {
	  set filen \[tk_getOpenFile -parent [winfo toplevel $w]\
                     -title \"Select File:\"\];
        } else {
	  set filen \[tk_getOpenFile -title \"Select File:\"\];
        };
        winAutoFocusOn;
	if { \$filen != \"\" } {
	 if { \$${prop}($name) != \"\" } {
          set ${prop}($name) \$${prop}($name)\$sep\$filen;
	 } else {
	  set ${prop}($name) \$filen;
         };
	 entryViewEnd $f.e;
	 update;
         eval balloon_setsplit $f.e \[list \$${prop}($name)\] 15;
        };"

    if { $ay(ws) == "Aqua" } {
	$f.b configure -padx 6
    }

    pack $f.l -in $f -side left -fill x
    pack $f.e -in $f -side left -fill both -expand yes
    pack $f.b -in $f -side left -fill x
    pack $f -in $w -side top -fill x

 return;
}
# addMFile

#
#
#
proc addCommandB { w name text command help } {

    addCommand $w $name $text $command

    balloon_set $w.f${name}.b "${text}:\n${help}"

 return;
}
# addCommandB

#
#
#
proc addCommand { w name text command } {
    global ay ayprefs aymainshortcuts

    if { [winfo toplevel $w] == "." } {
	set escapecmd "resetFocus;break"
    } else {
	set escapecmd "after idle {$ay(bca) invoke}"
    }

    set bw 1

    set f [frame $w.f${name} -relief sunken -bd $bw]

    button $f.b -text $text -bd $bw -command $command -pady 0
    eval [subst "bindtags $f.b \{$f.b pge Button all\}"]
    bind $f.b <Key-Escape> $escapecmd
    bind $f.b [repctrl $aymainshortcuts(Help)] "uie_callhelp %W"

    if { ! $ay(iapplydisable) } {
	global aymainshortcuts
	bind $f.b <${aymainshortcuts(IApplyMod)}-ButtonRelease-1>\
	    "global ay; set ay(shiftcommand) 1;"
    }

    pack $f.b -in $f -side left -fill x -expand yes
    pack $f -in $w -side top -fill x

 return;
}
# addCommand

#
#
#
proc addText { w name text} {

    set f [frame $w.f${name}]

    label $f.l -text $text

    pack $f.l -in $f -side left -fill x -expand yes
    pack $f -in $w -side top

 return;
}
# addText

#
#
#
proc addInfoB { w prop name help } {

    addInfo $w $prop $name

    balloon_set $w.f${name}.l "${name}:\n${help}"

 return;
}
# addInfoB

#
#
#
proc updateInfoBalloon { f name1 name2 op } {
    global ${name1}
    set newtext [subst \$${name1}($name2)]
    balloon_set $f.l2 "$newtext"
 return;
}
# updateInfoBalloon

#
#
#
proc addInfo { w prop name } {
    set bw 1
    set f [frame $w.f${name} -relief sunken -bd $bw]

    label $f.l1 -width 14 -text ${name}:
    if {[string length ${name}] > 12} {
	balloon_set $f.l1 ${name}
    }
    label $f.l2 -textvariable ${prop}(${name})

    pack $f.l1 -in $f -side left -fill x -expand no
    pack $f.l2 -in $f -side left -fill x -expand yes
    pack $f -in $w -side top -fill x
    global $prop
    if { [ info exists "${prop}(${name}Ball)" ] } {
	global $prop
	set balltext [subst \$${prop}(${name}Ball)]
	balloon_set $f.l2 "$balltext"
	trace variable ${prop}(${name}Ball) w "updateInfoBalloon $f"
    }
 return;
}
# addInfo

#
#
#
proc updateProgress { w val n1 n2 op } {
    SetProgress $w $val
 return;
}
# updateProgress

#
#
#
proc addProgress { w prop name } {
    global $prop

    set bw 1
    set f [frame $w.f${name} -relief sunken -bd $bw]

    label $f.l1 -width 14 -text ${name}:
    if {[string length ${name}] > 12} {
	balloon_set $f.l1 ${name}
    }

    Progress $f.p

    set tracecommand "updateProgress $f.p \$${prop}(${name})"

    trace variable ${prop}(${name}) w $tracecommand

    bind $f.p <Destroy> "trace vdelete ${prop}(${name}) w {$tracecommand}"

    pack $f.l1 -in $f -side left -fill x -expand no
    pack $f.p -in $f -side left -fill x -expand yes
    pack $f -in $w -side top -fill x

 return;
}
# addProgress

#
#
#
proc addVSpace { w name h } {
    set f [frame $w.f${name}]
    set l [label $w.f${name}.${name} -height $h -image ay_Empty_img]
    pack $l -in $f
    pack $f -in $w -side top -fill x
 return;
}
# addVSpace

# addPropertyGUI:
#  create frame and global property management array
#
proc addPropertyGUI { name {sproc ""} {gproc ""} } {
    global ay $name

    set w $ay(pca).f$name
    set ay(bok) $ay(appb)

    if { ![info exists $name] } {
	frame $w
	set ${name}(arr) ${name}Data
	set ${name}(sproc) $sproc
	set ${name}(gproc) $gproc
	set ${name}(w) f$name

	set ::${name}GUI 1
    }

 return $w;
}
# addPropertyGUI
