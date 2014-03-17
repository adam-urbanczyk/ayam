# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2001 by Randolf Schultz
# (randolf.schultz@gmail.com) and others.
#
# All rights reserved.
#
# See the file License for details.

# riattr.tcl - helpers to add RiAttribute tags to objects
# This file has a pendant named riopt.tcl!

proc riattr_addp { } {
    global ay ayprefs

    winAutoFocusOff

    set ay(addRiAttrFocus) [focus]

    set w .addRiAttrw
    set t "Add RiAttribute"
    winDialog $w $t

    if { $ayprefs(FixDialogTitles) == 1 } {
	pack [frame $w.fl] -in $w -side top
	pack [label $w.fl.l -text $t] -in $w.fl -side left -fill x -expand yes
    }

    set f [frame $w.f1]
    label $f.l -text "Attribute:"
    pack $f.l -in $f

    # scrollbar
    scrollbar $f.sc -command "$f.li yview" -takefocus 0
    pack $f.sc -in $f -side right -fill y -expand no

    # listbox
    pack $f -in $w -side top -fill both -expand yes
    listbox $f.li -width 24 -height 10 -selectmode browse -activestyle none\
	-yscrollcommand "$f.sc set" -exportselection 0
    pack $f.li -in $f -side left -fill both -expand yes

    # create/set variables
    global riattrval
    array set riattrval {
	Int 0
	Float 0.0
	String ""
	Point_X 0
	Point_Y 0
	Point_Z 0
	Color_R 0
	Color_G 0
	Color_B 0
	DPoint 0
	def_Point {}
    }

    # fill listbox
    global riattr
    set names [array names riattr]
    foreach name $names {
	set num [llength $riattr($name)]
	set i 0
	while {$i < $num} {
	    set attr [lindex $riattr($name) $i]
	    set attrname [lindex $attr 0]
	    $f.li insert end "$name $attrname"

	    incr i
	}
    }

    bind $f.li <<ListboxSelect>> {
	global ay riattr
	set lb .addRiAttrw.f1.li

	set ay(bca) .addRiAttrw.f3.bca
	set ay(bok) .addRiAttrw.f3.bok

	# get matching attribute list entry
	set attr [$lb get [$lb curselection]]

	set attrname [lindex $attr 0]
	set attrpara [lindex $attr 1]

	set list $riattr($attrname)

	# disable instant apply bindings
	set oldiapplydisable $ay(iapplydisable)
	set ay(iapplydisable) 1

	foreach param $list {
	    if { $attrpara == [lindex $param 0] } {
		# destroy old UI
		set wins [winfo children .addRiAttrw.f2]
		foreach win $wins {
		    catch {destroy $win}
		}

		# create new UI
		set f .addRiAttrw.f2
		label $f.l -text "Value:"
		pack $f.l -in $f

		set type [lindex $param 1]
		set def {}
		set def [lindex $param 2]

		switch $type {
		    i {
			if { $def != {} } {
			    set riattrval(Int) [lindex $def 0] } else {
				set riattrval(Int) 0
			    }
			addParam .addRiAttrw.f2 riattrval Int $def
		    }
		    f {
			if { $def != {} } {
			    set riattrval(Float) [lindex $def 0] } else {
				set riattrval(Float) 0.0
			    }
			addParam .addRiAttrw.f2 riattrval Float $def
		    }
		    s {
			if { $def != {} } {
			    set riattrval(String) [lindex $def 0] } else {
				set riattrval(String) ""
			}
			addString .addRiAttrw.f2 riattrval String $def
		      }
		    p { addParam .addRiAttrw.f2 riattrval Point_X
		        addParam .addRiAttrw.f2 riattrval Point_Y
		        addParam .addRiAttrw.f2 riattrval Point_Z
			if { $def != "" } {
			    addMenu .addRiAttrw.f2 riattrval DPoint $def

			    set riattrval(def_Point) $def

			    trace variable riattrval(DPoint) w {
				after 100 {
				    global riattrval

		set def [lindex $riattrval(def_Point) $riattrval(DPoint) ]
				set riattrval(Point_X) [lindex $def 0]
				set riattrval(Point_Y) [lindex $def 1]
				set riattrval(Point_Z) [lindex $def 2]
				}
				return;
			    }
			}
		    }
		    c {
			if { $def != {} } {
			    set riattrval(Color_R) [lindex [lindex $def 0] 0]
			    set riattrval(Color_G) [lindex [lindex $def 0] 1]
			    set riattrval(Color_B) [lindex [lindex $def 0] 2]
			} else {
			    set riattrval(Color_R) 0
			    set riattrval(Color_G) 0
			    set riattrval(Color_B) 0
			}
			addColor .addRiAttrw.f2 riattrval Color $def
		    }
		    j {
			if { $def != {} } {
			    set riattrval(IntPair_0) [lindex [lindex $def 0] 0]
			    set riattrval(IntPair_1) [lindex [lindex $def 0] 1]
			} else {
			    set riattrval(IntPair_0) 0
			    set riattrval(IntPair_1) 0
			}
			addParamPair .addRiAttrw.f2 riattrval IntPair $def
		    }
		    g {
			if { $def != {} } {
			 set riattrval(FloatPair_0) [lindex [lindex $def 0] 0]
			 set riattrval(FloatPair_1) [lindex [lindex $def 0] 1]
			} else {
			    set riattrval(FloatPair_0) 0.0
			    set riattrval(FloatPair_1) 0.0
			}
			addParamPair .addRiAttrw.f2 riattrval FloatPair $def
		    }
		}
		# switch
	    }
	    # if
	}
	# foreach

	set ay(iapplydisable) $oldiapplydisable
    }
    # bind

    # entry
    set f [frame $w.f2]
    pack $f -in $w -side top -fill x -expand no

    # buttons
    set f [frame $w.f3]
    button $f.bok -text "Ok" -pady $ay(pady) -width 5 -command {
	global riattr riattrval

	set lb .addRiAttrw.f1.li

	set sel [$lb curselection]
	if { $sel != "" } {
	    set attr [$lb get $sel]

	    set attrname [lindex $attr 0]
	    set attrpara [lindex $attr 1]

	    set val ""
	    set list $riattr($attrname)
	    foreach param $list {
		if { $attrpara == [lindex $param 0] } {
		    catch {destroy [winfo children .addRiAttrw.f2]}
		    set type [lindex $param 1]
		    switch $type {
			i { set val $riattrval(Int) }
			f { set val $riattrval(Float) }
			s { set val $riattrval(String) }
			p { set val \
	"$riattrval(Point_X),$riattrval(Point_Y),$riattrval(Point_Z)" }
			c { set val \
	"$riattrval(Color_R),$riattrval(Color_G),$riattrval(Color_B)" }
                        j { set val \
	"$riattrval(IntPair_0),$riattrval(IntPair_1)" }
                        g { set val \
	"$riattrval(FloatPair_0),$riattrval(FloatPair_1)" }
		    }
		    # switch
		}
		# if
	    }
	    # foreach param
	    if { $val != "" } {
		set selected ""
		getSel selected
		if { $selected == "" } { ayError 20 "AddRiAttr" ""; return; }

		undo save AddRiAttr
		addTag RiAttribute "$attrname,$attrpara,$type,$val"
		plb_update
	    } else {
		ayError 2 riattr_addp "No value entered!"
	    }
	} else {
	    ayError 2 riattr_addp "No attribute selected!"
	}

	grab release .addRiAttrw
	restoreFocus $ay(addRiAttrFocus)
	destroy .addRiAttrw
    }
    # button

    button $f.bca -text "Cancel" -pady $ay(pady) -width 5 -command "\
	grab release $w;\
        restoreFocus $ay(addRiAttrFocus);\
        destroy $w"

    pack $f.bok $f.bca -in $f -side left -fill x -expand yes
    pack $f -in $w -side bottom -fill x

    # Esc-key && close via window decoration == Cancel button
    bind $w <Escape> "$f.bca invoke"
    wm protocol $w WM_DELETE_WINDOW "$f.bca invoke"
    bind $w <Key-Return> "$f.bok invoke"
    catch { bind $w <Key-KP_Enter> "$f.bok invoke" }

    winRestoreOrCenter $w $t

    grab $w
    focus $w.f1.li
    tkwait window $w
    winAutoFocusOn

 return;
}
# riattr_addp
