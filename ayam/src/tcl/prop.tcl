# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2005 by Randolf Schultz
# (randolf.schultz@gmail.com) and others.
#
# All rights reserved.
#
# See the file License for details.

# prop.tcl - handle standard properties

# Transformations property

array set Transformations {
arr   transfPropData
sproc setTrafo
gproc getTrafo
w     fTrafoAttr
}

array set transfPropData {
    QuaternionBall "n/a"
}

proc resetTrafo { { undo 0 } { redraw 0 } } {
 global ay transfPropData

    if { $undo } {
	undo save ResetTrafos
    }

    set transfPropData(Translate_X) 0.0
    set transfPropData(Translate_Y) 0.0
    set transfPropData(Translate_Z) 0.0

    set transfPropData(Scale_X) 1.0
    set transfPropData(Scale_Y) 1.0
    set transfPropData(Scale_Z) 1.0

    set transfPropData(Rotate_X) 0.0
    set transfPropData(Rotate_Y) 0.0
    set transfPropData(Rotate_Z) 0.0

    set transfPropData(Quat0) 0.0
    set transfPropData(Quat1) 0.0
    set transfPropData(Quat2) 0.0
    set transfPropData(Quat3) 1.0

    set transfPropData(Quaternion) "\[0, 0, 0, 1\]"

    setTrafo

    if { $redraw } {
	rV
    }

 return;
}

proc resetRotate { { undo 0 } { redraw 0 } } {
 global ay transfPropData

    if { $undo } {
	undo save ResetRotate
    }

    getTrafo

    set transfPropData(Rotate_X) 0.0
    set transfPropData(Rotate_Y) 0.0
    set transfPropData(Rotate_Z) 0.0

    set transfPropData(Quat0) 0.0
    set transfPropData(Quat1) 0.0
    set transfPropData(Quat2) 0.0
    set transfPropData(Quat3) 1.0

    set transfPropData(Quaternion) "\[0, 0, 0, 1\]"

    setTrafo

    if { $redraw } {
	rV
    }

 return;
}

# create Transformations-UI
set w [frame $ay(pca).$Transformations(w)]
addVSpace $w s1 2
addCommand $w c1 "Reset All!" {resetTrafo 1 1}

addText $w e1 "Translation:"
addParam $w transfPropData Translate_X
addParam $w transfPropData Translate_Y
addParam $w transfPropData Translate_Z

addText $w e2 "Rotation:"
addParam $w transfPropData Rotate_X
addParam $w transfPropData Rotate_Y
addParam $w transfPropData Rotate_Z
addInfo $w transfPropData Quaternion

addText $w e3 "Scale:"
addParam $w transfPropData Scale_X
addParam $w transfPropData Scale_Y
addParam $w transfPropData Scale_Z

# Attributes property

proc setAttrp { } {
    global attrPropData
    # replace all whitespace/control characters by underscores
    regsub -all "\[\[:space:\]\[:cntrl:\]\]" $attrPropData(Objectname)\
	"_" attrPropData(Objectname)
    setAttr
    uCL cl "0 1"
 return;
}

array set Attributes {
arr   attrPropData
sproc setAttrp
gproc getAttr
w     fAttrAttr

}

# create Attributes-UI
set w [frame $ay(pca).$Attributes(w)]
addVSpace $w s1 2
addString $w attrPropData Objectname
addCheck $w attrPropData Hide
addCheck $w attrPropData HideChildren
addInfo $w attrPropData RefCount


# Material property

array set Material {
arr   matPropData
sproc setMat
gproc getMat
w     fMatAttr

}
# create Material-UI
set w [frame $ay(pca).$Material(w)]
addVSpace $w s1 2
addCommand $w c1 "Clear Material!" {
    undo save ClrMat
    global matPropData; set matPropData(Materialname) ""
    setMat
    plb_update
}

addCommand $w c2 "Add/Edit Material!" {
    global ay
    set m $ay(editmenu)
    $m invoke 15
}

