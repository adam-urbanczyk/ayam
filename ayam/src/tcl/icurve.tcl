# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2001 by Randolf Schultz
# (randolf.schultz@gmail.com) and others.
#
# All rights reserved.
#
# See the file License for details.

# icurve.tcl - interpolating curves objects Tcl code

set ICurve 1

# icurve_getAttr:
#  get Attributes from C context and build new PropertyGUI
#
proc icurve_getAttr { } {
    global ay ayprefs ICurveAttr ICurveAttrData

    set oldfocus [focus]

    catch {destroy $ay(pca).$ICurveAttr(w)}
    set w [frame $ay(pca).$ICurveAttr(w)]
    getProp

    set ay(bok) $ay(appb)

    # create ICurveAttr-UI
    addVSpace $w s1 2
    addMenu $w ICurveAttrData Type [list "Open" "Closed" ]
    addParam $w ICurveAttrData Length
    addParam $w ICurveAttrData Order
    addMenu $w ICurveAttrData ParamType\
	[list "Chordal" "Centripetal" "Uniform" ]
    addMenu $w ICurveAttrData Derivatives [list "Automatic" "Manual"]
    if { $ICurveAttrData(Derivatives) == 0 } {
	    addParam $w ICurveAttrData SDLen
	    addParam $w ICurveAttrData EDLen
	}

    addParam $w ICurveAttrData Tolerance
    addMenu $w ICurveAttrData DisplayMode $ay(ncdisplaymodes)

    addText $w ICurveAttrData "Created NURBS Curve:"
    addInfo $w ICurveAttrData NCInfo

    plb_setwin $w $oldfocus

 return;
}
# icurve_getAttr

proc init_ICurve { } {
global ay ICurve_props ICurveAttr ICurveAttrData

    set ICurve_props { Transformations Attributes Tags ICurveAttr }

    array set ICurveAttr {
	arr   ICurveAttrData
	sproc ""
	gproc icurve_getAttr
	w     fICurveAttr
    }

    array set ICurveAttrData {
	Type 0
	DisplayMode 0
	ParamType 0
	Derivatives 0
	NCInfoBall "N/A"
    }

}
# init_ICurve
