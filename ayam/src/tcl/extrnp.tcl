# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2004 by Randolf Schultz
# (randolf.schultz@gmail.com) and others.
#
# All rights reserved.
#
# See the file License for details.

# extrnp.tcl - ExtrNP objects Tcl code

proc init_ExtrNP { } {
    global ay ExtrNP_props ExtrNPAttr ExtrNPAttrData

    set ExtrNP_props { Attributes Material Tags Bevels Caps ExtrNPAttr }

    array set ExtrNPAttr {
	arr   ExtrNPAttrData
	sproc ""
	gproc ""
	w     fExtrNPAttr
    }

    array set ExtrNPAttrData {
	DisplayMode 1
	UMin 0.0
	UMin 1.0
	VMin 0.0
	VMin 1.0
	BoundaryNames { "U0" "U1" "V0" "V1" }
	BevelsChanged 0
	CapsChanged 0
    }

    # create ExtrNPAttr-UI
    set w [frame $ay(pca).$ExtrNPAttr(w)]
    set a $ExtrNPAttr(arr)
    addVSpace $w s1 2
    addParam $w $a UMin
    addParam $w $a UMax
    addParam $w $a VMin
    addParam $w $a VMax
    addCheck $w $a Relative
    addParam $w $a PatchNum

    addParam $w $a Tolerance
    addMenu $w $a DisplayMode $ay(npdisplaymodes)

    addText $w $a "Extracted NURBS Patch:"
    addInfo $w $a NPInfo

 return;
}
# init_ExtrNP