addString $w matPropData Materialname


# Tags property

# copyTagp:
# manage context menu entries ( Copy, Copy (Add) )
proc copyTagp { i {add 0} } {
global tagsPropData pclip_prop
upvar #0 pclip_clipboard clipboard

    if { $add == 0 } {
	catch {unset clipboard}
	set pclip_prop Tags
	set clipboard(names) [lindex $tagsPropData(names) $i]
	set clipboard(values) [lindex $tagsPropData(values) $i]
    } else {
	if { ($pclip_prop != "Tags") || (![info exists clipboard(names)]) } {
	    copyTagp $i 0
	    return;
	}
	lappend clipboard(names) [lindex $tagsPropData(names) $i]
	lappend clipboard(values) [lindex $tagsPropData(values) $i]
    }
    return;
}
# copyTagp


# getTagsp:
#
proc getTagsp { } {
global ay ayprefs aymainshortcuts tagsPropData Tags tcl_platform

getTags names values
set tagsPropData(names) $names
set tagsPropData(values) $values

set ay(bok) $ay(appb)

# create Tags-UI
catch {destroy $ay(pca).$Tags(w)}
set w [frame $ay(pca).$Tags(w)]
addVSpace $w s1 2
addCommand $w c0 "Remove all Tags!" {undo save RemTags;delTags all;plb_update}

set bw 1
set f [frame $w.fDelete -relief sunken -bd $bw]
menubutton $f.mb -text "Remove Tag!" -menu $f.mb.m -relief raised -bd $bw\
	-padx 0 -pady 1 -takefocus 1 -highlightthickness 1\
	-indicatoron 1
eval [subst "bindtags $f.mb \{$f.mb Menubutton all\}"]
bind $f.mb <Key-Escape> "resetFocus;break"
if { $tcl_platform(platform) == "windows" } {
    $f.mb configure -pady 1
}
set m [menu $f.mb.m -tearoff 0]
pack $f.mb -in $f -side left -fill x -expand yes -pady 0
pack $f -in $w -side top -fill x

set i 0
foreach tag $names {
    $m add command -label "Tag#$i ($tag)" -command\
	"undo save RemTag;setTags -delete $i;plb_update"
    incr i
}

addCommand $w c2 "Add Tag!" {addTagp}

set i 0
foreach tag $names {
    set val [lindex $values $i]
    set ind [string first "\n" $val]
    if { $ind > -1 } {
	incr ind -1
	if { $ind >= 0 } {
	    set val [string range $val 0 $ind]
	}
	set val "${val}..."
    }
    set len [string length $val]
    if { $len > $ayprefs(MaxTagLen) } {
	set val [string range $val 0 $ayprefs(MaxTagLen)]
	set val "${val}..."
    }
    set b $w.b$i
    button $b -text "$tag: $val" -command "addTagp $i" -bd 1 -pady 0
    pack $b -fill x -expand yes

    set m [menu $b.popup -tearoff 0]
    $m add command -label "Copy" -command "copyTagp $i"
    $m add command -label "Copy (Add)" -command "copyTagp $i 1"
    bind $b <ButtonPress-$aymainshortcuts(CMButton)> "winOpenPopup $b"

    incr i
}

    plb_setwin $w ""

}
# getTagsp


# setTagsp:
#
proc setTagsp { } {
global ay tagsPropData Tags

set names $tagsPropData(names)
set values $tagsPropData(values)

set alltags ""
set i 0
foreach tag $names {
    lappend alltags $tag
    lappend alltags [lindex $values $i]
    incr i
}

if { [llength $alltags] > 0 } {
    eval [subst "setTags $alltags"]
}

notifyOb
rV
}
# setTagsp


