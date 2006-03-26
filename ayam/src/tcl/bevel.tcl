# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2001 by Randolf Schultz
# (rschultz@informatik.uni-rostock.de) and others.
#
# All rights reserved.
#
# See the file License for details.

# bevel.tcl - bevel tags processing Tcl code


proc bevel_parseTags { tagnames tagvalues } {
    global BevelTags

    array set BevelTags {
	HasStartBevel 0
	HasEndBevel 0
	HasLeftBevel 0
	HasRightBevel 0
    }

    if { [llength $tagnames] == 0 } { return }

    set l [llength $tagnames]
    for {set i 0} {$i < $l} {incr i} {

	set tagname [lindex $tagnames $i]

	if { $tagname == "BP" } {
	    set tagvalue [lindex $tagvalues $i]
	    set bplace -1
	    scan $tagvalue "%d," bplace
	    if { $bplace == 0 } {
		set BevelTags(HasStartBevel) 1
		scan $tagvalue "%d,%d,%lg,%d" dummy BevelTags(SBType) \
			BevelTags(SBRadius) BevelTags(SBRevert)
	    }
	    if { $bplace == 1 } {
		set BevelTags(HasEndBevel) 1
		scan $tagvalue "%d,%d,%lg,%d" dummy BevelTags(EBType) \
			BevelTags(EBRadius) BevelTags(EBRevert)
	    }
	    if { $bplace == 2 } {
		set BevelTags(HasLeftBevel) 1
		scan $tagvalue "%d,%d,%lg,%d" dummy BevelTags(LBType) \
			BevelTags(LBRadius) BevelTags(LBRevert)
	    }
	    if { $bplace == 3 } {
		set BevelTags(HasRightBevel) 1
		scan $tagvalue "%d,%d,%lg,%d" dummy BevelTags(RBType) \
			BevelTags(RBRadius) BevelTags(RBRevert)
	    }
	}
	# if
    }
    # for

 return;
}
# bevel_parseTags


proc bevel_setTags { } {
    global BevelTags

    set newtags ""
    set oldtagnames ""
    set oldtagvalues ""

    getTags oldtagnames oldtagvalues

    if { [llength $oldtagnames] != 0 } {
	set l [llength $oldtagnames]
	for {set i 0} {$i < $l} {incr i} {
	    set tagname [lindex $oldtagnames $i]
	    if { $tagname != "BP" } {
		lappend newtags $tagname
		lappend newtags [lindex $oldtagvalues $i]
	    }
	    # if
	}
	# for
    }
    # if

    if { $BevelTags(HasStartBevel) } {
	lappend newtags BP
	lappend newtags [format "0,%d,%f,%d" $BevelTags(SBType)\
		$BevelTags(SBRadius) $BevelTags(SBRevert)]
    }
    if { $BevelTags(HasEndBevel) } {
	lappend newtags BP
	lappend newtags [format "1,%d,%f,%d" $BevelTags(EBType)\
		$BevelTags(EBRadius) $BevelTags(EBRevert)]
    }
    if { $BevelTags(HasLeftBevel) } {
	lappend newtags BP
	lappend newtags [format "2,%d,%f,%d" $BevelTags(LBType)\
		$BevelTags(LBRadius) $BevelTags(LBRevert)]
    }
    if { $BevelTags(HasRightBevel) } {
	lappend newtags BP
	lappend newtags [format "3,%d,%f,%d" $BevelTags(RBType)\
		$BevelTags(RBRadius) $BevelTags(RBRevert)]
    }

    if { $newtags != "" } {
	eval [subst "setTags $newtags"]
    } else {
	delTags all
    }

 return;
}
# bevel_setTags


proc bevel_add { place } {
    global BevelTags

    # set up array
    if { $place == 0 } {
	set BevelTags(HasStartBevel) 1
	set BevelTags(SBType) 0
	set BevelTags(SBRadius) 0.1
	set BevelTags(SBRevert) 0
    }
    if { $place == 1 } {
	set BevelTags(HasEndBevel) 1
	set BevelTags(EBType) 0
	set BevelTags(EBRadius) 0.1
	set BevelTags(EBRevert) 0
    }
    if { $place == 2 } {
	set BevelTags(HasLeftBevel) 1
	set BevelTags(LBType) 0
	set BevelTags(LBRadius) 0.1
	set BevelTags(LBRevert) 0
    }
    if { $place == 3 } {
	set BevelTags(HasRightBevel) 1
	set BevelTags(RBType) 0
	set BevelTags(RBRadius) 0.1
	set BevelTags(RBRevert) 0
    }

    # create tags
    bevel_setTags

    # update property GUI
    plb_update

 return;
}
# bevel_add


proc bevel_rem { place } {
    global BevelTags

    # set up array
    if { $place == 0 } {
	set BevelTags(HasStartBevel) 0
    }
    if { $place == 1 } {
	set BevelTags(HasEndBevel) 0
    }
    if { $place == 2 } {
	set BevelTags(HasLeftBevel) 0
    }
    if { $place == 3 } {
	set BevelTags(HasRightBevel) 0
    }

    # create tags
    bevel_setTags

    # update property GUI
    plb_update

 return;
}
# bevel_rem
