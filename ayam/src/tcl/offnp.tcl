# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2007 by Randolf Schultz
# (randolf.schultz@gmail.com) and others.
#
# All rights reserved.
#
# See the file License for details.

# offnp.tcl - offset surfaces objects Tcl code

proc init_OffsetNP { } {
    global ay OffsetNP_props OffsetNPAttr OffsetNPAttrData

    set OffsetNP_props { Attributes Material Tags Bevels Caps OffsetNPAttr }

    array set OffsetNPAttr {
	arr   OffsetNPAttrData
	sproc ""
	gproc ""
	w     fOffsetNPAttr
    }

    array set OffsetNPAttrData {
	Mode 1
	DisplayMode 0
	NPInfoBall "N/A"
	BoundaryNames { "U0" "U1" "V0" "V1" }
    }
    # create UI
    set w [frame $ay(pca).$OffsetNPAttr(w)]
    set a $OffsetNPAttr(arr)
    addVSpace $w s1 2
    addMenu $w $a Mode [list "Normal" "Section"]

    addParam $w $a Offset

    addParam $w $a Tolerance
    addMenu $w $a DisplayMode $ay(npdisplaymodes)

    addText $w $a "Created NURBS Patch:"
    addInfo $w $a NPInfo

 return;
}
# init_OffsetNP