# editTagshelper:
#  used to edit tags
proc editTagshelper { index } {
 global tagsPropData
 undo save EditTag
 set tagsPropData(names) [lreplace $tagsPropData(names) $index $index [.addTag.fu.e get]]
 set val [string trimright [.addTag.fm.t get 1.0 end]]
 set tagsPropData(values) [lreplace $tagsPropData(values) $index $index $val]
 grab release .addTag
 focus .
 destroy .addTag
 setTagsp
 plb_update
}
#editTagshelper


# addTagp:
#  used to edit and add tags
proc addTagp { {edit -1} } {
global ay ayprefs tagsPropData Tags

winAutoFocusOff

set w .addTag
if { $edit >= 0 } {
    set t "Edit Tag"
} else {
    set t "Add Tag"
}
winDialog $w $t
if { $ayprefs(FixDialogTitles) == 1 } {
    pack [frame $w.fl] -in $w -side top
    pack [label $w.fl.l -text $t] -in $w.fl -side left -fill x -expand yes
}
set f [frame $w.fu]
label $f.lt -text "Type:" -width 6
entry $f.e -width 30
eval [subst "bindtags $f.e \{$f.e Entry all\}"]
bind $f.e <Key-Escape> ".addTag.fd.bca invoke;break"
bind $f.e <Key-Return> ".addTag.fd.bok invoke;break"
catch {bind $f.e <Key-KP_Enter> ".addTag.fd.bok invoke;break"}
uie_fixEntry $f.e

set mbs [expr [winfo reqheight $f.e] - 4]
if { $ay(ws) == "Aqua" } {
    incr mbs -8
}
if { $ay(ws) == "Win32" } {
    incr mbs 4
}

set mb [menubutton $f.mb -height $mbs -width $mbs -bd 2\
	    -image ay_Triangle_img -takefocus 0\
	    -highlightthickness 0 -relief raised -menu $f.mb.m]

if { $ay(ws) == "Aqua" } {
    $mb conf -height [$f.e cget -height]
}

set m [menu $mb.m -tearoff 0]
foreach val $ayprefs(Tags) {
    set tt [lindex $val 0]
    $m add command -label $val\
	-command "$f.e delete 0 end; $f.e insert end $tt;"
}

pack $f.lt -in $f -padx 2 -pady 2 -side left
pack $f.e -in $f -padx 2 -pady 2 -side left -fill x -expand yes
pack $f.mb -in $f -padx 2 -pady 2 -side left
pack $f -in $w -side top -fill x -expand no

if { $edit >= 0 } {
    $f.e insert 0 [lindex $tagsPropData(names) $edit]
}

set f [frame $w.fm]
label $f.lv -text "Value:" -width 6
text $f.t -width 30 -height 1
eval [subst "bindtags $f.t \{$f.t Text all\}"]
bind $f.t <Key-Escape> ".addTag.fd.bca invoke;break"
set height 24
incr height [expr [font metrics [$f.t cget -font] -linespace] - 13]
bind $f.t <Key-Return> "\
if \{\[winfo height $f.t\] < $height \} \{.addTag.fd.bok invoke;break\}"
catch {bind $f.t <Key-KP_Enter> [bind $f.t <Key-Return>]}
uie_fixEntry $f.t

pack $f.lv -in $f -padx 2 -pady 2 -side left
pack $f.t -in $f -padx 2 -pady 2 -side left -fill both -expand yes
pack $f -in $w -side top -fill both -expand yes

if { $edit >= 0 } {
    $f.t insert end [lindex $tagsPropData(values) $edit]
}

set f [frame $w.fd]
button $f.bok -text "Ok" -pady $ay(pady) -width 5 -command {
    global ay
    undo save AddTag
    if { [.addTag.fu.e get] != "" } {
	addTag [.addTag.fu.e get] [string trimright [.addTag.fm.t get 1.0 end]]
    }
    grab release .addTag
    focus .
    destroy .addTag
    plb_update
}

button $f.bclr -text "Clear" -pady $ay(pady) -width 5 -command {
    global ay
    .addTag.fu.e delete 0 end
    .addTag.fm.t delete 1.0 end
}

if { $edit >= 0 } {
    $f.bok configure -command "editTagshelper $edit"
}

button $f.bca -text "Cancel" -pady $ay(pady) -width 5\
    -command "grab release .addTag; focus .; destroy $w"

pack $f.bok $f.bclr $f.bca -in $f -side left -fill x -expand yes
pack $f -in $w -side top -fill x

# Esc-key && close via window decoration == Cancel button
bind $w <Escape> "$f.bca invoke"
wm protocol $w WM_DELETE_WINDOW "$f.bca invoke"

winRestoreOrCenter $w $t
grab $w
if { $edit >= 0 } {
    focus .addTag.fm.t
} else {
    focus .addTag.fu.e
}
tkwait window $w

winAutoFocusOn

 return;
}
# addTagp

