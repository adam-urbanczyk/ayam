# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2004 by Randolf Schultz
# (randolf.schultz@gmail.com) and others.
#
# All rights reserved.
#
# See the file License for details.

# ncircle.tcl - NCircle objects Tcl code

proc init_NCircle { } {
    global ay NCircle_props NCircleAttr NCircleAttrData

    set NCircle_props { Transformations Attributes Material Tags NCircleAttr }

    array set NCircleAttr {
	arr   NCircleAttrData
	sproc ""
	gproc ""
	w     fNCircleAttr
    }

    array set NCircleAttrData {
	DisplayMode 1
	NCInfoBall "N/A"
    }

    # create NCircleAttr-UI
    set w [frame $ay(pca).$NCircleAttr(w)]
    set a $NCircleAttr(arr)
    addVSpace $w s1 2
    addParam $w $a Radius
    addParam $w $a TMin
    addParam $w $a TMax

    addParam $w $a Tolerance
    addMenu $w $a DisplayMode $ay(ncdisplaymodes)

    addText $w $a "Created NURBS Curve:"
    addInfo $w $a NCInfo

 return;
}
# init_NCircle
