# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2008 by Randolf Schultz
# (randolf.schultz@gmail.com) and others.
#
# All rights reserved.
#
# See the file License for details.

# acurve.tcl - approximating curves objects Tcl code

proc init_ACurve { } {
    global ay ACurve_props ACurveAttr ACurveAttrData

    set ACurve_props { Transformations Attributes Tags ACurveAttr }

    array set ACurveAttr {
	arr   ACurveAttrData
	sproc ""
	gproc ""
	w     fACurveAttr
    }

    array set ACurveAttrData {
	Mode 1
	DisplayMode 0
	NCInfoBall "N/A"
    }

    # create ACurveAttr-UI
    set w [frame $ay(pca).$ACurveAttr(w)]
    set a $ACurveAttr(arr)
    addVSpace $w s1 2
    addParam $w $a Length
    addParam $w $a ALength
    addCheck $w $a Closed
    addCheck $w $a Symmetric
    addParam $w $a Order

    addParam $w $a Tolerance
    addMenu $w $a DisplayMode $ay(ncdisplaymodes)

    addText $w $a "Created NURBS Curve:"
    addInfo $w $a NCInfo

 return;
}
# init_ACurve
