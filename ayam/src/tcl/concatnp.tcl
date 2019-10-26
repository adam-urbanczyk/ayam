# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2009 by Randolf Schultz
# (randolf.schultz@gmail.com) and others.
#
# All rights reserved.
#
# See the file License for details.

# concatnp.tcl - concatnp objects Tcl code

proc init_ConcatNP { } {
    global ay ConcatNP_props ConcatNPAttr ConcatNPAttrData

    set ConcatNP_props [list Transformations Attributes Tags Caps Bevels\
			    ConcatNPAttr]

    array set ConcatNPAttr {
	arr   ConcatNPAttrData
	sproc ""
	gproc ""
	w     fConcatNPAttr
    }

    array set ConcatNPAttrData {
	Type 0
	FillGaps 0
	Revert 0
	Length 0
	Order 0
	Knot-Type 1
	FTLength 1.0
	NPInfoBall "N/A"
	BoundaryNames { "U0" "U1" "V0" "V1" }
    }

    # create ConcatNPAttr-UI
    set w [frame $ay(pca).$ConcatNPAttr(w)]
    set a $ConcatNPAttr(arr)
    addVSpace $w s1 2
    addMenu $w $a Type [list Open Closed Periodic]
    addParam $w $a Order
    addCheck $w $a Revert
    addCheck $w $a FillGaps
    addParam $w $a FTLength
    addMenu $w $a Knot-Type\
	[list B-Spline NURB Chordal Centripetal Custom]
    addString $w $a UVSelect
    addCheck $w $a Compatible

    addText $w e0 "Resulting Patch:"
    addInfo $w $a NPInfo

 return;
}
# init_ConcatNP
