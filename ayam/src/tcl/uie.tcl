# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2001 by Randolf Schultz
# (rschultz@informatik.uni-rostock.de) and others.
#
# All rights reserved.
#
# See the file License for details.

# Property User Interface Elements


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
    }

    set ${prop}(${name}) $newval

return;
}

#
#
#
proc addParam { w prop name {def {}}} {
global $prop ayprefs

set bw 1

set f [frame $w.f${name} -relief sunken -bd $bw]

label $f.l -width 12 -text ${name}:

if {[string length ${name}] > 11} {
    balloon_set $f.l ${name}
}

bind $f.l <Double-ButtonPress-1> "pclip_toggleomit $f.l $name"

button $f.b1 -pady 1 -bd $bw -text "/2" -command "updateParam $w $prop $name /2" -takefocus 0 -highlightthickness 0

set e [entry $f.e -width 8 -textvariable ${prop}(${name}) -bd $bw]

button $f.b2 -pady 1 -bd $bw -text "*2"  -command "updateParam $w $prop $name *2" -takefocus 0 -highlightthickness 0
set mb ""
if { $def != {} } {
    set mb [menubutton $f.b3 -pady 1 -bd $bw -text "Def" -takefocus 0\
	    -highlightthickness 0 -relief raised -menu $f.b3.m]
    set m [menu $mb.m -tearoff 0]
    foreach val $def {
	$m add command -label $val\
		-command "global $prop; $e delete 0 end; $e insert end $val;"
    }
}

global tcl_platform
if { $tcl_platform(platform) == "windows" } {
    pack $f.l -in $f -side left
    pack $f.b1 -in $f -side left -pady 0 -fill x -expand yes
    pack $f.e -in $f -side left -pady 0 -fill y
    pack $f.b2 -in $f -side left -pady 0 -fill x -expand yes
    if { $mb != "" } { pack $mb -side left -pady 0 -fill both -expand yes}
} else {
    pack $f.l $f.b1 $f.e $f.b2 -in $f -side left -fill both -expand yes
    if { $mb != "" } { pack $mb -side left -fill both -expand yes}
}
pack $f -in $w -side top -fill x
return;
}


#
#
#
proc addMatrix { w prop name } {
global $prop ayprefs

set bw 1

set f [frame $w.f${name} -relief sunken -bd $bw]

label $f.l -width 12 -text ${name}:

for { set i 0 } { $i < 16 } { incr i } {
    lappend omitl ${name}_$i
}

bind $f.l <Double-ButtonPress-1> "pclip_toggleomit $f.l { $omitl }"
pack $f.l -in $f
set f1 [frame $w.f${name}1 -relief sunken -bd $bw]
for { set i 0 } { $i < 4 } { incr i } {
    entry $f1.e$i -width 6 -textvariable ${prop}(${name}_${i}) -bd $bw
    pack $f1.e$i -in $f1 -side left -fill both
}
set f2 [frame $w.f${name}2 -relief sunken -bd $bw]
for { set i 4 } { $i < 8 } { incr i } {
    entry $f2.e$i -width 6 -textvariable ${prop}(${name}_${i}) -bd $bw
    pack $f2.e$i -in $f2 -side left -fill both
}
set f3 [frame $w.f${name}3 -relief sunken -bd $bw]
for { set i 8 } { $i < 12 } { incr i } {
    entry $f3.e$i -width 6 -textvariable ${prop}(${name}_${i}) -bd $bw
    pack $f3.e$i -in $f3 -side left -fill both
}
set f4 [frame $w.f${name}4 -relief sunken -bd $bw]
for { set i 12 } { $i < 16 } { incr i } {
    entry $f4.e$i -width 6 -textvariable ${prop}(${name}_${i}) -bd $bw
    pack $f4.e$i -in $f4 -side left -fill both
}

pack $f $f1 $f2 $f3 $f4 -in $w -side top -fill x
return;
}


#
#
#
proc updateColor { w prop name button } {
    global $prop 

    set rname ${prop}(${name}_R)
    set gname ${prop}(${name}_G)
    set bname ${prop}(${name}_B)

    set newcolor [tk_chooseColor -initialcolor [$button cget -background]]   
    if { $newcolor != "" } {
	$button configure -background $newcolor

	scan $newcolor "#%2x%2x%2x" r g b
	set $rname $r
	set $gname $g
	set $bname $b
    }

return;
}

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

    if { $green < 0 } { set $gname 0; set green 0 }
    if { $green > 255 } { set $gname 255; set green 255 }

    if { $blue < 0 } { set $bname 0; set blue 0 }
    if { $blue > 255 } { set $bname 255; set blue 255 }
    if { $red == -1 } {
	set newcolor black
    } else {
	set newcolor [format "#%02x%02x%02x" $red $green $blue]
    }
    $button configure -background $newcolor

