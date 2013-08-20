# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2001 by Randolf Schultz
# (randolf.schultz@gmail.com) and others.
#
# All rights reserved.
#
# See the file License for details.

# cap.tcl - cap objects Tcl code

set Cap 1

proc init_Cap { } {
global ay Cap_props CapAttr CapAttrData

set Cap_props { Transformations Attributes Material Tags CapAttr }

array set CapAttr {
arr   CapAttrData
sproc ""
gproc ""
w     fCapAttr

}

array set CapAttrData {
Type 1
DisplayMode 0
NPInfoBall "n/a"
}
# create CapAttr-UI
set w [frame $ay(pca).$CapAttr(w)]

addMenu $w CapAttrData Type {Trim Gordon Simple Simple3D}
addParam $w CapAttrData Fraction
addParam $w CapAttrData Tolerance
addMenu $w CapAttrData DisplayMode $ay(npdisplaymodes)

addText $w CapAttrData "Created NURBS Patch:"
addInfo $w CapAttrData NPInfo

return;
}
# init_Cap


##


array set Caps {
arr   CapsData
sproc cap_setCaps
gproc cap_getCaps
w     fCaps
}

proc cap_parseTags { tagnames tagvalues bnames {bids ""} } {
    global ay CapTags

    array set CapTags {
	Cap0 0
	Cap1 0
	Cap2 0
	Cap3 0
    }

    if { [llength $tagnames] == 0 } { return; }

    set l [llength $tagnames]
    for {set i 0} {$i < $l} {incr i} {

	set tagname [lindex $tagnames $i]

	if { $tagname == "CP" } {
	    set tagvalue [lindex $tagvalues $i]
	    set bplace -1
	    scan $tagvalue "%d," bplace

	    if { $bids != "" } {
		set bplace [lindex $bids $bplace]
	    }

	    set CapTags(Cap${bplace}) 1

	    set bname [lindex $bnames $bplace]

	    scan $tagvalue "%d,%d,%d,%lg" dummy CapTags(${bname}Type) \
		CapTags(${bname}Integrate) CapTags(${bname}Fraction)

	}
	# if
    }
    # for

 return;
}
# cap_parseTags


proc cap_setTags { bnames {bids ""} } {
    global ay CapTags

    set newtags ""
    set oldtagnames ""
    set oldtagvalues ""

    getTags oldtagnames oldtagvalues

    if { [llength $oldtagnames] != 0 } {
	set l [llength $oldtagnames]
	for {set i 0} {$i < $l} {incr i} {
	    set tagname [lindex $oldtagnames $i]
	    if { $tagname != "CP" } {
		lappend newtags $tagname
		lappend newtags [lindex $oldtagvalues $i]
	    }
	    # if
	}
	# for
    }
    # if

    set i 0
    foreach bname $bnames {
	if { $bids != "" } {
	    set j [lindex $bids $i]
	} else {
	    set j $i
	}
	if { $CapTags(Cap${i}) } {
	    lappend newtags CP

	    lappend newtags [format "%d,%d,%d,%g" $j\
				 $CapTags(${bname}Type)\
				 $CapTags(${bname}Integrate)\
				 $CapTags(${bname}Fraction)]
	}
	incr i
    }
    # foreach

    if { $newtags != "" } {
	eval [subst "setTags $newtags"]
    } else {
	delTags "CP"
    }

 return;
}
# cap_setTags


proc cap_add { bplace arr bids } {
    global ay CapTags $arr

    eval set bnames \$${arr}(BoundaryNames)
    set bname [lindex $bnames $bplace]

    # switch the cap
    set CapTags(Cap${bplace}) 1
    # set up some default values
    set CapTags(${bname}Type) 0
    set CapTags(${bname}Integrate) 0
    set CapTags(${bname}Fraction) 0.5

    # create tags
    cap_setTags $bnames $bids

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

	# restore property GUI state
	foreach name [array names $arr] {
	    eval set ${arr}($name) {$savearray($name)}
	}
    }
    # if

 return;
}
# cap_add


proc cap_rem { bplace arr bids } {
    global ay CapTags $arr

    eval set bnames \$${arr}(BoundaryNames)

    # switch the cap
    set CapTags(Cap${bplace}) 0

    # create tags
    cap_setTags $bnames $bids

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

	# restore property GUI state
	foreach name [array names $arr] {
	    eval set ${arr}($name) {$savearray($name)}
	}
    }
    # if

 return;
}
# cap_rem

proc cap_getCaps { } {
    global ay CapTags Caps CapsData

    set oldfocus [focus]

    # remove old, create new Caps-UI
    catch {destroy $ay(pca).$Caps(w)}
    set w [frame $ay(pca).$Caps(w)]

    set ay(bok) $ay(appb)

    getProp

    set type ""
    getType type
    if { $type != "" } {
	global ${type}AttrData

	if { [info exists ${type}AttrData(BoundaryNames)] } {

	eval set bnames \$${type}AttrData(BoundaryNames)

	set bids ""
	if { [info exists ${type}AttrData(BoundaryIDs)] } {
	    eval set bids \$${type}AttrData(BoundaryIDs)
	}
	set tagnames ""
	set tagvalues ""
	getTags tagnames tagvalues
	cap_parseTags $tagnames $tagvalues $bnames $bids

	set i 0
	foreach bname $bnames {
	    if { $CapTags(Cap${i}) } {
		set str "Remove "
		append str $bname
		append str " Cap!"
		set cmd "cap_rem "
		append cmd $i
		append cmd " ${type}AttrData"
		append cmd " {$bids}"
		addCommand $w c$i $str $cmd
		addMenu $w CapTags ${bname}Type \
		    [list "Trim" "Gordon" "Simple" "Simple3D" ]
		addCheck $w CapTags ${bname}Integrate
		addParam $w CapTags ${bname}Fraction

	    } else {
		set str "Add "
		append str $bname
		append str " Cap!"
		set cmd "cap_add "
		append cmd $i
		append cmd " ${type}AttrData"
		append cmd " {$bids}"
		addCommand $w c$i $str $cmd
	    }

	    incr i
	}
	set ${type}AttrData(CapsChanged) 0
    } else {
	addText $w e1 "Not supported here!"
    }

    }
    # if

    # add UI to property canvas
    plb_setwin $w $oldfocus

 return;
}
# cap_getCaps


proc cap_setCaps { } {

    set type ""
    getType type
    if { $type != "" } {
	global ${type}AttrData
	eval set bnames \$${type}AttrData(BoundaryNames)
	set bids ""
	if { [info exists ${type}AttrData(BoundaryIDs)] } {
	    eval set bids \$${type}AttrData(BoundaryIDs)
	}
	cap_setTags $bnames $bids
	set ${type}AttrData(CapsChanged) 1
	setProp
    }

 return;
}
# cap_setCaps

