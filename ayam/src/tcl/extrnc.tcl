# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2004 by Randolf Schultz
# (randolf.schultz@gmail.com) and others.
#
# All rights reserved.
#
# See the file License for details.

# extrnc.tcl - ExtrNC objects Tcl code

proc init_ExtrNC { } {
    global ay ExtrNC_props ExtrNCAttr ExtrNCAttrData

    set ExtrNC_props { Attributes Tags ExtrNCAttr }

    array set ExtrNCAttr {
	arr   ExtrNCAttrData
	sproc ""
	gproc extrnc_getAttr
	w     fExtrNCAttr
    }

    array set ExtrNCAttrData {
	Side 1
	DisplayMode 1
	NCInfoBall "N/A"
	Extract 0
    }

 return;
}
# init_ExtrNC


# extrnc_getAttr:
#  get Attributes from C context and build new PropertyGUI
#
proc extrnc_getAttr { } {
    global ay ayprefs ExtrNCAttr ExtrNCAttrData

    set oldfocus [focus]

    catch {destroy $ay(pca).$ExtrNCAttr(w)}
    set w [frame $ay(pca).$ExtrNCAttr(w)]
    set a $ExtrNCAttr(arr)

    set ExtrNCAttrData(trims) ""

    set ay(bok) $ay(appb)

    # create new UI
    addVSpace $w s1 2
    set sides [list U0 Un V0 Vn U V Boundary Middle_U Middle_V]
    set cnt 1
    foreach trim $ExtrNCAttrData(trims) {
	if { $trim == "Level" || $trim == "NCurve" } {
	    lappend sides Trim$cnt
	} else {
	    lappend sides Trim${cnt}($trim)
	}
	incr cnt
    }
    addMenu $w $a Side $sides
    addParam $w $a Parameter
    addCheck $w $a Relative
    addParam $w $a PatchNum
    addCheck $w $a Revert
    addMenu $w $a Extract [list Nothing Normals "Normals&Tangents"]

    addParam $w $a Tolerance
    addMenu $w $a DisplayMode $ay(ncdisplaymodes)

    addText $w $a "Extracted NURBS Curve:"
    addInfo $w $a NCInfo

    getProp

    plb_setwin $w $oldfocus

 return;
}
# extrnc_getAttr
