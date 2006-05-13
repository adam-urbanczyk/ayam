# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2001 by Randolf Schultz
# (rschultz@informatik.uni-rostock.de) and others.
#
# All rights reserved.
#
# See the file License for details.

# extrude.tcl - Extrude objects Tcl code

set Extrude_props { Transformations Attributes Material Tags ExtrudeAttr }


proc extrude_getAttr { } {
global ay ExtrudeAttr ExtrudeAttrData BevelTags
# create ExtrudeAttr-UI

catch {destroy $ay(pca).$ExtrudeAttr(w)}
set w [frame $ay(pca).$ExtrudeAttr(w)]
getProp

set tagnames ""
set tagvalues ""
getTags tagnames tagvalues
bevel_parseTags $tagnames $tagvalues

addParam $w ExtrudeAttrData Height
addCheck $w ExtrudeAttrData StartCap
addCheck $w ExtrudeAttrData EndCap

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

addParam $w ExtrudeAttrData Tolerance
addMenu $w ExtrudeAttrData DisplayMode $ay(npdisplaymodes)

$ay(pca) itemconfigure 1 -window $w
update
plb_resize
# adapt scrollregion
set width [expr [winfo reqwidth $w] + 10]
set height [expr [winfo reqheight $w] + 10]
$ay(pca) configure -scrollregion [list 0 5 $width $height]

return;
}
# extrude_getAttr


proc extrude_setAttr { } {

    bevel_setTags
    setProp

 return;
}
# extrude_setAttr


array set ExtrudeAttr {
    arr   ExtrudeAttrData
    sproc extrude_setAttr
    gproc extrude_getAttr
    w     fExtrudeAttr
}

array set ExtrudeAttrData {
    DisplayMode 1
    BevelType 0
}


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
    if { $ay_error } { return; }

    cutOb
    set ay(ul) $ay(CurrentLevel)
    uS
    sL
    getLevel a b
    goDown [expr [llength $a]-1]
    cmovOb
    goUp
    set ay(ul) $ay(CurrentLevel)
    uS; sL; forceNot; rV

 return;
}
# extrude_crt