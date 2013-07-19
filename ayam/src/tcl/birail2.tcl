# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2009 by Randolf Schultz
# (randolf.schultz@gmail.com) and others.
#
# All rights reserved.
#
# See the file License for details.

# birail2.tcl - Birail2 objects Tcl code

set Birail2 1

proc init_Birail2 { } {
global ay Birail2_props Birail2Attr Birail2AttrData

set Birail2_props { Transformations Attributes Material Tags Bevels Caps Birail2Attr }


array set Birail2Attr {
arr   Birail2AttrData
sproc ""
gproc ""
w     fBirail2Attr
}

array set Birail2AttrData {
DisplayMode 1
NPInfoBall "n/a"
BoundaryNames { "Start" "End" "Left" "Right" }
BoundaryIDs { 2 3 0 1 }
StartCap 0
EndCap 0
R1Cap 0
R2Cap 0
}

set w [frame $ay(pca).$Birail2Attr(w)]
addVSpace $w s1 2
#addCheck $w Birail2AttrData Close
addParam $w Birail2AttrData Sections
addCheck $w Birail2AttrData InterpolCtrl

addParam $w Birail2AttrData Tolerance
addMenu $w Birail2AttrData DisplayMode $ay(npdisplaymodes)

addText $w Birail2AttrData "Created NURBS Patch:"
addInfo $w Birail2AttrData NPInfo

return;
}
# init_Birail2
