# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2009 by Randolf Schultz
# (randolf.schultz@gmail.com) and others.
#
# All rights reserved.
#
# See the file License for details.

# birail1.tcl - Birail1 objects Tcl code

proc init_Birail1 { } {
    global ay Birail1_props Birail1Attr Birail1AttrData

    set Birail1_props [list Transformations Attributes Material Tags Bevels\
			   Caps	Birail1Attr]

    array set Birail1Attr {
	arr   Birail1AttrData
	sproc ""
	gproc ""
	w     fBirail1Attr
    }

    array set Birail1AttrData {
	Type 0
	DisplayMode 1
	NPInfoBall "n/a"
	BoundaryNames { "Start" "End" "Left" "Right" }
	BoundaryIDs { 2 3 0 1 }
	StartCap 0
	EndCap 0
	R1Cap 0
	R2Cap 0
    }

    set w [frame $ay(pca).$Birail1Attr(w)]
    set a $Birail1Attr(arr)
    addVSpace $w s1 2
    addMenu $w $a Type [list Open Closed Periodic]
    addParam $w $a Sections

    addParam $w $a Tolerance
    addMenu $w $a DisplayMode $ay(npdisplaymodes)

    addText $w $a "Created NURBS Patch:"
    addInfo $w $a NPInfo

 return;
}
# init_Birail1
