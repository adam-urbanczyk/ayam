# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2009 by Randolf Schultz
# (randolf.schultz@gmail.com) and others.
#
# All rights reserved.
#
# See the file License for details.

# gordon.tcl - Gordon objects Tcl code

set Gordon_props { Transformations Attributes Material Tags GordonAttr }


# gordon_getAttr:
#  get Attributes from C context and build new PropertyGUI
#
proc gordon_getAttr { } {
    global ay GordonAttr GordonAttrData BevelTags

    set oldfocus [focus]

    # remove old, create new GordonAttr-UI
    catch {destroy $ay(pca).$GordonAttr(w)}
    set w [frame $ay(pca).$GordonAttr(w)]
    getProp

    set tagnames ""
    set tagvalues ""
    getTags tagnames tagvalues
    bevel_parseTags $tagnames $tagvalues

    set ay(bok) $ay(appb)

    addVSpace $w s1 2
    addCheck $w GordonAttrData WatchCurves
    addParam $w GordonAttrData Order_U
    addParam $w GordonAttrData Order_V

    if { $BevelTags(HasStartBevel) } {
	addCommand $w c1 "Remove Start Bevel!" "bevel_rem 0 GordonAttrData"
	addMenu $w BevelTags SBType $ay(bevelmodes)
	addParam $w BevelTags SBRadius
	addCheck $w BevelTags SBRevert
    } else {
	addCommand $w c1 "Add Start Bevel!" "bevel_add 0 GordonAttrData"
    }

    if { $BevelTags(HasEndBevel) } {
	addCommand $w c2 "Remove End Bevel!" "bevel_rem 1 GordonAttrData"
	addMenu $w BevelTags EBType $ay(bevelmodes)
	addParam $w BevelTags EBRadius
	addCheck $w BevelTags EBRevert
    } else {
	addCommand $w c2 "Add End Bevel!" "bevel_add 1 GordonAttrData"
    }

    if { $BevelTags(HasLeftBevel) } {
	addCommand $w c3 "Remove Left Bevel!" "bevel_rem 2 GordonAttrData"
	addMenu $w BevelTags LBType $ay(bevelmodes)
	addParam $w BevelTags LBRadius
	addCheck $w BevelTags LBRevert
    } else {
	addCommand $w c3 "Add Left Bevel!" "bevel_add 2 GordonAttrData"
    }

    if { $BevelTags(HasRightBevel) } {
	addCommand $w c4 "Remove Right Bevel!" "bevel_rem 3 GordonAttrData"
	addMenu $w BevelTags RBType $ay(bevelmodes)
	addParam $w BevelTags RBRadius
	addCheck $w BevelTags RBRevert
    } else {
	addCommand $w c4 "Add Right Bevel!" "bevel_add 3 GordonAttrData"
    }

    addParam $w GordonAttrData Tolerance
    addMenu $w GordonAttrData DisplayMode $ay(npdisplaymodes)

    addText $w GordonAttrData "Created NURBS Patch:"
    addInfo $w GordonAttrData NPInfo

    # add UI to property canvas
    plb_setwin $w $oldfocus

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
NPInfoBall "n/a"
}
