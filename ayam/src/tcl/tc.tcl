# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2002 by Randolf Schultz
# (rschultz@informatik.uni-rostock.de) and others.
#
# All rights reserved.
#
# See the file License for details.

# tc.tcl - texture coordinate tag editor

# tc_updateCanvas:
#  redraw the canvas
#
proc tc_updateCanvas { ca } {
    global ay tc
    
    set graycolor #A0A0A0

    $ca delete all

    $ca create polygon 0.0 0.0 0.0 1.0 1.0 1.0 1.0 0.0\
	    -fill "" -outline $graycolor

    $ca create line -1.0 0.0 1.0 0.0 -fill $graycolor -arrow last
    $ca create line 0.0 -1.0 0.0 1.0 -fill $graycolor -arrow last

#    $ca create line 0.0 0.0 $tc(v1) $tc(v2) -fill $graycolor
#    $ca create line 1.0 0.0 $tc(v3) $tc(v4) -fill $graycolor
#    $ca create line 0.0 1.0 $tc(v5) $tc(v6) -fill $graycolor
#    $ca create line 1.0 1.0 $tc(v7) $tc(v8) -fill $graycolor

#    $ca create text [expr $tc(v1)-0.01] [expr $tc(v2)-0.01] -text "a"
#    $ca create text [expr $tc(v3)-0.01] [expr $tc(v4)-0.01] -text "b"
#    $ca create text [expr $tc(v5)-0.01] [expr $tc(v6)-0.01] -text "c"
#    $ca create text [expr $tc(v7)-0.01] [expr $tc(v8)-0.01] -text "d"

    $ca create polygon $tc(v1) $tc(v2) $tc(v3) $tc(v4)\
	    $tc(v7) $tc(v8) $tc(v5) $tc(v6) -fill "" -outline black

    $ca create line $tc(v1) $tc(v2) $tc(v3) $tc(v4) -arrow last
    $ca create line $tc(v1) $tc(v2) $tc(v5) $tc(v6) -arrow last
    set mins Inf
    set maxs -Inf
    foreach i [list 1 3 5 7] {
	if { $tc(v$i) < $mins } { set mins $tc(v$i) }
	if { $tc(v$i) > $maxs } { set maxs $tc(v$i) }
    }
    set mint Inf
    set maxt -Inf
    foreach i [list 2 4 6 8] {
	if { $tc(v$i) < $mint } { set mint $tc(v$i) }
	if { $tc(v$i) > $maxt } { set maxt $tc(v$i) }
    }

    set width [expr $maxs-$mins]
    set height [expr $maxt-$mint]
    set scalex [expr ([winfo width $ca]-20)]
    set scaley [expr ([winfo height $ca]-20)]
    if { $width > 1 } {
	set scalex [expr ([winfo width $ca]-20)/double($width)]
    }
    if { $height > 1 } {
	set scaley [expr ([winfo height $ca]-20)/double($height)]
    }

    $ca scale all 0 0 $scalex $scaley

    $ca move all 10 10

    set movx 0
    set movy 0
    if { [expr abs($mins)] > 0.00001 } {
	set movx [expr -($scalex * $mins)]
    }
    if { [expr abs($mint) > 0.00001] } {
	set movy [expr -($scaley * $mint)]
    }

    $ca move all $movx $movy

 return;
}
# tc_updateCanvas


