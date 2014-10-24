# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2004 by Randolf Schultz
# (randolf.schultz@gmail.com) and others.
#
# All rights reserved.
#
# See the file License for details.

# extrnc.tcl - ExtrNC objects Tcl code

set ExtrNC 1

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

    set ExtrNCAttrData(trims) ""

    getProp

    set ay(bok) $ay(appb)

    # create new UI
    addVSpace $w s1 2
    set sides [list U0 Un V0 Vn U V Boundary Middle_U Middle_V]
    set cnt 1
    foreach trim $ExtrNCAttrData(trims) {
	if { $trim == "Level" || $trim == "NCurve" } {
	    lappend sides Trim$cnt
	} else {
	    lappend sides $trim
	}
	incr cnt
    }
    addMenu $w ExtrNCAttrData Side $sides
    addParam $w ExtrNCAttrData Parameter
    addCheck $w ExtrNCAttrData Relative
    addParam $w ExtrNCAttrData PatchNum
    addCheck $w ExtrNCAttrData Revert
    addMenu $w ExtrNCAttrData Extract [list Nothing Normals "Normals&Tangents"]

    addParam $w ExtrNCAttrData Tolerance
    addMenu $w ExtrNCAttrData DisplayMode $ay(ncdisplaymodes)

    addText $w ExtrNCAttrData "Extracted NURBS Curve:"
    addInfo $w ExtrNCAttrData NCInfo

    plb_setwin $w $oldfocus

 return;
}
# extrnc_getAttr
