# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2009 by Randolf Schultz
# (randolf.schultz@gmail.com) and others.
#
# All rights reserved.
#
# See the file License for details.

# birail2.tcl - Birail2 objects Tcl code

# birail2_getAttr:
#  get Attributes from C context and build new PropertyGUI
#
proc birail2_getAttr { } {
    global ay Birail2Attr Birail2AttrData BevelTags

    set oldfocus [focus]

    # remove old, create new Birail2Attr-UI
    catch {destroy $ay(pca).$Birail2Attr(w)}
    set w [frame $ay(pca).$Birail2Attr(w)]
    getProp

    set ay(bok) $ay(appb)

    #addCheck $w Birail2AttrData Close
    addParam $w Birail2AttrData Sections
    addCheck $w Birail2AttrData InterpolCtrl

    addParam $w Birail2AttrData Tolerance
    addMenu $w Birail2AttrData DisplayMode $ay(npdisplaymodes)

    addText $w Birail2AttrData "Created NURBS Patch:"
    addInfo $w Birail2AttrData NPInfo

    # add UI to property canvas
    plb_setwin $w $oldfocus

 return;
}
# birail2_getAttr

set Birail2 1

proc init_Birail2 { } {
global ay Birail2_props Birail2Attr Birail2AttrData

set Birail2_props { Transformations Attributes Material Tags Caps Bevels Birail2Attr }


array set Birail2Attr {
arr   Birail2AttrData
sproc ""
gproc birail2_getAttr
w     fBirail2Attr
}

array set Birail2AttrData {
DisplayMode 1
NPInfoBall "n/a"
BoundaryNames {"Start" "End"}
StartCap 0
EndCap 0
}
return;
}
# init_Birail2
