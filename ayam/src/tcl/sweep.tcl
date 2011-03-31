# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2009 by Randolf Schultz
# (randolf.schultz@gmail.com) and others.
#
# All rights reserved.
#
# See the file License for details.

# sweep.tcl - sweep objects Tcl code

set Sweep_props { Transformations Attributes Material Tags SweepAttr }


# sweep_getAttr:
#  get Attributes from C context and build new PropertyGUI
#
proc sweep_getAttr { } {
    global ay SweepAttr SweepAttrData BevelTags

    set oldfocus [focus]

    # remove old, create new SweepAttr-UI
    catch {destroy $ay(pca).$SweepAttr(w)}
    set w [frame $ay(pca).$SweepAttr(w)]
    getProp

    set tagnames ""
    set tagvalues ""
    getTags tagnames tagvalues
    bevel_parseTags $tagnames $tagvalues

    set ay(bok) $ay(appb)

    addMenu $w SweepAttrData Type [list Open Closed Periodic]
    addCheck $w SweepAttrData Rotate
    addCheck $w SweepAttrData Interpolate
    addParam $w SweepAttrData Sections
    addCheck $w SweepAttrData StartCap
    addCheck $w SweepAttrData EndCap

    if { $BevelTags(HasStartBevel) } {
	addCommand $w c1 "Remove Start Bevel!" "bevel_rem 0 SweepAttrData"
	addMenu $w BevelTags SBType $ay(bevelmodes)
	addParam $w BevelTags SBRadius
	addCheck $w BevelTags SBRevert
    } else {
	addCommand $w c1 "Add Start Bevel!" "bevel_add 0 SweepAttrData"
    }

    if { $BevelTags(HasEndBevel) } {
	addCommand $w c2 "Remove End Bevel!" "bevel_rem 1 SweepAttrData"
	addMenu $w BevelTags EBType $ay(bevelmodes)
	addParam $w BevelTags EBRadius
	addCheck $w BevelTags EBRevert
    } else {
	addCommand $w c2 "Add End Bevel!" "bevel_add 1 SweepAttrData"
    }

    addParam $w SweepAttrData Tolerance
    addMenu $w SweepAttrData DisplayMode $ay(npdisplaymodes)

    addText $w SweepAttrData "Created NURBS Patch:"
    addInfo $w SweepAttrData NPInfo

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
# sweep_getAttr


proc sweep_setAttr { } {

    bevel_setTags
    setProp

 return;
}
# sweep_setAttr

array set SweepAttr {
arr   SweepAttrData
sproc sweep_setAttr
gproc sweep_getAttr
w     fSweepAttr

}

array set SweepAttrData {
Type 0
DisplayMode 1
NPInfoBall "n/a"
}