return;
}

#
#
#
proc addColor { w prop name  {def {}}} {
    global $prop ayprefs

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
    set e2 [entry $f.eg -width 4 -textvariable ${prop}(${name}_G) -bd $bw]
    set e3 [entry $f.eb -width 4 -textvariable ${prop}(${name}_B) -bd $bw]

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
	button $f.b1 -pady 1 -background $bcolor\
		-command "updateColor $w $prop $name $f.b1"\
		-bd $bw
    }

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
    }



    pack $f.er $f.eg $f.eb $f.b1 -in $f -fill both -expand yes\
	    -side left -padx 2

    if { $mb != "" } { pack $mb -side left -fill both -expand yes}

    pack $f -in $w.fl${name} -side bottom -fill x

 return;
}


#
#
#
proc addCheck { w prop name } {
    global $prop ayprefs

    set bw 1

    set f [frame $w.f${name} -relief sunken -bd $bw]

    label $f.l -width 12 -text ${name}:
    bind $f.l <Double-ButtonPress-1> "pclip_toggleomit $f.l $name"

    if {[string length ${name}] > 11} {
	balloon_set $f.l ${name}
    }

    global tcl_platform

    if { $tcl_platform(platform) == "windows" } {
	# damn windows
	checkbutton $f.cb -image emptyimg -variable ${prop}(${name})\
		-bd $bw -indicatoron 0 -selectcolor #b03060

	pack $f.l -in $f -side left
	pack $f.cb -in $f -side left -padx 50 -pady 2
    } else {
	checkbutton $f.cb -variable ${prop}(${name}) -bd $bw -pady 1
	pack $f.l -in $f -side left
	pack $f.cb -in $f -side left -fill x -expand yes
    }


    pack $f -in $w -side top -fill x

return;
}


#
#
#
proc updateMenu { m name1 name2 op } {
    global ${name1}
    $m invoke [subst \$${name1}($name2)]
}
# updateMenu


#
#
#
proc addMenu { w prop name elist } {
    global $prop ayprefs tcl_platform

    set bw 1

    set f [frame $w.f${name} -relief sunken -bd $bw]

    label $f.l -width 12 -text ${name}:
    bind $f.l <Double-ButtonPress-1> "pclip_toggleomit $f.l $name"

    if {[string length ${name}] > 11} {
	balloon_set $f.l ${name}
    }

    menubutton $f.mb -text Eimer -menu $f.mb.m -relief raised -bd $bw\
	    -padx 0 -pady 1

    if { $tcl_platform(platform) == "windows" } {
	$f.mb configure -pady 1
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

    pack $f.l -in $f -side left -fill x
    pack $f.mb -in $f -side left -fill x -expand yes -pady 0
    pack $f -in $w -side top -fill x

return;
}

#
#
#
proc addString { w prop name  {def {}}} {
    global $prop ayprefs tcl_platform
 
    set bw 1

    set f [frame $w.f${name} -relief sunken -bd $bw]
    
    label $f.l -width 12 -text ${name}:
    bind $f.l <Double-ButtonPress-1> "pclip_toggleomit $f.l $name"

    if {[string length ${name}] > 11} {
	balloon_set $f.l ${name}
    }

    set e [entry $f.e -textvariable ${prop}(${name}) -width 15 -bd $bw]
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
	     -command "global $prop; $e delete 0 end; $e insert end $val;"
	}
    }


    pack $f.l -in $f -side left -fill x
    pack $f.e -in $f -side left -fill both -expand yes
    if { $mb != "" } { pack $mb -side left -fill both -expand yes}

    pack $f -in $w -side top -fill x

return;
}

