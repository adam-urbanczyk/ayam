# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2001 by Randolf Schultz
# (rschultz@informatik.uni-rostock.de) and others.
#
# All rights reserved.
#
# See the file License for details.

# clone.tcl - clone objects Tcl code

set Clone_props { Transformations Attributes Material Tags CloneAttr }


array set CloneAttr {
arr   CloneAttrData
sproc ""
gproc ""
w     fCloneAttr

}

array set CloneAttrData {
 Mirror 0
}

# create CloneAttr-UI
set w [frame $ay(pca).$CloneAttr(w)]

addParam $w CloneAttrData NumClones
addCheck $w CloneAttrData Rotate
addMenu $w CloneAttrData Mirror [list "No" "At YZ-Plane" "At XZ-Plane"\
	"At XY-Plane" ]
addText $w e1 "Translation:"
addParam $w CloneAttrData Translate_X
addParam $w CloneAttrData Translate_Y
addParam $w CloneAttrData Translate_Z

addText $w e2 "Rotation:"
addParam $w CloneAttrData Rotate_X
addParam $w CloneAttrData Rotate_Y
addParam $w CloneAttrData Rotate_Z

addText $w e3 "Scale:"
addParam $w CloneAttrData Scale_X
addParam $w CloneAttrData Scale_Y
addParam $w CloneAttrData Scale_Z

addText $w e4 "Quaternion:"
addParam $w CloneAttrData Quat0
addParam $w CloneAttrData Quat1
addParam $w CloneAttrData Quat2
addParam $w CloneAttrData Quat3


array set CloneAttrData {
    NumClones 1
}


#clone_crt:
#
#
proc clone_crt { } {
    global ay ay_error selected
    set selected ""
    getSel selected
    if { $selected == "" } { ayError 20 "clone_crt" ""; return; }
    set ay_error 0
    crtOb Clone
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
# clone_crt


#mirror_crt:
#
#
proc mirror_crt { } {
    global ay ay_error selected
    set selected ""
    getSel selected
    if { $selected == "" } { ayError 20 "mirror_crt" ""; return; }
    set ay_error 0
    crtOb Clone -mirror 1
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
# mirror_crt