# tc_updateMenu:
#  update menu entries according to currently present tags
#
proc tc_updateMenu { } {
    global ay tc tclist tcindex tcumsema

    # protect against double invocation using a semaphore
    if { $tcumsema == 1 } { return } else { set tcumsema 1 }

    # get tc tags from currently selected object
    set names ""
    set values ""
    getTags names values
    set tclist {}
    set tcindex {dummy}
    if { $names != "" } {
	set namec [llength $names]
	set i 0
	while { $i < $namec } {
	    set name [lindex $names $i]
	    if { $name == "TC" } {
		lappend tclist [lindex $values $i]
		lappend tcindex $i
	    }
	    incr i
	}
    }

    # update load menu
    set m .tcEditw.fb1.mb1.m
    $m delete 0 end
    update
    $m add command -label "Defaults" -command {
	global tc
	puts "tc_edit: Loading default values..."
	set values [list 0.0 0.0 1.0 0.0 0.0 1.0 1.0 1.0]
	foreach i [list 1 2 3 4 5 6 7 8] {
	    set tc(v$i) [lindex $values [expr $i - 1]]
	}
	tc_updateCanvas .tcEditw.fc.ca
	bind .tcEditw <Enter> {tc_updateMenu}
    }

    $m add separator
    $m add command -label "From BPatch" -command {
	global tc
	puts "tc_edit: Loading values from BPatch..."

	getPnt -trafo 0 tc(v1) tc(v2) dummy
	getPnt -trafo 1 tc(v3) tc(v4) dummy
	getPnt -trafo 3 tc(v5) tc(v6) dummy
	getPnt -trafo 2 tc(v7) tc(v8) dummy

	tc_updateCanvas .tcEditw.fc.ca
	bind .tcEditw <Enter> {tc_updateMenu}
    }

    $m add separator

    set i 1
    foreach tag $tclist {
	$m add command -label "From Tag#$i" -command "\
	    global tc tclist;\
	    scan $tag \"%g,%g,%g,%g,%g,%g,%g,%g\" tc(v1) tc(v2) tc(v3)\
	    tc(v4) tc(v5) tc(v6) tc(v7) tc(v8);\
	    tc_updateCanvas .tcEditw.fc.ca;\
	    bind .tcEditw <Enter> {tc_updateMenu}"
	incr i
    }

    # update save menu
    set m .tcEditw.fb1.mb2.m
    $m delete 0 end
    $m add command -label "To BPatch" -command {
	global tc

	setPnt 0 $tc(v1) $tc(v2) 0.0
	setPnt 1 $tc(v3) $tc(v4) 0.0
	setPnt 3 $tc(v5) $tc(v6) 0.0
	setPnt 2 $tc(v7) $tc(v8) 0.0

	rV
	tc_updateCanvas .tcEditw.fc.ca
	bind .tcEditw <Enter> {tc_updateMenu}
    }
    $m add separator

    set i 1

    if { [llength $tclist] > 0 } {
	foreach tag $tclist {
	    $m add command -label "To Tag#$i" -command "\
 	      global tc tcindex;\
	      setTags -index [lindex $tcindex $i] TC\
 \"\$tc(v1),\$tc(v2),\$tc(v3),\$tc(v4),\$tc(v5),\$tc(v6),\$tc(v7),\$tc(v8)\";\
              tc_updateMenu;\
	      plb_update;\
	      bind .tcEditw <Enter> {tc_updateMenu}"
	    incr i
	}
    }

    $m add command -label "To Tag#$i" -command {
	global tc
	addTag TC\
	      "$tc(v1),$tc(v2),$tc(v3),$tc(v4),$tc(v5),$tc(v6),$tc(v7),$tc(v8)"
	tc_updateMenu
	plb_update
	bind .tcEditw <Enter> {tc_updateMenu}
    }

    set tcumsema 0

 return;
}
# tc_updateMenu


