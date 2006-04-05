# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2004 by Randolf Schultz
# (rschultz@informatik.uni-rostock.de) and others.
#
# All rights reserved.
#
# See the file License for details.

# text.tcl - text objects Tcl code

set Text_props { Transformations Attributes Material Tags TextAttr }

proc text_getAttr { } {
global ay TextAttr TextAttrData BevelTags
# create TextAttr-UI

catch {destroy $ay(pca).$TextAttr(w)}
set w [frame $ay(pca).$TextAttr(w)]
getProp

set tagnames ""
set tagvalues ""
getTags tagnames tagvalues
bevel_parseTags $tagnames $tagvalues

addFile $w TextAttrData FontName
addString $w TextAttrData String
addParam $w TextAttrData Height
addCheck $w TextAttrData Revert
addCheck $w TextAttrData UpperCap
addCheck $w TextAttrData LowerCap

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

addParam $w TextAttrData Tolerance
addMenu $w TextAttrData DisplayMode $ay(npdisplaymodes)

$ay(pca) itemconfigure 1 -window $w
update
plb_resize
# adapt scrollregion
set width [expr [winfo reqwidth $w] + 10]
set height [expr [winfo reqheight $w] + 10]
$ay(pca) configure -scrollregion [list 0 5 $width $height]

return;
}
# text_getAttr


proc text_setAttr { } {

    bevel_setTags
    setProp

 return;
}
# text_setAttr


array set TextAttr {
    arr   TextAttrData
    sproc text_setAttr
    gproc text_getAttr
    w     fTextAttr
}

array set TextAttrData {
    DisplayMode 1
}
# create TextAttr-UI
set w [frame $ay(pca).$TextAttr(w)]

