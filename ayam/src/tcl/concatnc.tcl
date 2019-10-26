# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2001 by Randolf Schultz
# (randolf.schultz@gmail.com) and others.
#
# All rights reserved.
#
# See the file License for details.

# concatnc.tcl - concatnc objects Tcl code

proc init_ConcatNC { } {
    global ay ConcatNC_props ConcatNCAttr ConcatNCAttrData

    set ConcatNC_props { Transformations Attributes Tags ConcatNCAttr }

    array set ConcatNCAttr {
	arr   ConcatNCAttrData
	sproc ""
	gproc ""
	w     fConcatNCAttr
    }

    array set ConcatNCAttrData {
	Closed 0
	FillGaps 0
	Revert 0
	Length 0
	Order 0
	Knot-Type 1
	TanLength 3.0
	NCInfoBall "N/A"
    }

    # create ConcatNCAttr-UI
    set w [frame $ay(pca).$ConcatNCAttr(w)]
    set a $ConcatNCAttr(arr)
    addVSpace $w s1 2
    addCheck $w $a Closed
    addCheck $w $a FillGaps
    addCheck $w $a Revert
    addParam $w $a FTLength
    addMenu $w $a Knot-Type [list NURB Custom]
    addText $w e0 "Resulting Curve:"
    addInfo $w $a NCInfo

 return;
}
# init_ConcatNC