array set Tags {
arr   tagsPropData
sproc setTagsp
gproc getTagsp
w     fTagsAttr
}


# create Tags-UI
set w [frame $ay(pca).$Tags(w)]
addCommand $w c1 "Remove all Tags" {delTags all;plb_update}


# getProperty:
#
#
proc getProperty { property varName {report 0} } {
upvar $varName vname
global ay ay_error

    set vname ""

    # decode arrayname from property argument
    set arrayname [string range $property 0 \
		       [expr [string first "(" $property]-1]]

    set propname [string range $property \
		       [expr [string first "(" $property]+1] \
		       [expr [string first ")" $property]-1]]

    getType types
    if { $types == "" } {
	ayError 2 "getProperty" "Could not get type of object."
	return;
    }
    set obj 0
    foreach type $types {
	withOb $obj {
	    # get list of properties of selected object
	    global ${type}_props
	    if { ! [info exists ${type}_props] } {
		init_${type}
	    }
	    eval [subst "set props {\$${type}_props}"]

	    # also get properties from NP tags
	    set tn ""
	    getTags tn tv
	    if { ($tn != "") && ([ string first NP $tn ] != -1) } {
		set i 0
		foreach tag $tn {
		    if { [lindex $tn $i] == "NP" } {
			lappend props [lindex $tv $i]
		    }
		    incr i
		}
		# foreach
	    }
	    # if

	    # check presence of wanted property for selected object
	    set propindex ""
	    catch {set propindex [lsearch -exact $props $arrayname]}
	    if { $propindex == -1 } {
		if { $report } {
		    ayError 2 "getProperty" "Could not find property: $arrayname."
		}
		return 0;
	    }

	    # get property data
	    global $arrayname
	    set getprocp ""
	    eval [subst "set getprocp {\$${arrayname}(gproc)}"]
	    if { $getprocp != "" } { eval $getprocp } else { getProp }

	    # fill value to variable
	    eval [subst "set arr \$${arrayname}(arr)"]
	    global $arr
	    set pvarname ${arr}($propname)
	    eval [subst "lappend vname \{\$$pvarname\}"]
	}
	incr obj
    }
 return 1;
}
# getProperty