# tc_edit:
#  open the texture coordinate tag editor
#
proc tc_edit { } {
    global ay tc tcumsema

    # unset semaphore
    set tcumsema 0

    set w .tcEditw
    catch {destroy $w}
    toplevel $w
    wm title $w "Edit Texture Coordinates"
    wm iconname $w "Ayam"
    wm transient $w .

    # default values
    set values [list 0.0 0.0 1.0 0.0 0.0 1.0 1.0 1.0]
    foreach i [list 1 2 3 4 5 6 7 8] {
	set tc(v$i) [lindex $values [expr $i - 1]]
    }
    set tc(amount) 0.1

    # action buttons
    set f [frame $w.fab1]
    button $f.b1 -text "RotateL" -width 7 -pady $ay(pady) -command {
	global tc
	array set ttc [array get tc]

	set tc(v1) $ttc(v3)
	set tc(v2) $ttc(v4)
	set tc(v3) $ttc(v7)
	set tc(v4) $ttc(v8)
	set tc(v5) $ttc(v1)
	set tc(v6) $ttc(v2)
	set tc(v7) $ttc(v5)
	set tc(v8) $ttc(v6)

	tc_updateCanvas .tcEditw.fc.ca
    }
    button $f.b2 -text "RotateR" -width 7 -pady $ay(pady) -command {

	global tc
	array set ttc [array get tc]

	set tc(v1) $ttc(v5)
	set tc(v2) $ttc(v6)
	set tc(v3) $ttc(v1)
	set tc(v4) $ttc(v2)
	set tc(v5) $ttc(v7)
	set tc(v6) $ttc(v8)
	set tc(v7) $ttc(v3)
	set tc(v8) $ttc(v4)

	tc_updateCanvas .tcEditw.fc.ca

    }

    button $f.b3 -text "FlipS" -width 5 -pady $ay(pady) -command {
	global tc
	array set ttc [array get tc]

	set tc(v1) $ttc(v3)
	set tc(v2) $ttc(v4)
	set tc(v3) $ttc(v1)
	set tc(v4) $ttc(v2)
	set tc(v5) $ttc(v7)
	set tc(v6) $ttc(v8)
	set tc(v7) $ttc(v5)
	set tc(v8) $ttc(v6)

	tc_updateCanvas .tcEditw.fc.ca
    }

    button $f.b4 -text "FlipT" -width 5 -pady $ay(pady) -command {
	global tc
	array set ttc [array get tc]

	set tc(v1) $ttc(v5)
	set tc(v2) $ttc(v6)
	set tc(v3) $ttc(v7)
	set tc(v4) $ttc(v8)
	set tc(v5) $ttc(v1)
	set tc(v6) $ttc(v2)
	set tc(v7) $ttc(v3)
	set tc(v8) $ttc(v4)

	tc_updateCanvas .tcEditw.fc.ca
    }

    pack $f.b1 $f.b2 $f.b3 $f.b4 -in $f -side left -fill x -expand yes
    pack $f -side top -fill x -expand no

    set f [frame $w.fab2]
    entry $f.e -text "Amount:" -width 8 -textvariable tc(amount)

    button $f.b1 -text "ScaleS" -width 6 -pady $ay(pady) -command {
	global tc
	foreach i [list 1 3 5 7] {
	    set tc(v$i) [expr $tc(v$i) * $tc(amount)]
	}
	tc_updateCanvas .tcEditw.fc.ca
    }

    button $f.b2 -text "ScaleT" -width 6 -pady $ay(pady) -command {
	global tc
	foreach i [list 2 4 6 8] {
	    set tc(v$i) [expr $tc(v$i) * $tc(amount)]
	}
	tc_updateCanvas .tcEditw.fc.ca
    }

    button $f.b3 -text "MoveS" -width 5 -pady $ay(pady) -command {
	global tc
	foreach i [list 1 3 5 7] {
	    set tc(v$i) [expr $tc(v$i) + $tc(amount)]
	}
	tc_updateCanvas .tcEditw.fc.ca
    }

    button $f.b4 -text "MoveT" -width 5 -pady $ay(pady) -command {
	global tc
	foreach i [list 2 4 6 8] {
	    set tc(v$i) [expr $tc(v$i) + $tc(amount)]
	}
	tc_updateCanvas .tcEditw.fc.ca
    }

    pack $f.e  -pady 0 -fill both -in $f -side left -expand yes
    pack $f.b1 $f.b2 $f.b3 $f.b4\
	    -in $f -side left -fill x -expand yes
    pack $f -side top -fill x -expand no

    # a canvas
    set f [frame $w.fc]

    set ca [canvas $f.ca -width 200 -height 200 -bd 1 -relief sunken]

    pack $f.ca -in $f -side top -fill both -expand yes
    pack $f -side top -fill both -expand yes

    # 8 entries for numeric values
    set f [frame $w.fe1]
    label $f.ls -text "S:" -width 2
    pack $f.ls -side left
    foreach i [list 1 3 5 7] {
	entry $f.e_$i -textvariable tc(v$i) -width 8
	bind $f.e_$i <KeyPress-Tab> "tc_updateCanvas $ca;tc_updateMenu"
	pack $f.e_$i -side left -expand yes
    }
    pack $f -side top -fill x -expand no
    set f [frame $w.fe2]
    label $f.lt -text "T:" -width 2
    pack $f.lt -side left
    foreach i [list 2 4 6 8] {
	entry $f.e_$i -textvariable tc(v$i) -width 8
	bind $f.e_$i <KeyPress-Tab> "tc_updateCanvas $ca;tc_updateMenu"
	pack $f.e_$i -side left -expand yes
    }
    pack $f -side top -fill x -expand no


    # 3 buttons
    set f [frame $w.fb1]

    # load menubutton
    menubutton $f.mb1 -text "Load" -pady $ay(pady) -relief raised\
	    -menu $f.mb1.m
    set m [menu $f.mb1.m -tearoff 0 -postcommand {
	bind .tcEditw <Enter> "" } ]

    # save menubutton
    menubutton $f.mb2 -text "Save" -pady $ay(pady) -relief raised\
	    -menu $f.mb2.m
    set m [menu $f.mb2.m -tearoff 0 -postcommand {
	bind .tcEditw <Enter> "" } ]

    tc_updateMenu

    pack $f.mb1 $f.mb2 -in $f -side left -fill x -expand yes
    pack $f -in $w -side top -fill x

    # dismiss button
    set f [frame $w.fb2]
    button $f.bd -text "Dismiss" -pady $ay(pady) -command {
	global tclist tcindex tc tcumsema
	# clean up
	unset tc tclist tcindex tcumsema
	focus .
	destroy .tcEditw
    }
    pack $f.bd -in $f -side left -fill x -expand yes
    pack $f -in $w -side bottom -fill x


    # in case tags are changing while tc_edit is open
    bind $w <Enter> {tc_updateMenu}

    # create initial drawing
    tc_updateCanvas $ca

    # act on resize operations of the user
    bind $w <Configure> "tc_updateCanvas $ca"

    # center window
    winCenter $w

    # adjust focus
    focus $w.fb1.mb1

    # block application? (No! We want the user to be able to edit
    # BPatches while the requester is open!)
    #tkwait window $w

 return;
}
# tc_edit

