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
    bevel_parseTags $tagnames $tagvalues $ExtrudeAttrData(BoundaryNames)

    set ay(bok) $ay(appb)

    addVSpace $w s1 2
    addParam $w ExtrudeAttrData Height
    addCheck $w ExtrudeAttrData StartCap
    addCheck $w ExtrudeAttrData EndCap

    if { $BevelTags(Bevel2) } {
	addCommand $w c1 "Remove Start Bevel!"\
	    "bevel_rem 2 ExtrudeAttrData \{$ExtrudeAttrData(BoundaryIDs)\}"
	addMenu $w BevelTags StartType $ay(bevelmodes)
	addParam $w BevelTags StartRadius
	addCheck $w BevelTags StartRevert
    } else {
	addCommand $w c1 "Add Start Bevel!"\
	    "bevel_add 2 ExtrudeAttrData \{$ExtrudeAttrData(BoundaryIDs)\}"
    }

    if { $BevelTags(Bevel3) } {
	addCommand $w c2 "Remove End Bevel!"\
	    "bevel_rem 3 ExtrudeAttrData \{$ExtrudeAttrData(BoundaryIDs)\}"
	addMenu $w BevelTags EndType $ay(bevelmodes)
	addParam $w BevelTags EndRadius
	addCheck $w BevelTags EndRevert
    } else {
	addCommand $w c2 "Add End Bevel!"\
	    "bevel_add 3 ExtrudeAttrData \{$ExtrudeAttrData(BoundaryIDs)\}"
    }

    addParam $w ExtrudeAttrData Tolerance
    addMenu $w ExtrudeAttrData DisplayMode $ay(npdisplaymodes)

    # add UI to property canvas
    plb_setwin $w $oldfocus

 return;
}
# extrude_getAttr


proc extrude_setAttr { } {

    bevel_setTags $::ExtrudeAttrData(BoundaryNames)
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
    BoundaryNames { "dummy" "dummy" "Start" "End" }
    BoundaryIDs { 0 1 2 3 }
}
