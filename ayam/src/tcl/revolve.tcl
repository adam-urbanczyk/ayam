# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2001 by Randolf Schultz
# (randolf.schultz@gmail.com) and others.
#
# All rights reserved.
#
# See the file License for details.

# revolve.tcl - Revolve objects Tcl code

proc init_Revolve { } {
    global ay Revolve_props RevolveAttr RevolveAttrData

    set Revolve_props [list Transformations Attributes Material Tags Bevels\
			   Caps	RevolveAttr]

    array set RevolveAttr {
	arr   RevolveAttrData
	sproc ""
	gproc ""
	w     fRevolveAttr
    }

    array set RevolveAttrData {
	DisplayMode 1
	NPInfoBall "n/a"
	BoundaryNames { "Start" "End" "Upper" "Lower" }
	BoundaryIDs { 0 1 2 3 }
    }

    # create RevolveAttr-UI
    set w [frame $ay(pca).$RevolveAttr(w)]
    set a $RevolveAttr(arr)
    addVSpace $w s1 2
    addParam $w $a ThetaMax
    addParam $w $a Sections
    addParam $w $a Order

    addParam $w $a Tolerance
    addMenu $w $a DisplayMode $ay(npdisplaymodes)

    addText $w $a "Created NURBS Patch:"
    addInfo $w $a NPInfo

 return;
}
# init_Revolve
