# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2004 by Randolf Schultz
# (rschultz@informatik.uni-rostock.de) and others.
#
# All rights reserved.
#
# See the file License for details.

# birail1.tcl - Birail1 objects Tcl code

set Birail1_props { Transformations Attributes Material Tags Birail1Attr }


proc birail1_getAttr { } {
global ay Birail1Attr Birail1AttrData BevelTags

# create Birail1Attr-UI
catch {destroy $ay(pca).$Birail1Attr(w)}
set w [frame $ay(pca).$Birail1Attr(w)]
getProp

set tagnames ""
set tagvalues ""
getTags tagnames tagvalues
bevel_parseTags $tagnames $tagvalues

#addCheck $w Birail1AttrData Close
addParam $w Birail1AttrData Sections
addCheck $w Birail1AttrData StartCap
addCheck $w Birail1AttrData EndCap

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

addParam $w Birail1AttrData Tolerance
addMenu $w Birail1AttrData DisplayMode $ay(npdisplaymodes)

addText $w Birail1AttrData "Created NURBS Patch:"
addInfo $w Birail1AttrData NPInfo

$ay(pca) itemconfigure 1 -window $w
update
plb_resize
# adapt scrollregion
set width [expr [winfo reqwidth $w] + 10]
set height [expr [winfo reqheight $w] + 10]
$ay(pca) configure -scrollregion [list 0 5 $width $height]

return;
}
# birail1_getAttr


proc birail1_setAttr { } {

    bevel_setTags
    setProp

 return;
}
# birail1_setAttr


array set Birail1Attr {
arr   Birail1AttrData
sproc birail1_setAttr
gproc birail1_getAttr
w     fBirail1Attr

}

array set Birail1AttrData {
DisplayMode 1
}

#birail1_crt:
#
#
proc birail1_crt { } {
    global ay ay_error selected
    set selected ""
    getSel selected
    if { $selected == "" } { ayError 20 "birail1_crt" ""; return; }
    set ay_error 0
    crtOb Birail1
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
    uS; sL; forceNot; rV;

 return;
}
# birail1_crt