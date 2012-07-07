# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2009 by Randolf Schultz
# (randolf.schultz@gmail.com) and others.
#
# All rights reserved.
#
# See the file License for details.

# extrude.tcl - Extrude objects Tcl code

set Extrude_props { Transformations Attributes Material Tags ExtrudeAttr }


# extrude_getAttr:
#  get Attributes from C context and build new PropertyGUI
#
proc extrude_getAttr { } {
    global ay ExtrudeAttr ExtrudeAttrData BevelTags

    set oldfocus [focus]

    # remove old, create new ExtrudeAttr-UI
    catch {destroy $ay(pca).$ExtrudeAttr(w)}
    set w [frame $ay(pca).$ExtrudeAttr(w)]
    getProp

    set tagnames ""
    set tagvalues ""
    getTags tagnames tagvalues
    bevel_parseTags $tagnames $tagvalues

    set ay(bok) $ay(appb)

    addVSpace $w s1 2
    addParam $w ExtrudeAttrData Height
    addCheck $w ExtrudeAttrData StartCap
    addCheck $w ExtrudeAttrData EndCap

    if { $BevelTags(HasStartBevel) } {
	addCommand $w c1 "Remove Start Bevel!" "bevel_rem 0 ExtrudeAttrData"
	addMenu $w BevelTags SBType $ay(bevelmodes)
	addParam $w BevelTags SBRadius
	addCheck $w BevelTags SBRevert
    } else {
	addCommand $w c1 "Add Start Bevel!" "bevel_add 0 ExtrudeAttrData"
    }

    if { $BevelTags(HasEndBevel) } {
	addCommand $w c2 "Remove End Bevel!" "bevel_rem 1 ExtrudeAttrData"
	addMenu $w BevelTags EBType $ay(bevelmodes)
	addParam $w BevelTags EBRadius
	addCheck $w BevelTags EBRevert
    } else {
	addCommand $w c2 "Add End Bevel!" "bevel_add 1 ExtrudeAttrData"
    }

    addParam $w ExtrudeAttrData Tolerance
    addMenu $w ExtrudeAttrData DisplayMode $ay(npdisplaymodes)

    # add UI to property canvas
    plb_setwin $w $oldfocus

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
