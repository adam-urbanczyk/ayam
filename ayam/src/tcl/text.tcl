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
    bevel_parseTags $tagnames $tagvalues $TextAttrData(BoundaryNames)

    set ay(bok) $ay(appb)
    addVSpace $w s1 2
    addFile $w TextAttrData FontName
    addString $w TextAttrData String
    addParam $w TextAttrData Height
    addCheck $w TextAttrData Revert
    addCheck $w TextAttrData UpperCap
    addCheck $w TextAttrData LowerCap

    if { $BevelTags(Bevel2) } {
	addCommand $w c1 "Remove Lower Bevel!"\
	"bevel_rem 2 TextAttrData {$TextAttrData(BoundaryIDs)}"
	addMenu $w BevelTags LowerType $ay(bevelmodes)
	addParam $w BevelTags LowerRadius
	addCheck $w BevelTags LowerRevert
    } else {
	addCommand $w c1 "Add Lower Bevel!"\
	    "bevel_add 2 TextAttrData {$TextAttrData(BoundaryIDs)}"
    }

    if { $BevelTags(Bevel3) } {
	addCommand $w c2 "Remove Upper Bevel!"\
	    "bevel_rem 3 TextAttrData {$TextAttrData(BoundaryIDs)}"
	addMenu $w BevelTags UpperType $ay(bevelmodes)
	addParam $w BevelTags UpperRadius
	addCheck $w BevelTags UpperRevert
    } else {
	addCommand $w c2 "Add Upper Bevel!"\
	    "bevel_add 3 TextAttrData {$TextAttrData(BoundaryIDs)}"
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

    bevel_setTags $::TextAttrData(BoundaryNames)
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
    BevelType 0
    BoundaryNames { "dummy" "dummy" "Lower" "Upper" }
    BoundaryIDs { 0 1 2 3 }
}
