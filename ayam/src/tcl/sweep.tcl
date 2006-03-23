# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2001 by Randolf Schultz
# (rschultz@informatik.uni-rostock.de) and others.
#
# All rights reserved.
#
# See the file License for details.

# sweep.tcl - sweep objects Tcl code

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

set Sweep_props { Transformations Attributes Material Tags SweepAttr }

proc sweep_getAttr { } {
global ay SweepAttr SweepAttrData BevelTags
# create SweepAttr-UI

catch {destroy $ay(pca).$SweepAttr(w)}
set w [frame $ay(pca).$SweepAttr(w)]
getProp

set tagnames ""
set tagvalues ""
getTags tagnames tagvalues
bevel_parseTags $tagnames $tagvalues

addMenu $w SweepAttrData Type [list Open Closed Periodic]
addCheck $w SweepAttrData Rotate
addCheck $w SweepAttrData Interpolate
addParam $w SweepAttrData Sections
addCheck $w SweepAttrData StartCap
addCheck $w SweepAttrData EndCap

if { $BevelTags(HasStartBevel) } {
    addCommand $w c1 "Remove Start Bevel!" "bevel_rem 0"
    addMenu $w BevelTags SBType [list Round Linear Ridge]
    addParam $w BevelTags SBRadius
    addCheck $w BevelTags SBRevert
} else {
    addCommand $w c1 "Add Start Bevel!" "bevel_add 0"
}

if { $BevelTags(HasEndBevel) } {
    addCommand $w c2 "Remove End Bevel!" "bevel_rem 1"
    addMenu $w BevelTags EBType [list Round Linear Ridge]
    addParam $w BevelTags EBRadius
    addCheck $w BevelTags EBRevert
} else {
    addCommand $w c2 "Add End Bevel!" "bevel_add 1"
}

addParam $w SweepAttrData Tolerance
addMenu $w SweepAttrData DisplayMode $ay(npdisplaymodes)

$ay(pca) itemconfigure 1 -window $w
update
plb_resize
# adapt scrollregion
set width [expr [winfo reqwidth $w] + 10]
set height [expr [winfo reqheight $w] + 10]
$ay(pca) configure -scrollregion [list 0 5 $width $height]

return;
}
# sweep_getAttr


proc sweep_setAttr { } {

    bevel_setTags
    setProp

 return;
}
# sweep_setAttr

array set SweepAttr {
arr   SweepAttrData
sproc sweep_setAttr
gproc sweep_getAttr
w     fSweepAttr

}

array set SweepAttrData {
Type 0
DisplayMode 1
}



#sweep_crt:
#
#
proc sweep_crt { } {
    global ay ay_error selected
    set selected ""
    getSel selected
    if { $selected == "" } { return; }
    set ay_error 0
    crtOb Sweep
    if { $ay_error } {  return; }

    cutOb
    set ay(ul) $ay(CurrentLevel)
    uS
    sL
    getLevel a b
    goDown [expr [llength $a]-1]
    cmovOb
    goUp
    set ay(ul) $ay(CurrentLevel)
    uS
    sL
    rV

 return;
}
# sweep_crt