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
    Quat0 0.0
    Quat1 0.0
    Quat2 0.0
    Quat3 0.0
    Quaternion ""
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

proc getMaterialp { } {
    global ay Material matPropData

    set oldfocus [focus]

    getMat

    set ay(bok) $ay(appb)

    # create Material-UI
    catch {destroy $ay(pca).$Material(w)}
    set w [frame $ay(pca).$Material(w)]
    addVSpace $w s1 2
    addCommand $w c1 "Clear Material!" {
	undo save ClearMat
	global matPropData
	set matPropData(Materialname) ""
	setMat
	plb_update
    }

    addCommand $w c2 "Add/Edit Material!" {
	global ay
	set m $ay(editmenu)
	$m invoke 15
    }

    addString $w matPropData Materialname [getRegMats]

    plb_setwin $w $oldfocus

 return;
}

array set Material {
    arr   matPropData
    sproc setMat
    gproc getMaterialp
    w     fMatAttr
}


# Tags property

# copyTagp:
# manage context menu entries ( Copy (Replace), Copy (Add) )
proc copyTagp { i {add 0} } {
global ay tagsPropData pclip_prop
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

    focus $ay(pca)

 return;
}
# copyTagp


proc _remTags { {i -1} } {
    global ay
    set fw [focus]
    if { $i != -1 } {
	undo save RemTag
	setTags -delete $i
    } else {
	undo save RemTags
	delTags all
    }
    plb_update
    set i [lsearch -exact [$ay(plb) get 0 end] Tags]
    incr i 1
    if { $i > 0 } {plb_showprop $i}
    restoreFocus $fw
}


# getTagsp:
#
proc getTagsp { } {
    global ay ayprefs aymainshortcuts tagsPropData Tags tcl_platform

    set oldfocus [focus]

    getTags names values
    set tagsPropData(names) $names
    set tagsPropData(values) $values

    set ay(bok) $ay(appb)

    # create Tags-UI
    catch {destroy $ay(pca).$Tags(w)}
    set w [frame $ay(pca).$Tags(w)]
    addVSpace $w s1 2
    addCommand $w c0 "Remove all Tags!" _remTags

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
    if { $ay(ws) == "Aqua" } {
	pack conf $f.mb -ipady 0 -anchor n -expand 0
    }

    set m [menu $f.mb.m -tearoff 0]
    pack $f.mb -in $f -side left -fill x -expand yes -pady 0
    pack $f -in $w -side top -fill x

    set i 0
    foreach tag $names {
	$m add command -label "Tag#$i ($tag)" -command "_remTags $i"
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
	$m add command -label "Copy (Replace)" -command "copyTagp $i"
	$m add command -label "Copy (Add)" -command "copyTagp $i 1"
	$m add separator
	$m add command -label "Remove" -command "_remTags $i"
	if { $ay(ws) == "Win32" } {
	    bind $b <ButtonPress-$aymainshortcuts(CMButton)>\
		"after idle \{focus $b\}; bind $b.popup <Unmap>\
             \"$b conf -state normal\"; winOpenPopup $b"
	} else {
	    bind $b <ButtonPress-$aymainshortcuts(CMButton)>\
		"after idle \{$b conf -state active\}; bind $b.popup <Unmap>\
             \"$b conf -state normal\"; winOpenPopup $b"
	}

	bind $b <Key-F10> "winOpenPopup $b;break"
	bind $b <Key-space> "$b invoke;break"

	incr i
    }
    # foreach tag

    plb_setwin $w $oldfocus

 return;
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
	eval setTags $alltags
    }

    notifyOb
    rV

 return;
}
# setTagsp


# _commitTag:
#  used to add/edit tags
proc _commitTag { index } {
    global ay tagsPropData
    set ay(addTagHeight) [.addTag.fm.t cget -height]

    if { $index > -1 } {
	undo save EditTag
	if { [.addTag.fu.e get] != "" } {
	    setTags -index $index [.addTag.fu.e get]\
		[string trimright [.addTag.fm.t get 1.0 end]]
	} else {
	    setTags -delete $index
	}
    } else {
	undo save AddTag
	if { [.addTag.fu.e get] != "" } {
	    addTag [.addTag.fu.e get]\
		[string trimright [.addTag.fm.t get 1.0 end]]
	}
    }

    grab release .addTag
    focus .
    destroy .addTag

    plb_update

 return;
}
#_commitTag


