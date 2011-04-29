# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2009 by Randolf Schultz
# (randolf.schultz@gmail.com) and others.
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
	delTags "BP"
    }

 return;
}
# bevel_setTags


proc bevel_add { place arr } {
    global ay BevelTags $arr

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

    # apply changes to object
    if { $ay(shiftcommand) == 1 } {
	set ay(shiftcommand) 0
	update
	$ay(appb) invoke

	# update property GUI
	plb_update
    } else {
	# save current property GUI state
	foreach name [array names $arr] {
	    eval set savearray($name) $${arr}($name)
	}

	# update property GUI (temporarily destroys the property GUI state)
	plb_update

	# restore  property GUI state
	foreach name [array names $arr] {
	    eval set ${arr}($name) $savearray($name)
	}
    }
    # if

 return;
}
# bevel_add


proc bevel_rem { place arr } {
    global ay BevelTags $arr

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

    # apply changes to object?
    if { $ay(shiftcommand) == 1 } {
	set ay(shiftcommand) 0
	update
	$ay(appb) invoke

	# update property GUI
	plb_update
    } else {
	# save current property GUI state
	foreach name [array names $arr] {
	    eval set savearray($name) $${arr}($name)
	}

	# update property GUI (temporarily destroys the property GUI state)
	plb_update

	# restore  property GUI state
	foreach name [array names $arr] {
	    eval set ${arr}($name) $savearray($name)
	}
    }
    # if

 return;
}
# bevel_rem

set Bevel 1

# init_Bevel:
#
#
proc init_Bevel { } {
global ay Bevel_props BevelAttr BevelAttrData

set Bevel_props { Transformations Attributes Material Tags BevelAttr }

array set BevelAttr {
    arr   BevelAttrData
    sproc bevel_setAttr
    gproc bevel_getAttr
    w     fBevelAttr
}

array set BevelAttrData {
    DisplayMode 1
}

return;
}
# init_Bevel


# bevel_getAttr:
#  get Attributes from C context and build new PropertyGUI
#
proc bevel_getAttr { } {
    global ay BevelAttr BevelAttrData BevelTags

    set oldfocus [focus]

    # remove old, create new BevelAttr-UI
    catch {destroy $ay(pca).$BevelAttr(w)}
    set w [frame $ay(pca).$BevelAttr(w)]
    getProp

    set BevelTags(SBType) 0
    set BevelTags(SBRadius) 0.1
    set BevelTags(SBRevert) 0

    set tagnames ""
    set tagvalues ""
    getTags tagnames tagvalues
    bevel_parseTags $tagnames $tagvalues

    set BevelTags(BevelType) $BevelTags(SBType)
    set BevelTags(BevelRadius) $BevelTags(SBRadius)
    set BevelTags(BevelRevert) $BevelTags(SBRevert)

    set ay(bok) $ay(appb)

    addMenu $w BevelTags BevelType $ay(bevelmodes)
    addParam $w BevelTags BevelRadius
    addCheck $w BevelTags BevelRevert

    addParam $w BevelAttrData Tolerance
    addMenu $w BevelAttrData DisplayMode $ay(npdisplaymodes)

    # add UI to property canvas
    plb_setwin $w $oldfocus

 return;
}
# bevel_getAttr


proc bevel_setAttr { } {
    global BevelTags

    set BevelTags(HasStartBevel) 1
    set BevelTags(SBType) $BevelTags(BevelType)
    set BevelTags(SBRadius) $BevelTags(BevelRadius)
    set BevelTags(SBRevert) $BevelTags(BevelRevert)
    update
    bevel_setTags
    update
    setProp

 return;
}
# bevel_setAttr
