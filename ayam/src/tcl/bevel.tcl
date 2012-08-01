# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2009 by Randolf Schultz
# (randolf.schultz@gmail.com) and others.
#
# All rights reserved.
#
# See the file License for details.

# bevel.tcl - bevel tags processing Tcl code and Bevel object code


proc bevel_parseTags { tagnames tagvalues bnames } {
    global BevelTags

    array set BevelTags {
	Bevel0 0
	Bevel1 0
	Bevel2 0
	Bevel3 0
    }

    if { [llength $tagnames] == 0 } { return; }

    set l [llength $tagnames]
    for {set i 0} {$i < $l} {incr i} {

	set tagname [lindex $tagnames $i]

	if { $tagname == "BP" } {
	    set tagvalue [lindex $tagvalues $i]
	    set bplace -1
	    scan $tagvalue "%d," bplace

	    set BevelTags(Bevel${bplace}) 1

	    set bname [lindex $bnames $bplace]

	    scan $tagvalue "%d,%d,%lg,%d,%d" dummy type \
		BevelTags(${bname}Radius) BevelTags(${bname}Revert) \
		BevelTags(${bname}Integrate)
	    if { $type < 0 } {
		set BevelTags(${bname}Type) [expr 2 + (-$type)]
	    } else {
		set BevelTags(${bname}Type) $type
	    }
	}
	# if
    }
    # for

 return;
}
# bevel_parseTags


proc bevel_setTags { bnames } {
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

    set i 0
    foreach bname $bnames {
	if { $BevelTags(Bevel${i}) } {
	    lappend newtags BP
	    set type $BevelTags(${bname}Type)
	    if { $type > 2 } {
		set type [expr 2 - $type]
	    }
puts "newtype: $type"
	    lappend newtags [format "%d,%d,%f,%d,%d" $i\
				 $type\
				 $BevelTags(${bname}Radius)\
				 $BevelTags(${bname}Revert)\
				 $BevelTags(${bname}Integrate)]
	}
	incr i
    }
    # foreach

    if { $newtags != "" } {
	eval [subst "setTags $newtags"]
    } else {
	delTags "BP"
    }

 return;
}
# bevel_setTags


proc bevel_add { bplace arr } {
    global ay BevelTags $arr

    eval set bnames \$${arr}(BoundaryNames)
    set bname [lindex $bnames $bplace]

    # switch the bevel
    set BevelTags(Bevel${bplace}) 1
    # set up some default values
    set BevelTags(${bname}Type) 0
    set BevelTags(${bname}Radius) 0.1
    set BevelTags(${bname}Revert) 0
    set BevelTags(${bname}Integrate) 0

    # create tags
    bevel_setTags $bnames

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
# bevel_add


proc bevel_rem { bplace arr } {
    global ay BevelTags $arr

    eval set bnames \$${arr}(BoundaryNames)

    # switch the bevel
    set BevelTags(Bevel${bplace}) 0

    # create tags
    bevel_setTags $bnames

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
# bevel_rem

set Bevel 1

# init_Bevel:
#
#
proc init_Bevel { } {
global ay Bevel_props BevelAttr BevelAttrData

set Bevel_props { Transformations Attributes Material Tags Caps BevelAttr }

array set BevelAttr {
    arr   BevelAttrData
    sproc bevel_setAttr
    gproc bevel_getAttr
    w     fBevelAttr
}

array set BevelAttrData {
    DisplayMode 1
    BoundaryNames {"Start" "End"}
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

    set tagnames ""
    set tagvalues ""
    getTags tagnames tagvalues
    bevel_parseTags $tagnames $tagvalues Bevel

    set ay(bok) $ay(appb)

    addVSpace $w s1 2
    addMenu $w BevelTags BevelType $ay(bevelmodeswc)
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
    global BevelAttrData BevelTags

    set BevelTags(BevelIntegrate) 0
    bevel_setTags Bevel

    setProp

 return;
}
# bevel_setAttr


#
# Bevels property as e.g. used by Birail1 tool object
#

array set Bevels {
arr   BevelsData
sproc bevel_setBevels
gproc bevel_getBevels
w     fBevels
}

proc bevel_getBevels { } {
    global ay BevelTags Bevels BevelsData

    set oldfocus [focus]

    # remove old, create new Bevels-UI
    catch {destroy $ay(pca).$Bevels(w)}
    set w [frame $ay(pca).$Bevels(w)]

    set ay(bok) $ay(appb)

    getProp

    set type ""
    getType type
    if { $type != "" } {
	global ${type}AttrData

	if { [info exists ${type}AttrData(BoundaryNames)] } {

	eval set bnames \$${type}AttrData(BoundaryNames)

	set tagnames ""
	set tagvalues ""
	getTags tagnames tagvalues
	bevel_parseTags $tagnames $tagvalues $bnames

	set i 0
	foreach bname $bnames {
	    if { $BevelTags(Bevel${i}) } {
		set str "Remove "
		append str $bname
		append str " Bevel!"
		set cmd "bevel_rem "
		append cmd $i
		append cmd " ${type}AttrData"
		addCommand $w c$i $str $cmd
		addMenu $w BevelTags ${bname}Type $ay(bevelmodeswc)
		addParam $w BevelTags ${bname}Radius
		addCheck $w BevelTags ${bname}Revert
		addCheck $w BevelTags ${bname}Integrate
	    } else {
		set str "Add "
		append str $bname
		append str " Bevel!"
		set cmd "bevel_add "
		append cmd $i
		append cmd " ${type}AttrData"
		addCommand $w c$i $str $cmd
	    }

	    incr i
	}
    } else {
	addText $w e1 "Not supported here!"
    }

    }
    # if

    # add UI to property canvas
    plb_setwin $w $oldfocus

 return;
}
# bevel_getBevels


proc bevel_setBevels { } {

    set type ""
    getType type
    if { $type != "" } {
	global ${type}AttrData
	eval set bnames \$${type}AttrData(BoundaryNames)
	bevel_setTags $bnames
	set ${type}AttrData(BevelsChanged) 1
	setProp
    }

 return;
}
# bevel_setBevels


proc bevel_updateCustomBevels { } {
    global ay
    set ay(bevelmodeswc) $ay(bevelmodes)
    goTop
    getLevel names types
    set i 0
    foreach name $names {
	if { [string first "Bevels" $name] == 0 } {
	    if { [lindex $types $i] eq "Level" } {
		goDown $i
		getLevel cnames ctypes
		set j 0
		foreach ctype $ctypes {
		    if { $ctype eq "NCurve" } {
			set cname [lindex $cnames $j]
			if { [string first "(" $cname] > 0 } {
			    set index [string first "(" $cname]
			    incr index -1
			    lappend ay(bevelmodeswc)\
				[string range $cname 0 $index]
			} else {
			    lappend ay(bevelmodeswc) $cname
			}
		    }
		    # if
		    incr j
		}
		# foreach
	    }
	    # if
	}
	# if
	incr i
    }
    # foreach
    goTop
    uS
 return;
}
# bevel_updateCustomBevels