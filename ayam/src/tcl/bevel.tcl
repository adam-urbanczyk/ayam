# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2001 by Randolf Schultz
# (rschultz@informatik.uni-rostock.de) and others.
#
# All rights reserved.
#
# See the file License for details.

# bevel.tcl - bevel tags processing Tcl code and Bevel object code


proc bevel_parseTags { tagnames tagvalues } {
    global BevelTags

    array set BevelTags {
	HasStartBevel 0
	HasEndBevel 0
	HasLeftBevel 0
	HasRightBevel 0
    }

    if { [llength $tagnames] == 0 } { return; }

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
    global ay BevelTags

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

    # apply changes to object?
    if { $ay(shiftcommand) == 1 } {
	set ay(shiftcommand) 0
	update
	$ay(appb) invoke
    }

 return;
}
# bevel_add


proc bevel_rem { place } {
    global ay BevelTags

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

    # apply changes to object?
    if { $ay(shiftcommand) == 1 } {
	set ay(shiftcommand) 0
	update
	$ay(appb) invoke
    }

 return;
}
# bevel_rem


set Bevel_props { Transformations Attributes Material Tags BevelAttr }

proc bevel_getAttr { } {
    global ay BevelAttr BevelAttrData BevelTags
 
    # create BevelAttr-UI
    catch {destroy $ay(pca).$BevelAttr(w)}
    set w [frame $ay(pca).$BevelAttr(w)]
    getProp

    set tagnames ""
    set tagvalues ""
    getTags tagnames tagvalues
    bevel_parseTags $tagnames $tagvalues

    set BevelTags(BevelType) $BevelTags(SBType)
    set BevelTags(BevelRadius) $BevelTags(SBRadius)
    set BevelTags(BevelRevert) $BevelTags(SBRevert)

    addMenu $w BevelTags BevelType $ay(bevelmodes)
    addParam $w BevelTags BevelRadius
    addCheck $w BevelTags BevelRevert

    addParam $w BevelAttrData Tolerance
    addMenu $w BevelAttrData DisplayMode $ay(npdisplaymodes)

    $ay(pca) itemconfigure 1 -window $w
    update
    plb_resize

    # adapt scrollregion
    set width [expr [winfo reqwidth $w] + 10]
    set height [expr [winfo reqheight $w] + 10]
    $ay(pca) configure -scrollregion [list 0 5 $width $height]

 return;
}
# bevel_getAttr


proc bevel_setAttr { } {
    global BevelTags

    set BevelTags(SBType) $BevelTags(BevelType)
    set BevelTags(SBRadius) $BevelTags(BevelRadius)
    set BevelTags(SBRevert) $BevelTags(BevelRevert)

    bevel_setTags
    setProp

 return;
}
# bevel_setAttr

array set BevelAttr {
    arr   BevelAttrData
    sproc bevel_setAttr
    gproc bevel_getAttr
    w     fBevelAttr
}

array set BevelAttrData {
    DisplayMode 1
}


#bevel_crt:
#
#
proc bevel_crt { } {
    global ay ay_error selected

    set selected ""
    getSel selected
    if { $selected == "" } { return; }
    cutOb

    set ay_error 0
    crtOb Bevel
    if { $ay_error } {  return; }
    set ay(ul) $ay(CurrentLevel)
    uS
    sL
    global BevelTags

    set BevelTags(HasStartBevel) 1
    set BevelTags(HasEndBevel) 0
    set BevelTags(HasLeftBevel) 0
    set BevelTags(HasRightBevel) 0
    set BevelTags(SBType) 0
    set BevelTags(SBRadius) 0.1
    set BevelTags(SBRevert) 0

    bevel_setTags

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
# bevel_crt