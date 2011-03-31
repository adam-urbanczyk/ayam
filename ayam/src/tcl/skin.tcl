# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2001 by Randolf Schultz
# (randolf.schultz@gmail.com) and others.
#
# All rights reserved.
#
# See the file License for details.

# skin.tcl - Skin objects Tcl code

set Skin_props { Transformations Attributes Material Tags SkinAttr }


# skin_getAttr:
#  get Attributes from C context and build new PropertyGUI
#
proc skin_getAttr { } {
    global ay SkinAttr SkinAttrData BevelTags

    set oldfocus [focus]

    # remove old, create new SkinAttr-UI
    catch {destroy $ay(pca).$SkinAttr(w)}
    set w [frame $ay(pca).$SkinAttr(w)]
    getProp

    set tagnames ""
    set tagvalues ""
    getTags tagnames tagvalues
    bevel_parseTags $tagnames $tagvalues

    set ay(bok) $ay(appb)

    addCheck $w SkinAttrData Interpolate
    addParam $w SkinAttrData Order_U
    addMenu $w SkinAttrData Knot-Type_U [list Bezier B-Spline NURB Custom]
    addCheck $w SkinAttrData StartCap
    addCheck $w SkinAttrData EndCap

    if { $BevelTags(HasStartBevel) } {
	addCommand $w c1 "Remove Start Bevel!" "bevel_rem 0 SkinAttrData"
	addMenu $w BevelTags SBType $ay(bevelmodes)
	addParam $w BevelTags SBRadius
	addCheck $w BevelTags SBRevert
    } else {
	addCommand $w c1 "Add Start Bevel!" "bevel_add 0 SkinAttrData"
    }

    if { $BevelTags(HasEndBevel) } {
	addCommand $w c2 "Remove End Bevel!" "bevel_rem 1 SkinAttrData"
	addMenu $w BevelTags EBType $ay(bevelmodes)
	addParam $w BevelTags EBRadius
	addCheck $w BevelTags EBRevert
    } else {
	addCommand $w c2 "Add End Bevel!" "bevel_add 1 SkinAttrData"
    }

    addParam $w SkinAttrData Tolerance
    addMenu $w SkinAttrData DisplayMode $ay(npdisplaymodes)

    addText $w SkinAttrData "Created NURBS Patch:"
    addInfo $w SkinAttrData NPInfo

    # add UI to property canvas
    $ay(pca) itemconfigure 1 -window $w
    update
    plb_resize
    # adapt canvas scrollregion
    set width [expr [winfo reqwidth $w] + 10]
    set height [expr [winfo reqheight $w] + 10]
    $ay(pca) configure -scrollregion [list 0 5 $width $height]

    # restore focus
    if { [winfo exists $oldfocus] } {
	focus -force $oldfocus
    }

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
NPInfoBall "n/a"
}
