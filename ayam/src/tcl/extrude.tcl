# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2001 by Randolf Schultz
# (rschultz@informatik.uni-rostock.de) and others.
#
# All rights reserved.
#
# See the file License for details.

# extrude.tcl - interpolating curves objects Tcl code

set Extrude_props { Transformations Attributes Material Tags ExtrudeAttr }


array set ExtrudeAttr {
arr   ExtrudeAttrData
sproc ""
gproc ""
w     fExtrudeAttr

}

array set ExtrudeAttrData {
DisplayMode 1
BevelType 0
}
# create ExtrudeAttr-UI
set w [frame $ay(pca).$ExtrudeAttr(w)]

addParam $w ExtrudeAttrData Height
addCheck $w ExtrudeAttrData UpperCap
addCheck $w ExtrudeAttrData LowerCap
addCheck $w ExtrudeAttrData UpperBevels
addCheck $w ExtrudeAttrData LowerBevels
addMenu $w ExtrudeAttrData BevelType [list Round Linear Ridge]
addParam $w ExtrudeAttrData BevelRadius
global ay
set l $ay(npdisplaymodes)
set l [linsert $l 0 Global]
addMenu $w ExtrudeAttrData DisplayMode $l
addParam $w ExtrudeAttrData Tolerance


#extrude_crt:
#
#
proc extrude_crt { } {
    global ay ay_error selected
    set selected ""
    getSel selected
    if { $selected == "" } { ayError 20 "extrude_crt" ""; return; }

    # the next command sorts the selected objects
    eval "selOb $selected"

    set ay_error 0
    crtOb Extrude
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
# extrude_crt