# _pasteToText
# paste text from system clipboard to designated text widget
proc _pasteToText { t } {
    catch {set nt ""; set nt [selection get -selection CLIPBOARD]}
    if { $nt != "" } {
	$t delete 1.0 end
	$t insert end $nt
    }
}
# _pasteToText


# _loadToText
# load a text file to the designated text widget
proc _loadToText { t {types ""} } {

    lappend types  {"All files" *}
    set newfilename [tk_getOpenFile -filetypes $types -parent .\
			 -title "Select file to load:"]
    if { $newfilename != "" } {
	set tfile [open $newfilename r]
	if { $tfile != "" } {
	    set nt [read $tfile]
	    if { $nt != "" } {
		$t delete 1.0 end
		$t insert end $nt
	    }
	}
    }

 return;
}
# _loadToText


# _toggleMultiline
# toggle tag add/edit dialogs multiline feature
proc _toggleMultiline { t } {
    global ay
    if { $ay(addTagHeight) > 1 } {
	set ay(addTagHeight) 1
	$t conf -height 1
    } else {
	set ay(addTagHeight) 3;
	$t conf -height 3
    }
    set aygeom(addTag_Add_Tag) ""
    after idle { wm geom .addTag "" }
}
# _toggleMultiline


# addTagp:
#  used to edit and add tags
proc addTagp { {edit -1} } {
global ay ayprefs tagsPropData Tags

set focusWin [focus]

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
    $f.e conf -bd 2 -highlightthickness 1
}
if { $ay(ws) == "Win32" } {
    incr mbs 4
}

set mb [menubutton $f.mb -height $mbs -width $mbs -bd 2\
	    -image ay_Triangle_img -takefocus 0\
	    -highlightthickness 0 -relief raised -menu $f.mb.m]

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

# insert initial tag type
if { $edit >= 0 } {
    $f.e insert 0 [lindex $tagsPropData(names) $edit]
}

set f [frame $w.fm]
label $f.lv -text "Value:" -width 6

set he 1
if {[info exists ay(addTagHeight)]} {
    set he $ay(addTagHeight)
} else {
    set ay(addTagHeight) 1
}

text $f.t -width 30 -height $he
eval [subst "bindtags $f.t \{$f.t Text all\}"]
bind $f.t <Key-Escape> ".addTag.fd.bca invoke;break"
set height 24
incr height [expr [font metrics [$f.t cget -font] -linespace] - 13]
bind $f.t <Key-Return> "\
if \{\[winfo height $f.t\] < $height \} \{.addTag.fd.bok invoke;break\}"
catch {bind $f.t <Key-KP_Enter> [bind $f.t <Key-Return>]}
bind $f.t <Key-Tab> "\
if \{\[winfo height $f.t\] < $height \} \{focus \[tk_focusNext $f.t\];break\}"
uie_fixEntry $f.t

global ayviewshortcuts
bind $f.t <$ayviewshortcuts(RotMod)-Down> \
    "$f.t conf -height \[expr \[$f.t cget -height\] + 1\]; wm geom $w \"\";\
     break"
bind $f.t <$ayviewshortcuts(RotMod)-Up> \
    "$f.t conf -height \[expr \[$f.t cget -height\] - 1\]; wm geom $w \"\";\
     break"

if { $ay(ws) == "Aqua" } {
    $f.t conf -relief sunken -bd 2 -highlightthickness 1
}

pack $f.lv -in $f -padx 2 -pady 2 -side left
pack $f.t -in $f -padx 2 -pady 2 -side left -fill both -expand yes
pack $f -in $w -side top -fill both -expand yes

