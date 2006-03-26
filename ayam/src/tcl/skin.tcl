# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2001 by Randolf Schultz
# (rschultz@informatik.uni-rostock.de) and others.
#
# All rights reserved.
#
# See the file License for details.

# skin.tcl - Skin objects Tcl code

set Skin_props { Transformations Attributes Material Tags SkinAttr }


# skin_getAttr:
#
#
proc skin_getAttr { } {
global ay SkinAttr SkinAttrData BevelTags

# create SkinAttr-UI
catch {destroy $ay(pca).$SkinAttr(w)}
set w [frame $ay(pca).$SkinAttr(w)]
getProp

set tagnames ""
set tagvalues ""
getTags tagnames tagvalues
bevel_parseTags $tagnames $tagvalues

addCheck $w SkinAttrData Interpolate
addParam $w SkinAttrData Order_U
addMenu $w SkinAttrData Knot-Type_U [list Bezier B-Spline NURB Custom]
addCheck $w SkinAttrData StartCap
addCheck $w SkinAttrData EndCap

if { $BevelTags(HasStartBevel) } {
    addCommand $w c1 "Remove Start Bevel!" "bevel_rem 0"
    addMenu $w BevelTags SBType $ay(bevelmodes)
    addParam $w BevelTags SBRadius
    addCheck $w BevelTags SBRevert
} else {
    addCommand $w c1 "Add Start Bevel!" "bevel_add 0"
}

if { $BevelTags(HasEndBevel) } {
    addCommand $w c2 "Remove End Bevel!" "bevel_rem 1"
    addMenu $w BevelTags EBType $ay(bevelmodes)
    addParam $w BevelTags EBRadius
    addCheck $w BevelTags EBRevert
} else {
    addCommand $w c2 "Add End Bevel!" "bevel_add 1"
}

addParam $w SkinAttrData Tolerance
addMenu $w SkinAttrData DisplayMode $ay(npdisplaymodes)

$ay(pca) itemconfigure 1 -window $w
update
plb_resize
# adapt scrollregion
set width [expr [winfo reqwidth $w] + 10]
set height [expr [winfo reqheight $w] + 10]
$ay(pca) configure -scrollregion [list 0 5 $width $height]

return;
}
# skin_getAttr

# skin_setAttr:
#
#
proc skin_setAttr { } {

    bevel_setTags
    setProp

 return;
}
# skin_setAttr

array set SkinAttr {
arr   SkinAttrData
sproc skin_setAttr
gproc skin_getAttr
w     fSkinAttr

}

array set SkinAttrData {
Interpolate 0
DisplayMode 1
Knot-Type_U 1
}


# skin_crt:
#
#
proc skin_crt { } {
    global ay ay_error selected
    set selected ""
    getSel selected
    if { $selected == "" } { ayError 20 "skin_crt" ""; return; }
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
