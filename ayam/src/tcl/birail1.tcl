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

    set ay(bok) $ay(appb)

    addMenu $w Birail1AttrData Type [list Open Closed Periodic]
    addParam $w Birail1AttrData Sections

    addParam $w Birail1AttrData Tolerance
    addMenu $w Birail1AttrData DisplayMode $ay(npdisplaymodes)

    addText $w Birail1AttrData "Created NURBS Patch:"
    addInfo $w Birail1AttrData NPInfo

    # add UI to property canvas
    plb_setwin $w $oldfocus

 return;
}
# birail1_getAttr


set Birail1 1

proc init_Birail1 { } {
global ay Birail1_props Birail1Attr Birail1AttrData

set Birail1_props { Transformations Attributes Material Tags Caps Bevels Birail1Attr }

array set Birail1Attr {
arr   Birail1AttrData
sproc ""
gproc birail1_getAttr
w     fBirail1Attr
}

array set Birail1AttrData {
Type 0
DisplayMode 1
NPInfoBall "n/a"
BoundaryNames { "Start" "End" "R1" "R2" }
StartCap 0
EndCap 0
R1Cap 0
R2Cap 0
}
return;
}
# init_Birail1
