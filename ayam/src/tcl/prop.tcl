# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2001 by Randolf Schultz
# (rschultz@informatik.uni-rostock.de) and others.
#
# All rights reserved.
#
# See the file License for details.

# prop.tcl - handle standard properties

array set Transformations {
arr   transfPropData
sproc setTrafo
gproc getTrafo
w     fTrafoAttr

}
# create Transformations-UI
set w [frame $ay(pca).$Transformations(w)]
addCommand $w c1 "Reset All!" {
global transfPropData
undo save
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
setTrafo
rV
}

addText $w e1 "Translation:"
addParam $w transfPropData Translate_X
addParam $w transfPropData Translate_Y
addParam $w transfPropData Translate_Z

addText $w e2 "Rotation:"
addParam $w transfPropData Rotate_X
addParam $w transfPropData Rotate_Y
addParam $w transfPropData Rotate_Z

addText $w e3 "Scale:"
addParam $w transfPropData Scale_X
addParam $w transfPropData Scale_Y
addParam $w transfPropData Scale_Z

addText $w e4 "Quaternion:"
addParam $w transfPropData Quat0
addParam $w transfPropData Quat1
addParam $w transfPropData Quat2
addParam $w transfPropData Quat3

#
#
#

proc setAttrp { } {
    global attrPropData
    regsub -all " " $attrPropData(Objectname) "_" attrPropData(Objectname)
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
addString $w attrPropData Objectname
addCheck $w attrPropData Hide
addInfo $w attrPropData RefCount

#
#
#


array set Material {
arr   matPropData
sproc setMat
gproc getMat
w     fMatAttr

}
# create Material-UI
set w [frame $ay(pca).$Material(w)]
addCommand $w c1 "Clear Material" \
"global matPropData; set matPropData(Name) \"\"; setMat"
addString $w matPropData Name


#
#
#

# getTagsp:
#
proc getTagsp { } {
global ay tagsPropData Tags

getTags names values
set tagsPropData(names) $names
set tagsPropData(values) $values

# create Tags-UI
catch {destroy $ay(pca).$Tags(w)}
set w [frame $ay(pca).$Tags(w)]
addCommand $w c1 "Remove all Tags!" {delTags all;plb_update}
addCommand $w c2 "Add Tag!" {addTagp}
set i 0
foreach tag $names {
    set val [lindex $values $i]
    button $w.b$i -text "$tag: $val" -command "addTagp $i" -bd 1 -pady 0
    pack $w.b$i -fill x -expand yes
    incr i
}
$ay(pca) itemconfigure 1 -window $w
plb_resize
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

}
# setTagsp

proc editTagshelper { index } {
 global tagsPropData
 set tagsPropData(names) [lreplace $tagsPropData(names) $index $index [.addTag.fu.e get]]
 set tagsPropData(values) [lreplace $tagsPropData(values) $index $index [.addTag.fm.e get]]
 grab release .addTag
 destroy .addTag
 setTagsp
 plb_update
}

# addTagp:
# also used to edit tags
proc addTagp { {edit -1} } {
global ay tagsPropData Tags

set w .addTag
catch {destroy $w}
toplevel $w

if { $edit >= 0 } {
    wm title $w "Edit Tag"
} else {
    wm title $w "Add Tag"
}

wm iconname $w "Ayam"


set f [frame $w.fu]
label $f.lt -text "Type:"
entry $f.e -width 30
pack $f.lt $f.e -in $f -padx 2 -pady 2 -side left
pack $f -in $w -side top

if { $edit >= 0 } {
    $f.e insert 0 [lindex $tagsPropData(names) $edit]
}

set f [frame $w.fm]
label $f.lv -text "Value:"
entry $f.e -width 30
pack $f.lv $f.e -in $f -padx 2 -pady 2 -side left
pack $f -in $w -side top

if { $edit >= 0 } {
    $f.e insert 0 [lindex $tagsPropData(values) $edit]
}

set f [frame $w.fd]
button $f.bok -text "Ok" -pady 0 -width 5 -command {
    global ay
    if { [.addTag.fu.e get] != "" } {
	addTag [.addTag.fu.e get] [.addTag.fm.e get]
    }
    grab release .addTag
    destroy .addTag
    plb_update
}

button $f.bclr -text "Clear" -pady 0 -width 5 -command {
    global ay

    .addTag.fu.e delete 0 end
    .addTag.fm.e delete 0 end
}

if { $edit >= 0 } {
$f.bok configure -command "editTagshelper $edit"
}

button $f.bca -text "Cancel" -pady 0 -width 5 -command "grab release .addTag; destroy $w"

pack $f.bok $f.bclr $f.bca -in $f -side left -fill x -expand yes
pack $f -in $w -side top -fill x

winCenter $w
grab $w
focus .addTag.fu.e
tkwait window $w

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
