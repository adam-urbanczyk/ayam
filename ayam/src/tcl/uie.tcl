# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2005 by Randolf Schultz
# (rschultz@informatik.uni-rostock.de) and others.
#
# All rights reserved.
#
# See the file License for details.

# User Interface Elements for Property GUIs and Preferences


#
#
#
proc updateParam { w prop name op } {
    global $prop

    set f $w.f${name}

    set oldval [$f.e get]
    set newval $oldval

    if {$op == "*2"} {
	set newval [expr $oldval * 2]
    } elseif { $op == "/2"} {
	set newval [expr $oldval / 2]
    } elseif { $op == "+0.1" } {

	set ta $oldval
	set tb 1.0
	set done 0
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
	set newval [expr $oldval + $tb]

    } elseif { $op == "-0.1" } {

	set ta $oldval
	set tb 1.0
	set done 0
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
	set newval [expr $oldval - $tb]
    }

    set ${prop}(${name}) $newval

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
global $prop ayprefs ay

set bw 1

set f [frame $w.f${name} -relief sunken -bd $bw]

label $f.l -width 14 -text ${name}:

if {[string length ${name}] > 12} {
    balloon_set $f.l ${name}
}

bind $f.l <Double-ButtonPress-1> "pclip_toggleomit $f.l $name"

button $f.b1 -pady 1 -bd $bw -text "/2" -command "updateParam $w $prop $name /2" -takefocus 0 -highlightthickness 0

set e [entry $f.e -width 8 -textvariable ${prop}(${name}) -bd $bw]
eval [subst "bindtags $f.e \{$f.e Entry all\}"]
bind $f.e <Key-Escape> {resetFocus}

button $f.b2 -pady 1 -bd $bw -text "*2"  -command "updateParam $w $prop $name *2" -takefocus 0 -highlightthickness 0
set mb ""
if { $def != {} } {
    set mb [menubutton $f.b3 -pady 2 -bd $bw -text "Def" -takefocus 0\
	    -highlightthickness 0 -relief raised -menu $f.b3.m]
    set m [menu $mb.m -tearoff 0]
    foreach val $def {
	$m add command -label $val\
		-command "global $prop; $e delete 0 end; $e insert end $val;"
    }
}

bind $f.b1 <Control-ButtonPress-1> "updateParam $w $prop $name -0.1;break"
bind $f.b2 <Control-ButtonPress-1> "updateParam $w $prop $name +0.1;break"

if { ! $ay(iapplydisable) } {
    global aymainshortcuts
    bind $f.b1 <${aymainshortcuts(IApplyMod)}-ButtonRelease-1> "after idle {\
	    \$ay(appb) invoke}"
    bind $f.b2 <${aymainshortcuts(IApplyMod)}-ButtonRelease-1> "after idle {\
	    \$ay(appb) invoke}"
    bind $f.e <Key-Return> "+after idle {\
	    \$ay(appb) invoke}"
    if { $mb != "" } {
	bind $mb <${aymainshortcuts(IApplyMod)}-ButtonRelease-1>\
	    "after idle {\$ay(appb) invoke}"
    }
}

global tcl_platform
if { $tcl_platform(platform) == "windows" } {
    pack $f.l -in $f -side left
    pack $f.b1 -in $f -side left -pady 0 -fill x -expand no
    pack $f.e -in $f -side left -pady 0 -fill both -expand yes
    pack $f.b2 -in $f -side left -pady 0 -fill x -expand no
    if { $mb != "" } { pack $mb -side left -pady 0 -fill both -expand no}
} else {
    $f.b1 configure -highlightthickness 1
    $f.b2 configure -highlightthickness 1
    if { $def != {} } { $mb configure -highlightthickness 1 }
    pack $f.l $f.b1 -in $f -side left -fill x -expand no
    pack $f.e -in $f -side left -fill x -expand yes
    pack $f.b2 -in $f -side left -fill x -expand no
    if { $mb != "" } { pack $mb -side left -fill x -expand no}
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
global $prop ayprefs

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
    eval [subst "bindtags $f1.e$i \{$f1.e$i Entry all\}"]
    bind $f1.e$i <Key-Escape> {resetFocus}
    pack $f1.e$i -in $f1 -side left -fill both -expand yes
}
set f2 [frame $w.f${name}2 -relief sunken -bd $bw]
for { set i 4 } { $i < 8 } { incr i } {
    entry $f2.e$i -width 6 -textvariable ${prop}(${name}_${i}) -bd $bw
    eval [subst "bindtags $f2.e$i \{$f2.e$i Entry all\}"]
    bind $f2.e$i <Key-Escape> {resetFocus}
    pack $f2.e$i -in $f2 -side left -fill both -expand yes
}
set f3 [frame $w.f${name}3 -relief sunken -bd $bw]
for { set i 8 } { $i < 12 } { incr i } {
    entry $f3.e$i -width 6 -textvariable ${prop}(${name}_${i}) -bd $bw
    eval [subst "bindtags $f3.e$i \{$f3.e$i Entry all\}"]
    bind $f3.e$i <Key-Escape> {resetFocus}
    pack $f3.e$i -in $f3 -side left -fill both -expand yes
}
set f4 [frame $w.f${name}4 -relief sunken -bd $bw]
for { set i 12 } { $i < 16 } { incr i } {
    entry $f4.e$i -width 6 -textvariable ${prop}(${name}_${i}) -bd $bw
    eval [subst "bindtags $f4.e$i \{$f4.e$i Entry all\}"]
    bind $f4.e$i <Key-Escape> {resetFocus}
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

    set newcolor [tk_chooseColor -initialcolor [$button cget -background]]
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
    global $prop ayprefs ay

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
    eval [subst "bindtags $f.er \{$f.er Entry all\}"]
    bind $e1 <Key-Escape> {resetFocus}
    set e2 [entry $f.eg -width 4 -textvariable ${prop}(${name}_G) -bd $bw]
    eval [subst "bindtags $f.eg \{$f.eg Entry all\}"]
    bind $e2 <Key-Escape> {resetFocus}
    set e3 [entry $f.eb -width 4 -textvariable ${prop}(${name}_B) -bd $bw]
    eval [subst "bindtags $f.eb \{$f.eb Entry all\}"]
    bind $e3 <Key-Escape> {resetFocus}

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

    global tcl_platform
    if { $tcl_platform(platform) == "windows" } {
	button $f.b1 -pady 1 -background $bcolor\
		-command "updateColor $w $prop $name $f.b1"\
		-bd $bw -width 3
    } else {
	set ws ""
	catch [set ws [tk windowingsystem]]
	if {$ws == "aqua" } {
	    label $f.l2 -background $bcolor -text "   "
	    button $f.b1 -pady 1 -padx 6 -text Set\
		-command "updateColor $w $prop $name $f.b1"\
		-bd $bw
	} else {
	button $f.b1 -pady 1 -background $bcolor\
		-command "updateColor $w $prop $name $f.b1"\
		-bd $bw
	}
    }
    # if

    eval [subst "bindtags $f.b1 \{$f.b1 Button all\}"]
    bind $f.b1 <Key-Escape> {resetFocus}

    bind $f.b1 <Visibility> "updateColorFromE $w $prop $name $f.b1"

    set mb ""
    if { $def != {} } {
	set mb [menubutton $f.b3 -pady 1 -bd $bw -text "Def" -takefocus 0\
		-highlightthickness 0 -relief raised -menu $f.b3.m]
	if { $tcl_platform(platform) == "windows" } {
	    $mb configure -pady 0
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
    }
    # if

    if { ! $ay(iapplydisable) } {
	global aymainshortcuts
	bind $e1 <Key-Return> "+after idle {\$ay(appb) invoke;\
                               updateColorFromE $w $prop $name $f.b1}"
	bind $e2 <Key-Return> "+after idle {\$ay(appb) invoke;\
                               updateColorFromE $w $prop $name $f.b1}"
	bind $e3 <Key-Return> "+after idle {\$ay(appb) invoke;
                               updateColorFromE $w $prop $name $f.b1}"
	if { $mb != "" } {
	    bind $mb <${aymainshortcuts(IApplyMod)}-ButtonRelease-1>\
		"after idle {\$ay(appb) invoke}"
	}

	bind $f.b1 <${aymainshortcuts(IApplyMod)}-ButtonPress-1>\
		"updateColor $w $prop $name $f.b1;\
		 after idle {\$ay(appb) invoke};break;"
    }

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
    global $prop ayprefs ay

    set bw 1
    set ws ""

    set f [frame $w.f${name} -relief sunken -bd $bw]

    label $f.l -width 14 -text ${name}:
    bind $f.l <Double-ButtonPress-1> "pclip_toggleomit $f.l $name"

    if {[string length ${name}] > 12} {
	balloon_set $f.l ${name}
    }

    global tcl_platform

    if { $tcl_platform(platform) == "windows" } {
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

	eval [subst "bindtags $ff.cb \{$ff.cb Checkbutton all\}"]
	bind $ff.cb <Key-Escape> {resetFocus}
    } else {
	catch [set ws [tk windowingsystem]]
	if { $ws == "aqua" } {
	    # also Aqua gets its "Extrawurst"
	    set ff [frame $f.fcb -highlightthickness 1]
	    set cb [checkbutton $ff.cb -variable ${prop}(${name}) -bd $bw]
	    bind $ff <1> { %W.cb invoke }
	    pack $f.l -in $f -side left
	    pack $f.fcb -in $f -side left -expand yes -fill both
	    pack $ff.cb -in $ff -side top -padx 30 -pady 0

	    eval [subst "bindtags $ff.cb \{$ff.cb Checkbutton all\}"]
	    bind $ff.cb <Key-Escape> {resetFocus}
	} else {
	    # generic (X11) implementation
	    set cb [checkbutton $f.cb -variable ${prop}(${name}) -bd $bw\
		    -pady 1 -padx 30]
	    pack $f.l -in $f -side left
	    pack $f.cb -in $f -side left -fill x -expand yes

	    eval [subst "bindtags $f.cb \{$f.cb Checkbutton all\}"]
	    bind $f.cb <Key-Escape> {resetFocus}
	}
    }

    if { ! $ay(iapplydisable) } {
	global aymainshortcuts
	bind $cb <${aymainshortcuts(IApplyMod)}-ButtonRelease-1> "after idle {\
	    \$ay(appb) invoke}"
	bind $cb <Key-Return> "$ay(appb) invoke;break"
	if { $tcl_platform(platform) == "windows" ||
             $ws == "aqua" } {
	    bind $ff <${aymainshortcuts(IApplyMod)}-ButtonRelease-1>\
		"after idle {\$ay(appb) invoke}"
	}
    }

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
    global $prop ayprefs ay tcl_platform

    set bw 1

    set f [frame $w.f${name} -relief sunken -bd $bw]

    label $f.l -width 14 -text ${name}:
    bind $f.l <Double-ButtonPress-1> "pclip_toggleomit $f.l $name"

    if {[string length ${name}] > 12} {
	balloon_set $f.l ${name}
    }

    menubutton $f.mb -text Eimer -menu $f.mb.m -relief raised -bd $bw\
	    -padx 0 -pady 1 -takefocus 1 -highlightthickness 1

    eval [subst "bindtags $f.mb \{$f.mb Menubutton all\}"]
    bind $f.mb <Key-Escape> {resetFocus}

    if { $tcl_platform(platform) == "windows" } {
	$f.mb configure -pady 1
    }
    set ws ""
    catch [set ws [tk windowingsystem]]
    if {$ws == "aqua" } {
	$f.mb configure -pady 2
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
	    "after idle {\$ay(appb) invoke}"
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
    global $prop ayprefs ay tcl_platform

    set bw 1

    set f [frame $w.f${name} -relief sunken -bd $bw]

    label $f.l -width 14 -text ${name}:
    bind $f.l <Double-ButtonPress-1> "pclip_toggleomit $f.l $name"

    if {[string length ${name}] > 12} {
	balloon_set $f.l ${name}
    }

    set e [entry $f.e -textvariable ${prop}(${name}) -width 15 -bd $bw]
    eval [subst "bindtags $f.e \{$f.e Entry all\}"]
    bind $f.e <Key-Escape> {resetFocus}

    set mb ""
    if { $def != {} } {
	set mb [menubutton $f.b3 -pady 1 -bd $bw -text "Def" -takefocus 0\
		-highlightthickness 0 -relief raised -menu $f.b3.m]
	if { $tcl_platform(platform) == "windows" } {
	    $mb configure -pady 0
	}
	set m [menu $mb.m -tearoff 0]
	foreach val $def {
	    $m add command -label $val\
	      -command "global $prop; $e delete 0 end; $e insert end \{$val\};"
	}

	if { ! $ay(iapplydisable) } {
	    global aymainshortcuts
	    bind $mb <${aymainshortcuts(IApplyMod)}-ButtonRelease-1>\
		"after idle {\$ay(appb) invoke}"
	}
    }

    if { ! $ay(iapplydisable) } {
	global aymainshortcuts
	bind $f.e <Key-Return> "+after idle {\$ay(appb) invoke}"
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
	    global $prop;
            winAutoFocusOff;
	    set filen \[$f.e get\];
	    set filen \[tk_getOpenFile -filetypes {$ftypes} -parent .\
	                -title \"Set File:\"];
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
	    global $prop;
            winAutoFocusOff;
	    set filen \[$f.e get\];
            set filen \[tk_getSaveFile -filetypes {$ftypes} -parent .\
		        -title \"Set File:\"];
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
    global $prop ayprefs tcl_platform AYWITHAQUA

    set bw 1

    set f [frame $w.f${name} -relief sunken -bd $bw]

    label $f.l -width 14 -text ${name}:
    bind $f.l <Double-ButtonPress-1> "pclip_toggleomit $f.l $name"

    if {[string length ${name}] > 12} {
	balloon_set $f.l ${name}
    }

    set e [entry $f.e -textvariable ${prop}(${name}) -width 15 -bd $bw]
    eval [subst "bindtags $f.e \{$f.e Entry all\}"]
    bind $f.e <Key-Escape> {resetFocus}

    button $f.b -text "Set" -width 4 -bd $bw -padx 0 -pady 0 -takefocus 0\
     -command "\
	global $prop;
        winAutoFocusOff;
	set filen \[$f.e get\];
	set filen \[tk_getOpenFile -parent . -title \"Set File:\"];
        winAutoFocusOn;
	if { \$filen != \"\" } {
	    $f.e delete 0 end;
	    $f.e insert 0 \$filen;
	    entryViewEnd $f.e;
	    set ${prop}($name) \$filen;
        }"

    if { $AYWITHAQUA } {
	$f.b configure -padx 6
    }

    set mb ""
    if { $def != {} } {
	set mb [menubutton $f.b3 -pady 1 -bd $bw -text "Def" -takefocus 0\
		-highlightthickness 0 -relief raised -menu $f.b3.m]
	if { $tcl_platform(platform) == "windows" } {
	    $mb configure -pady 0
	}
	if { $AYWITHAQUA } {
	    $f.b3 configure -pady 2
	}
	set m [menu $mb.m -tearoff 0]
	foreach val $def {
	    $m add command -label $val\
		    -command "global $prop; $e delete 0 end;
	                      $e insert end \{$val\};"
	}
	# foreach
    }
    # if

    pack $f.l -in $f -side left -fill x -expand no
    pack $f.e -in $f -side left -fill both -expand yes
    pack $f.b -in $f -side left -fill x -expand no

    if { $mb != "" } {
	if { $tcl_platform(platform) == "windows" } {
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
    global $prop ayprefs AYWITHAQUA

    set bw 1

    set f [frame $w.f${name} -relief sunken -bd $bw]

    label $f.l -width 14 -text ${name}:
    bind $f.l <Double-ButtonPress-1> "pclip_toggleomit $f.l $name"

    if {[string length ${name}] > 12} {
	balloon_set $f.l ${name}
    }

    entry $f.e -textvariable ${prop}(${name}) -width 15 -bd $bw
    eval [subst "bindtags $f.e \{$f.e Entry all\}"]
    bind $f.e <Key-Escape> {resetFocus}

    bind $f.e <1> "+balloon_setsplit $f.e \[$f.e get\] 15"
    eval balloon_setsplit $f.e  \$${prop}(${name}) 15

    button $f.b -text "Add" -width 4 -bd $bw -padx 0 -pady 0 -takefocus 0\
     -command "\
	global $prop;
        winAutoFocusOff;
	set filen \[$f.e get\];
	global ay;
	set sep \$ay(separator);
	set filen \[tk_getOpenFile -title \"Select File:\"\];
        winAutoFocusOn;
	if { \$filen != \"\" } {
	  if { \$${prop}($name) != \"\" } {
	      set ${prop}($name) \$${prop}($name)\$sep\[file dirname \$filen\];
	  } else {
	      set ${prop}($name) \[file dirname \$filen\];
	  };
	  entryViewEnd $f.e;
	  update;
	  eval balloon_setsplit $f.e \[list \$${prop}($name)\] 15;
	};"

    if { $AYWITHAQUA } {
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
    global $prop ayprefs AYWITHAQUA

    set bw 1

    set f [frame $w.f${name} -relief sunken -bd $bw]

    label $f.l -width 14 -text ${name}:
    bind $f.l <Double-ButtonPress-1> "pclip_toggleomit $f.l $name"

    if {[string length ${name}] > 12} {
	balloon_set $f.l ${name}
    }

    entry $f.e -textvariable ${prop}(${name}) -width 15 -bd $bw
    eval [subst "bindtags $f.e \{$f.e Entry all\}"]
    bind $f.e <Key-Escape> {resetFocus}

    bind $f.e <1> "+balloon_setsplit $f.e \[$f.e get\] 15"
    eval balloon_setsplit $f.e \$${prop}(${name}) 15

    button $f.b -text "Add" -width 4 -bd $bw -padx 0 -pady 0 -takefocus 0\
     -command "\
	global $prop;
        winAutoFocusOff;
	set filen \[$f.e get\];
	global ay;
	set sep \$ay(separator);
	set filen \[tk_getOpenFile -title \"Select File:\"\];
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


    if { $AYWITHAQUA } {
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
    global ay ayprefs

    set bw 1

    set f [frame $w.f${name} -relief sunken -bd $bw]

    button $f.b -text $text -bd $bw -command $command -pady 0
    eval [subst "bindtags $f.b \{$f.b Button all\}"]
    bind $f.b <Key-Escape> {resetFocus}

    if { ! $ay(iapplydisable) } {
	bind $f.b <Shift-1> "global ay; set ay(shiftcommand) 1;"
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

    set f [frame $w.${name}]

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

 return;
}
# addInfo
