# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2001 by Randolf Schultz
# (rschultz@informatik.uni-rostock.de) and others.
#
# All rights reserved.
#
# See the file License for details.

# sweep.tcl - sweep objects Tcl code

set Sweep_props { Transformations Attributes Material Tags SweepAttr }

proc sweep_getAttr { } {
global ay SweepAttr SweepAttrData BevelTags
# create SweepAttr-UI

catch {destroy $ay(pca).$SweepAttr(w)}
set w [frame $ay(pca).$SweepAttr(w)]
getProp

set tagnames ""
set tagvalues ""
getTags tagnames tagvalues
bevel_parseTags $tagnames $tagvalues

addMenu $w SweepAttrData Type [list Open Closed Periodic]
addCheck $w SweepAttrData Rotate
addCheck $w SweepAttrData Interpolate
addParam $w SweepAttrData Sections
addCheck $w SweepAttrData StartCap
addCheck $w SweepAttrData EndCap

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

addParam $w SweepAttrData Tolerance
addMenu $w SweepAttrData DisplayMode $ay(npdisplaymodes)

$ay(pca) itemconfigure 1 -window $w
update
plb_resize
# adapt scrollregion
set width [expr [winfo reqwidth $w] + 10]
set height [expr [winfo reqheight $w] + 10]
$ay(pca) configure -scrollregion [list 0 5 $width $height]

return;
}
# sweep_getAttr


proc sweep_setAttr { } {

    bevel_setTags
    setProp

 return;
}
# sweep_setAttr

array set SweepAttr {
arr   SweepAttrData
sproc sweep_setAttr
gproc sweep_getAttr
w     fSweepAttr

}

array set SweepAttrData {
Type 0
DisplayMode 1
}



#sweep_crt:
#
#
proc sweep_crt { } {
    global ay ay_error selected
    set selected ""
    getSel selected
    if { $selected == "" } { return; }
    set ay_error 0
    crtOb Sweep
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
# sweep_crt