#
#
#
proc addFile { w prop name } {
    global $prop ayprefs

    set bw 1

    set f [frame $w.f${name} -relief sunken -bd $bw]

    label $f.l -width 10 -text ${name}:
    bind $f.l <Double-ButtonPress-1> "pclip_toggleomit $f.l $name"

    if {[string length ${name}] > 10} {
	balloon_set $f.l ${name}
    }

    entry $f.e -textvariable ${prop}(${name}) -width 15 -bd $bw
    button $f.b -text "Set" -bd $bw -padx 0 -pady 0 -command "\
	global $prop;
	set filen \[$f.e get\];
	set filen \[tk_getOpenFile -parent . -title \"Set File:\"];
	if { \$filen != \"\" } {
	    $f.e delete 0 end;
	    $f.e insert 0 \$filen;
	    set ${prop}($name) \$filen;
        }
	" 
	pack $f.l -in $f -side left -fill x
	pack $f.b -in $f -side right -fill x
	pack $f.e -in $f -side left -fill both -expand yes
	pack $f -in $w -side top -fill x

return;
}

#
#
#
proc addMDir { w prop name } {
    global $prop ayprefs

    set bw 1

    set f [frame $w.f${name} -relief sunken -bd $bw]

    label $f.l -width 10 -text ${name}:
    bind $f.l <Double-ButtonPress-1> "pclip_toggleomit $f.l $name"

    if {[string length ${name}] > 10} {
	balloon_set $f.l ${name}
    }

    entry $f.e -textvariable ${prop}(${name}) -width 15 -bd $bw
    bind $f.e <1> "+balloon_setsplit $f.e \[$f.e get\] 15"
    eval balloon_setsplit $f.e  \$${prop}(${name}) 15

    button $f.b -text "Add" -bd $bw -padx 0 -pady 0 -command "\
	global $prop;
	set filen \[$f.e get\];
	global ay;
	set sep \$ay(separator);
	set filen \[tk_getOpenFile -title \"Select File:\"\];
	if { \$filen != \"\" } {
	  if { \$${prop}($name) != \"\" } {
	      set ${prop}($name) \$${prop}($name)\$sep\[file dirname \$filen\];
	    } else {
		set ${prop}($name) \[file dirname \$filen\];
	    };
	  update;
          eval balloon_setsplit $f.e \$${prop}($name) 15;
	};
	" 
	pack $f.l -in $f -side left -fill x
	pack $f.b -in $f -side right -fill x
	pack $f.e -in $f -side left -fill both -expand yes
	pack $f -in $w -side top -fill x

return;
}


#
#
#
proc addMFile { w prop name } {
    global $prop ayprefs

    set bw 1

    set f [frame $w.f${name} -relief sunken -bd $bw]

    label $f.l -width 10 -text ${name}:
    bind $f.l <Double-ButtonPress-1> "pclip_toggleomit $f.l $name"

    if {[string length ${name}] > 10} {
	balloon_set $f.l ${name}
    }

    entry $f.e -textvariable ${prop}(${name}) -width 15 -bd $bw
    bind $f.e <1> "+balloon_setsplit $f.e \[$f.e get\] 15"
    eval balloon_setsplit $f.e \$${prop}(${name}) 15

    button $f.b -text "Add" -bd $bw -padx 0 -pady 0 -command "\
	global $prop;\
	set filen \[$f.e get\];\
	global ay;\
	set sep \$ay(separator);\
	set filen \[tk_getOpenFile\
	-title \"Select File:\"\];\
	if { \$filen != \"\" } {\
	 if { \$${prop}($name) != \"\" } {\
          set ${prop}($name) \$${prop}($name)\$sep\$filen;\
	 } else {\
	  set ${prop}($name) \$filen;\
         };\
	 update;
         eval balloon_setsplit $f.e \$${prop}($name) 15;
        };\
	" 
	pack $f.l -in $f -side left -fill x
	pack $f.b -in $f -side right -fill x
	pack $f.e -in $f -side left -fill both -expand yes
	pack $f -in $w -side top -fill x

return;
}

#
#
#
proc addCommand { w name text command } {
    global ayprefs
    set bw 1

    set f [frame $w.f${name} -relief sunken -bd $bw]

    button $f.b -text $text -bd $bw -command $command -pady 0

    pack $f.b -in $f -side left -fill x -expand yes
    pack $f -in $w -side top -fill x

return;
}

#
#
#
proc addText { w name text} {

set f [frame $w.${name}]

label $f.l -text $text

pack $f.l -in $f
pack $f -in $w -side top
return;
}

#
#
#
proc addInfo { w prop name } {
set bw 1
set f [frame $w.f${name} -relief sunken -bd $bw]

label $f.l1 -width 10 -text ${name}:
label $f.l2 -textvariable ${prop}(${name})

pack $f.l1 $f.l2 -in $f -side left
pack $f -in $w -side top -fill x
return;
}
