# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2001 by Randolf Schultz
# (rschultz@informatik.uni-rostock.de) and others.
#
# All rights reserved.
#
# See the file License for details.

# riattr.tcl - helpers to add RiAttribute tags to objects
# This file has a pendant named riopt.tcl!

proc riattr_addp { } {
    global ay

    set w .addRiAttrw
    catch {destroy $w}
    toplevel $w
    wm title $w "Add RiAttribute"
    wm iconname $w "Ayam"
    wm transient $w .

    set f [frame $w.f1]
    label $f.l -text "Attribute:"
    pack $f.l -in $f

    # scrollbar
    scrollbar $f.sc -command "$f.li yview" -takefocus 0
    pack $f.sc -in $f -side right -fill y -expand no

    # listbox
    pack $f -in $w -side top -fill x -expand yes
    listbox $f.li -width 24 -height 10 -selectmode single\
	-yscrollcommand "$f.sc set" -exportselection 0
    pack $f.li -in $f -side left -fill both -expand yes

    # center window
    winCenter $w

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

    bind $f.li <ButtonRelease-1> {
	global riattr
	set lb .addRiAttrw.f1.li
 
	# get matching attribute list entry
	set attr [$lb get [$lb curselection]]

	set attrname [lindex $attr 0]
	set attrpara [lindex $attr 1]

	set list $riattr($attrname)
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

		    i { addParam .addRiAttrw.f2 riattrval Int $def}
		    f { addParam .addRiAttrw.f2 riattrval Float $def}
		    s { 
			addString .addRiAttrw.f2 riattrval String $def
		    }
		    p { addParam .addRiAttrw.f2 riattrval Point_X
		        addParam .addRiAttrw.f2 riattrval Point_Y
		        addParam .addRiAttrw.f2 riattrval Point_Z
			if { $def != "" } {
			    addMenu .addRiOptw.f2 rioptval DPoint $def
			    
			    set rioptval(def_Point) $def

			    trace variable rioptval(DPoint) w {
				after 100 {
				    global rioptval

		set def [lindex $rioptval(def_Point) $rioptval(DPoint) ]
				set rioptval(Point_X) [lindex $def 0]
				set rioptval(Point_Y) [lindex $def 1]
				set rioptval(Point_Z) [lindex $def 2]
				}
				return;
			    }
			    
			    
			}
		    }
		    c { addColor .addRiAttrw.f2 riattrval Color $def}
		    
		}
	    }

	}

    }

    # entry
    set f [frame $w.f2]
    pack $f -in $w -side top -fill x -expand no

    # buttons
    set f [frame $w.f3]
    button $f.bok -text "Ok" -width 5 -command { 
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
			
		    }
		}

	    }

	    if { $val != "" } {
		addTag RiAttribute "$attrname,$attrpara,$type,$val"
		plb_update
	    } else {
		ayError 2 riattr_addp "No value entered!"
	    }

	} else {
	    ayError 2 riattr_addp "No attribute selected!"
	}

	focus .
	grab release .addRiAttrw
	destroy .addRiAttrw

    }

    button $f.bca -text "Cancel" -width 5 -command "destroy $w"

    pack $f.bok $f.bca -in $f -side left -fill x -expand yes
    pack $f -in $w -side bottom -fill x

    grab $w
    focus $f.bok
    tkwait window $w

 return;
}