# setProperty:
#
#
proc setProperty { property newValue } {
global ay curtypes ay_error

    # decode arrayname from property argument
    set arrayname [string range $property 0 \
		       [expr [string first "(" $property]-1]]

    set propname [string range $property \
		       [expr [string first "(" $property]+1] \
		       [expr [string first ")" $property]-1]]

    getType types
    if { $types == "" } {
	ayError 2 "setProperty" "Could not get type of object."
	return;
    }
    set obj 0
    foreach type $types {
	withOb $obj {
	    # get list of properties of selected object
	    global ${type}_props
	    if { ! [info exists ${type}_props] } {
		init_${type}
	    }
	    eval [subst "set props {\$${type}_props}"]

	    # also get properties from NP tags
	    set tn ""
	    getTags tn tv
	    if { ($tn != "") && ([ string first NP $tn ] != -1) } {
		set i 0
		foreach tag $tn {
		    if { [lindex $tn $i] == "NP" } {
			lappend props [lindex $tv $i]
		    }
		    incr i
		}
		# foreach
	    }
	    # if

	    # check presence of wanted property for selected object
	    set propindex ""
	    catch {set propindex [lsearch -exact $props $arrayname]}
	    if { $propindex == -1 } {
		ayError 2 "setProperty" "Could not find property: $arrayname."
		return;
	    }

	    # get property data
	    global $arrayname
	    set getprocp ""
	    eval [subst "set getprocp {\$${arrayname}(gproc)}"]
	    if { $getprocp != "" } { eval $getprocp } else { getProp }

	    # modify property data
	    eval [subst "set arr \$${arrayname}(arr)"]
	    global $arr
	    set pvarname ${arr}($propname)
	    eval [subst "set $pvarname \{\$newValue\}"]

	    # set property data
	    set setprocp ""
	    eval [subst "set setprocp {\$${arrayname}(sproc)}"]
	    if { $setprocp != "" } { eval $setprocp } else { setProp }
	}
	incr obj
    }
 return;
}
# setProperty


# markPropModified:
#  this procedure is used by traces to property variables and sets
#  the global variable <name1>(<name2>-Modified) to 1, where
#  name1 is a property data array (like NCurveAttrData) and
#  name2 is a single property in that array (like Knots)
proc markPropModified { name1 name2 op } {
    global $name1
    set ${name1}(${name2}-Modified) 1
}
# markPropModified


# prop_addrem:
#  add or remove a property from all selected objects by
#  management of their NP/RP tags
#  The global AddRemProp array is used to transfer the parameters
#  (actual operation mode, new property name) for this operation.
#  see: plb_addremprop
proc prop_addrem { } {
    global AddRemProp

    if { $AddRemProp(Property) != "" } {
	forAll -recursive 0 {
	    global AddRemProp

	    set tagnames ""
	    set tagvalues ""

	    getTags tagnames tagvalues

	    if { $AddRemProp(Operation) == 0 } {
		# add property
		set havetag 0

		set l [llength $tagnames]
		for {set j 0} {$j < $l} {incr j} {
		    set tagname [lindex $tagnames $j]
		    set tagval [lindex $tagvalues $j]
		    set remtag 0

		    if { $tagname == "NP" } {
			if { $tagval == $AddRemProp(Property) } {
			    set havetag 1
			}
		    }

		    if { $tagname == "RP" } {
			if { $tagval == $AddRemProp(Property) } {
			    set remtag 1
			    set havetag 1
			}
		    }

		    if { ! $remtag } {
			lappend newtags $tagname
			lappend newtags $tagval
		    }
		}
		# for

		if { $havetag == 0 } {
		    lappend newtags "NP"
		    lappend newtags $AddRemProp(Property)
		}

	    } else {
		# rem property (add RP tag)
		set havetag 0
		set l [llength $tagnames]
		for {set j 0} {$j < $l} {incr j} {
		    set tagname [lindex $tagnames $j]
		    set tagval [lindex $tagvalues $j]
		    set remtag 0

		    if { $tagname == "NP" } {
			if { $tagval == $AddRemProp(Property) } {
			    set remtag 1
			    set havetag 1
			}
		    }

		    if { $tagname == "RP" } {
			if { $tagval == $AddRemProp(Property) } {
			    set havetag 1
			}
		    }

		    if { ! $remtag } {
			lappend newtags $tagname
			lappend newtags $tagval
		    }
		}
		# for

		if { $havetag == 0 } {
		    lappend newtags "RP"
		    lappend newtags $AddRemProp(Property)
		}

	    }
	    # if
	    if { [info exists newtags] } {
		eval [subst "setTags $newtags"]
	    } else {
		delTags all
	    }
	}
	# forAll
    }
    # if

 return;
}
# prop_addrem
