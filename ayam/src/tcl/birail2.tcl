# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2004 by Randolf Schultz
# (rschultz@informatik.uni-rostock.de) and others.
#
# All rights reserved.
#
# See the file License for details.

# birail2.tcl - Birail2 objects Tcl code

set Birail2_props { Transformations Attributes Material Tags Birail2Attr }

proc birail2_getAttr { } {
global ay Birail2Attr Birail2AttrData BevelTags

# create Birail2Attr-UI
catch {destroy $ay(pca).$Birail2Attr(w)}
set w [frame $ay(pca).$Birail2Attr(w)]
getProp

set tagnames ""
set tagvalues ""
getTags tagnames tagvalues
bevel_parseTags $tagnames $tagvalues

#addCheck $w Birail2AttrData Close
addParam $w Birail2AttrData Sections
addCheck $w Birail2AttrData InterpolCtrl
addCheck $w Birail2AttrData StartCap
addCheck $w Birail2AttrData EndCap

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

addParam $w Birail2AttrData Tolerance
addMenu $w Birail2AttrData DisplayMode $ay(npdisplaymodes)

$ay(pca) itemconfigure 1 -window $w
update
plb_resize
# adapt scrollregion
set width [expr [winfo reqwidth $w] + 10]
set height [expr [winfo reqheight $w] + 10]
$ay(pca) configure -scrollregion [list 0 5 $width $height]

return;
}
# birail2_getAttr


proc birail2_setAttr { } {

    bevel_setTags
    setProp

 return;
}
# birail2_setAttr


array set Birail2Attr {
arr   Birail2AttrData
sproc birail2_setAttr
gproc birail2_getAttr
w     fBirail2Attr

}

array set Birail2AttrData {
DisplayMode 1
}
# create Birail2Attr-UI
set w [frame $ay(pca).$Birail2Attr(w)]


#birail2_crt:
#
#
proc birail2_crt { } {
    global ay ay_error selected
    set selected ""
    getSel selected
    if { $selected == "" } { ayError 20 "birail2_crt" ""; return; }
    set ay_error 0
    crtOb Birail2
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
# birail2_crt