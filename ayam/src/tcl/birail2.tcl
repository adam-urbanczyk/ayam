# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2009 by Randolf Schultz
# (randolf.schultz@gmail.com) and others.
#
# All rights reserved.
#
# See the file License for details.

# birail2.tcl - Birail2 objects Tcl code

proc init_Birail2 { } {
    global ay Birail2_props Birail2Attr Birail2AttrData

    set Birail2_props [list Transformations Attributes Material Tags Bevels\
			   Caps	Birail2Attr]

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
    set a $Birail2Attr(arr)
    addVSpace $w s1 2
    #addCheck $w $a Close
    addParam $w $a Sections
    addCheck $w $a InterpolCtrl

    addParam $w $a Tolerance
    addMenu $w $a DisplayMode $ay(npdisplaymodes)

    addText $w $a "Created NURBS Patch:"
    addInfo $w $a NPInfo

 return;
}
# init_Birail2
