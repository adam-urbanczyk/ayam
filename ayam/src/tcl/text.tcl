# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2009 by Randolf Schultz
# (randolf.schultz@gmail.com) and others.
#
# All rights reserved.
#
# See the file License for details.

# text.tcl - text objects Tcl code

set Text_props { Transformations Attributes Material Tags TextAttr }

# text_getAttr:
#  get Attributes from C context and build new PropertyGUI
#
proc text_getAttr { } {
    global ay TextAttr TextAttrData BevelTags

    set oldfocus [focus]

    # remove old, create new TextAttr-UI
    catch {destroy $ay(pca).$TextAttr(w)}
    set w [frame $ay(pca).$TextAttr(w)]
    getProp

    set tagnames ""
    set tagvalues ""
    getTags tagnames tagvalues
    bevel_parseTags $tagnames $tagvalues

    set ay(bok) $ay(appb)
    addVSpace $w s1 2
    addFile $w TextAttrData FontName
    addString $w TextAttrData String
    addParam $w TextAttrData Height
    addCheck $w TextAttrData Revert
    addCheck $w TextAttrData UpperCap
    addCheck $w TextAttrData LowerCap

    if { $BevelTags(HasStartBevel) } {
	addCommand $w c1 "Remove Start Bevel!" "bevel_rem 0 TextAttrData"
	addMenu $w BevelTags SBType $ay(bevelmodes)
	addParam $w BevelTags SBRadius
	addCheck $w BevelTags SBRevert
    } else {
	addCommand $w c1 "Add Start Bevel!" "bevel_add 0 TextAttrData"
    }

    if { $BevelTags(HasEndBevel) } {
	addCommand $w c2 "Remove End Bevel!" "bevel_rem 1 TextAttrData"
	addMenu $w BevelTags EBType $ay(bevelmodes)
	addParam $w BevelTags EBRadius
	addCheck $w BevelTags EBRevert
    } else {
	addCommand $w c2 "Add End Bevel!" "bevel_add 1 TextAttrData"
    }

    addParam $w TextAttrData Tolerance
    addMenu $w TextAttrData DisplayMode $ay(npdisplaymodes)

    # add UI to property canvas
    plb_setwin $w $oldfocus

 return;
}
# text_getAttr


# text_setAttr:
#  set attributes from Tcl to C context
#
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


