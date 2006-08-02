# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2001 by Randolf Schultz
# (rschultz@informatik.uni-rostock.de) and others.
#
# All rights reserved.
#
# See the file License for details.

# gordon.tcl - Gordon objects Tcl code

set Gordon_props { Transformations Attributes Material Tags GordonAttr }


proc gordon_getAttr { } {
global ay GordonAttr GordonAttrData BevelTags
# create GordonAttr-UI

catch {destroy $ay(pca).$GordonAttr(w)}
set w [frame $ay(pca).$GordonAttr(w)]
getProp

set tagnames ""
set tagvalues ""
getTags tagnames tagvalues
bevel_parseTags $tagnames $tagvalues

# create GordonAttr-UI
addCheck $w GordonAttrData WatchCurves
addParam $w GordonAttrData Order_U
addParam $w GordonAttrData Order_V

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

if { $BevelTags(HasLeftBevel) } {
    addCommand $w c3 "Remove Left Bevel!" "bevel_rem 2"
    addMenu $w BevelTags LBType $ay(bevelmodes)
    addParam $w BevelTags LBRadius
    addCheck $w BevelTags LBRevert
} else {
    addCommand $w c3 "Add Left Bevel!" "bevel_add 2"
}

if { $BevelTags(HasRightBevel) } {
    addCommand $w c4 "Remove Right Bevel!" "bevel_rem 3"
    addMenu $w BevelTags RBType $ay(bevelmodes)
    addParam $w BevelTags RBRadius
    addCheck $w BevelTags RBRevert
} else {
    addCommand $w c4 "Add Right Bevel!" "bevel_add 3"
}

addParam $w GordonAttrData Tolerance
addMenu $w GordonAttrData DisplayMode $ay(npdisplaymodes)

$ay(pca) itemconfigure 1 -window $w
update
plb_resize
# adapt scrollregion
set width [expr [winfo reqwidth $w] + 10]
set height [expr [winfo reqheight $w] + 10]
$ay(pca) configure -scrollregion [list 0 5 $width $height]

return;
}
# gordon_getAttr


proc gordon_setAttr { } {

    bevel_setTags
    setProp

 return;
}
# gordon_setAttr

array set GordonAttr {
    arr   GordonAttrData
    sproc gordon_setAttr
    gproc gordon_getAttr
    w     fGordonAttr
}

array set GordonAttrData {
DisplayMode 1
}


#gordon_crt:
#
#
proc gordon_crt { } {
    global ay ay_error selected
    set selected ""
    getSel selected
    if { $selected == "" } { ayError 20 "gordon_crt" ""; return; }

    # the next command sorts the selected objects
    eval "selOb $selected"

    set ay_error 0
    crtOb Gordon
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
# gordon_crt
