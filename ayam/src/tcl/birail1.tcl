# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2009 by Randolf Schultz
# (randolf.schultz@gmail.com) and others.
#
# All rights reserved.
#
# See the file License for details.

# birail1.tcl - Birail1 objects Tcl code

# birail1_getAttr:
#  get Attributes from C context and build new PropertyGUI
#
proc birail1_getAttr { } {
    global ay Birail1Attr Birail1AttrData BevelTags

    set oldfocus [focus]

    # remove old, create new Birail1Attr-UI
    catch {destroy $ay(pca).$Birail1Attr(w)}
    set w [frame $ay(pca).$Birail1Attr(w)]
    getProp

    set tagnames ""
    set tagvalues ""
    getTags tagnames tagvalues
    bevel_parseTags $tagnames $tagvalues

    set ay(bok) $ay(appb)

    addMenu $w Birail1AttrData Type [list Open Closed Periodic]
    addParam $w Birail1AttrData Sections
    addCheck $w Birail1AttrData StartCap
    addCheck $w Birail1AttrData EndCap

    if { $BevelTags(HasStartBevel) } {
	addCommand $w c1 "Remove Start Bevel!" "bevel_rem 0 Birail1AttrData"
	addMenu $w BevelTags SBType $ay(bevelmodes)
	addParam $w BevelTags SBRadius
	addCheck $w BevelTags SBRevert
    } else {
	addCommand $w c1 "Add Start Bevel!" "bevel_add 0 Birail1AttrData"
    }

    if { $BevelTags(HasEndBevel) } {
	addCommand $w c2 "Remove End Bevel!" "bevel_rem 1 Birail1AttrData"
	addMenu $w BevelTags EBType $ay(bevelmodes)
	addParam $w BevelTags EBRadius
	addCheck $w BevelTags EBRevert
    } else {
	addCommand $w c2 "Add End Bevel!" "bevel_add 1 Birail1AttrData"
    }

    addParam $w Birail1AttrData Tolerance
    addMenu $w Birail1AttrData DisplayMode $ay(npdisplaymodes)

    addText $w Birail1AttrData "Created NURBS Patch:"
    addInfo $w Birail1AttrData NPInfo

    # add UI to property canvas
    plb_setwin $w $oldfocus

 return;
}
# birail1_getAttr


proc birail1_setAttr { } {

    bevel_setTags
    setProp

 return;
}
# birail1_setAttr

set Birail1 1

proc init_Birail1 { } {
global ay Birail1_props Birail1Attr Birail1AttrData

set Birail1_props { Transformations Attributes Material Tags Birail1Attr }

array set Birail1Attr {
arr   Birail1AttrData
sproc birail1_setAttr
gproc birail1_getAttr
w     fBirail1Attr

}

array set Birail1AttrData {
Type 0
DisplayMode 1
NPInfoBall "n/a"
}
return;
}
# init_Birail1