# create popup menu
set m [menu $f.t.popup -tearoff 0]
$m add command -label "Clear" -command "$f.t delete 1.0 end"
$m add command -label "Paste (Replace)" -command "_pasteToText $f.t"
$m add command -label "Load from file" -command "_loadToText $f.t"

$m add separator
$m add command -label "Toggle Multiline   <$ayviewshortcuts(RotMod)-Down> / <$ayviewshortcuts(RotMod)-Up>" -command "_toggleMultiline $f.t"

# bind popup menu
set mb 3
if { $ay(ws) == "Aqua" && $ayprefs(SwapMB) } {
    set mb 2
}
bind $f.t <$mb> {
    set ay(xy) [winfo pointerxy .]
    tk_popup .addTag.fm.t.popup [lindex $ay(xy) 0] [lindex $ay(xy) 1]
}

# insert initial tag value
if { $edit >= 0 } {
    $f.t insert end [lindex $tagsPropData(values) $edit]
}

# create ok/clear/cancel buttons
set f [frame $w.fd]
button $f.bok -text "Ok" -pady $ay(pady) -width 5 -command "_commitTag $edit"

button $f.bclr -text "Clear" -pady $ay(pady) -width 5 -command {
    global ay
    .addTag.fu.e delete 0 end
    .addTag.fm.t delete 1.0 end
}

button $f.bca -text "Cancel" -pady $ay(pady) -width 5\
    -command "global ay; set ay(addTagHeight) \[.addTag.fm.t cget -height\];\
              grab release .addTag; focus .; destroy $w"

pack $f.bok $f.bclr $f.bca -in $f -side left -fill x -expand yes
pack $f -in $w -side top -fill x

# Esc-key && close via window decoration == Cancel button
bind $w <Escape> "$f.bca invoke"
wm protocol $w WM_DELETE_WINDOW "$f.bca invoke"

# establish "Help"-binding
shortcut_addcshelp $w ayam-4.html tagsprop

set t "Add Tag"
winRestoreOrCenter $w $t
grab $w
if { $edit >= 0 } {
    focus .addTag.fm.t
} else {
    focus .addTag.fu.e
}
tkwait window $w

winAutoFocusOn

after idle "restoreFocus $focusWin"

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
# transfer changes to args handling to safe_getProperty
proc getProperty { property args } {
    global ay ay_error

    set report true
    set return_result true
    set obey_rptags true

    foreach arg $args {
	if { $arg == "-s" } {
	    set report false
	} else {
	    if { $arg == "-i" } {
		set obey_rptags false
	    } else {
		set return_result false
		upvar $arg result
	    }
	}
    }

    set result ""

    # decode arrayname from property argument
    set arrayname [string range $property 0 \
		       [expr [string first "(" $property]-1]]

    set propname [string range $property \
		       [expr [string first "(" $property]+1] \
		       [expr [string first ")" $property]-1]]

    getType types
    if { $types == "" } {
	if { $report } {
	    ayError 2 "getProperty" "Could not get type of object."
	}
	if { $return_result } {
	    return $result;
	} else {
	    return 0;
	}
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
	    if { [hasTag "NP"] } {
		set tn ""
		getTags tn tv
		set i 0
		foreach tag $tn {
		    if { $tag == "NP" } {
			lappend props [lindex $tv $i]
		    }
		    incr i
		}
		# foreach
	    }
	    # if

	    if { $obey_rptags && [hasTag "RP"] } {
		set tn ""
		getTags tn tv
		set i 0
		foreach tag $tn {
		    if { $tag == "RP" } {
			lremove props [lindex $tv $i]
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
		if { $return_result } {
		    return $result;
		} else {
		    return 0;
		}
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
	    eval [subst "lappend result \{\$$pvarname\}"]
	}
	incr obj
    }
    # foreach object

    if { $return_result } {
	return $result;
    } else {
	return 1;
    }
}
# getProperty


# setProperty:
#
#
proc setProperty { property newValue } {
    global ay ay_error

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
	    if { ($tn != "") && ([string first NP $tn] != -1) } {
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
    # foreach object

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
