# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2001 by Randolf Schultz
# (rschultz@informatik.uni-rostock.de) and others.
#
# All rights reserved.
#
# See the file License for details.

# skin.tcl - interpolating curves objects Tcl code

set Skin_props { Transformations Attributes Material Tags SkinAttr }


array set SkinAttr {
arr   SkinAttrData
sproc ""
gproc ""
w     fSkinAttr

}

array set SkinAttrData {
DisplayMode 1
Knot-Type_U 1
}
# create SkinAttr-UI
set w [frame $ay(pca).$SkinAttr(w)]



addCheck $w SkinAttrData Interpolate
addParam $w SkinAttrData Order_U
addMenu $w SkinAttrData Knot-Type_U [list Bezier B-Spline NURB]
addCheck $w SkinAttrData StartCap
addCheck $w SkinAttrData EndCap
global ay
set l $ay(npdisplaymodes)
set l [linsert $l 0 Global]
addMenu $w SkinAttrData DisplayMode $l
addParam $w SkinAttrData Tolerance


#skin_crt:
#
#
proc skin_crt { } {
    global ay ay_error selected
    set selected ""
    getSel selected
    if { $selected == "" } { return; }
    set ay_error 0
    crtOb Skin
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
# skin_crt
