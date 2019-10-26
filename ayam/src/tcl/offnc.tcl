# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2007 by Randolf Schultz
# (randolf.schultz@gmail.com) and others.
#
# All rights reserved.
#
# See the file License for details.

# offnc.tcl - offset curves objects Tcl code

proc init_OffsetNC { } {
    global ay OffsetNC_props OffsetNCAttr OffsetNCAttrData

    set OffsetNC_props { Attributes Tags OffsetNCAttr }

    array set OffsetNCAttr {
	arr   OffsetNCAttrData
	sproc ""
	gproc ""
	w     fOffsetNCAttr
    }

    array set OffsetNCAttrData {
	Mode 1
	DisplayMode 0
	NCInfoBall "N/A"
    }

    # create UI
    set w [frame $ay(pca).$OffsetNCAttr(w)]
    set a $OffsetNCAttr(arr)
    addVSpace $w s1 2
    addMenu $w $a Mode [list "Point" "Section" "Hybrid" "3DPVN"]
    addCheck $w $a Revert
    addParam $w $a Offset

    addParam $w $a Tolerance
    addMenu $w $a DisplayMode $ay(ncdisplaymodes)

    addText $w $a "Created NURBS Curve:"
    addInfo $w $a NCInfo

 return;
}
# init_OffsetNC
