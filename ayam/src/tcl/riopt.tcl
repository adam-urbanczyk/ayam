# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2001 by Randolf Schultz
# (rschultz@informatik.uni-rostock.de) and others.
#
# All rights reserved.
#
# See the file License for details.

# riopt.tcl - helpers to add RiOption tags to objects
# This file has a pendant named riattr.tcl!

proc riopt_addp { } {
    global ay

    set w .addRiOptw
    catch {destroy $w}
    toplevel $w
    wm title $w "Add RiOption"
    wm iconname $w "Ayam"
    wm transient $w .

    set f [frame $w.f1]
    label $f.l -text "Option:"
    pack $f.l -in $f

    # scrollbar
    scrollbar $f.sc -command "$f.li yview" -takefocus 0
    pack $f.sc -in $f -side right -fill y -expand no

    # listbox
    pack $f -in $w -side top -fill both -expand yes
    listbox $f.li -width 24 -height 10 -selectmode single\
	-yscrollcommand "$f.sc set" -exportselection 0
    pack $f.li -in $f -side left -fill both -expand yes

    # center window
    winCenter $w
    global rioptval
    array set rioptval {
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
    global riopt
    set names [array names riopt]
    foreach name $names {
	set num [llength $riopt($name)]
	set i 0
	while {$i < $num} {
	    set opt [lindex $riopt($name) $i]
	    set optname [lindex $opt 0]
	    $f.li insert end "$name $optname"

	    incr i
	}
    }

    bind $f.li <ButtonRelease-1> {
	global riopt
	set lb .addRiOptw.f1.li
 
	# get matching attribute list entry
	set opt [$lb get [$lb curselection]]

	set optname [lindex $opt 0]
	set optpara [lindex $opt 1]

	set list $riopt($optname)
	foreach param $list {
	    if { $optpara == [lindex $param 0] } {
		# destroy old UI
		set wins [winfo children .addRiOptw.f2]
		foreach win $wins {
		    catch {destroy $win}
		}

		# create new UI
		set f .addRiOptw.f2
		label $f.l -text "Value:"
		pack $f.l -in $f

		set type [lindex $param 1]
		set def {}
		set def [lindex $param 2]

		switch $type {

		    i { 
			if { $def != {} } {
			    set rioptval(Int) [lindex $def 0] } else {
				set rioptval(Int) 0
			}
			addParam .addRiOptw.f2 rioptval Int $def 
		      }
		    f {
			if { $def != {} } {
			    set rioptval(Float) [lindex $def 0] } else {
				set rioptval(Float) 0.0
			}
			addParam .addRiOptw.f2 rioptval Float $def
		      }
		    s { 
			if { $def != {} } {
			    set rioptval(String) [lindex $def 0] } else {
				set rioptval(String) ""
			}
			addString .addRiOptw.f2 rioptval String $def
		      }
		    p {

			addParam .addRiOptw.f2 rioptval Point_X
		        addParam .addRiOptw.f2 rioptval Point_Y
		        addParam .addRiOptw.f2 rioptval Point_Z
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

		    c {
			if { $def != {} } {
			    set rioptval(Color_R) [lindex [lindex $def 0] 0]
			    set rioptval(Color_G) [lindex [lindex $def 0] 1]
			    set rioptval(Color_B) [lindex [lindex $def 0] 2]
			} else {
			    set rioptval(Color_R) 0
			    set rioptval(Color_G) 0
			    set rioptval(Color_B) 0
			}
			addColor .addRiOptw.f2 rioptval Color $def
		      }
		      
		    j {
			if { $def != {} } {
			    set rioptval(IntPair_0) [lindex [lindex $def 0] 0]
			    set rioptval(IntPair_1) [lindex [lindex $def 0] 1]
			} else {
			    set rioptval(IntPair_0) 0
			    set rioptval(IntPair_1) 0
			}
			addParamPair .addRioptw.f2 rioptval IntPair $def
		    }
		    g {
			if { $def != {} } {
			 set rioptval(FloatPair_0) [lindex [lindex $def 0] 0]
			 set rioptval(FloatPair_1) [lindex [lindex $def 0] 1]
			} else {
			    set rioptval(FloatPair_0) 0.0
			    set rioptval(FloatPair_1) 0.0
			}
			addParamPair .addRioptw.f2 rioptval FloatPair $def
		    }
		}
	    }

	}

    }

    # entry
    set f [frame $w.f2]
    pack $f -in $w -side top -fill x -expand no

    # buttons
    set f [frame $w.f3]
    button $f.bok -text "Ok" -pady $ay(pady) -width 5 -command { 
	global riopt rioptval


	set lb .addRiOptw.f1.li
 
	set sel [$lb curselection]
	if { $sel != "" } {
	    set opt [$lb get $sel]

	    set optname [lindex $opt 0]
	    set optpara [lindex $opt 1]

	    set val ""
	    set list $riopt($optname)
	    foreach param $list {
		if { $optpara == [lindex $param 0] } {
		    catch {destroy [winfo children .addRiOptw.f2]}
		    set type [lindex $param 1]		
		    switch $type {
			
			i { set val $rioptval(Int) }
			f { set val $rioptval(Float) }
			s { set val $rioptval(String) }
			p { set val \
	"$rioptval(Point_X),$rioptval(Point_Y),$rioptval(Point_Z)" }
			c { set val \
	"$rioptval(Color_R),$rioptval(Color_G),$rioptval(Color_B)" }
                        j { set val \
	"$rioptval(IntPair_0),$rioptval(IntPair_1)" }
                        g { set val \
	"$rioptval(FloatPair_0),$rioptval(FloatPair_1)" }
			
		    }
		}

	    }

	    if { $val != "" } {
		global ay
		if { $ay(lb) == 1 } {
		    set lb $ay(olb)
		    set sel ""
		    set sel [$lb curselection]
		    selOb
		    selOb 0
		    undo save AddRiOpt
		    addTag RiOption "$optname,$optpara,$type,$val"
		    selOb $sel

		} else {
		    set t $ay(tree)
		    set sel ""
		    set sel [$t selection get]
		    treeSelect "root:0"
		    undo save AddRiOpt
		    addTag RiOption "$optname,$optpara,$type,$val"
		    if { $sel != "" } {
			$t selection set $sel
			tree_handleSelection
		    }
		}
		plb_update
	    } else {
		ayError 2 riopt_addp "No value entered!"
	    }

	} else {
	    ayError 2 riopt_addp "No option selected!"
	}

	focus .
	grab release .addRiOptw
	destroy .addRiOptw

    }

    button $f.bca -text "Cancel" -pady $ay(pady) -width 5 -command "\
	focus . ; destroy $w "

    pack $f.bok $f.bca -in $f -side left -fill x -expand yes
    pack $f -in $w -side bottom -fill x -expand no

    grab $w
    focus $f.bok
    tkwait window $w

 return;
}
# riopt_addp
