# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2011 by Randolf Schultz
# (randolf.schultz@gmail.com) and others.
#
# All rights reserved.
#
# See the file License for details.

# ipatch.tcl - IPatch objects Tcl code

# ipatch_getAttr:
#  get Attributes from C context and build new PropertyGUI
#
proc ipatch_getAttr { } {
    global ay ayprefs IPatchAttr IPatchAttrData

    set oldfocus [focus]

    catch {destroy $ay(pca).$IPatchAttr(w)}
    set w [frame $ay(pca).$IPatchAttr(w)]
    set a $IPatchAttr(arr)

    getProp

    set ay(bok) $ay(appb)

    # create IPatchAttr-UI
    addVSpace $w s1 2
    addParam $w $a Width
    addParam $w $a Height

    addParam $w $a Order_U
    addParam $w $a Order_V

    addCheck $w $a Close_U
    addCheck $w $a Close_V

    addMenu $w $a Knot-Type_U [list Chordal Centripetal Uniform]
    addMenu $w $a Knot-Type_V [list Chordal Centripetal Uniform]

    if { $IPatchAttrData(Order_U) > 2 } {
	addMenu $w $a Derivatives_U [list None Automatic Manual]
	if { $IPatchAttrData(Derivatives_U) == 1 } {
	    addParam $w $a SDLen_U
	    addParam $w $a EDLen_U
	}
    }

    if { $IPatchAttrData(Order_V) > 2 } {
	addMenu $w $a Derivatives_V [list None Automatic Manual]
	if { $IPatchAttrData(Derivatives_V) == 1 } {
	    addParam $w $a SDLen_V
	    addParam $w $a EDLen_V
	}
    }

    addParam $w $a Tolerance
    addMenu $w $a DisplayMode $ay(npdisplaymodes)

    addText $w $a "Created NURBS Patch:"
    addInfo $w $a NPInfo

    plb_setwin $w $oldfocus

 return;
}
# ipatch_getAttr

# initialize object type
proc init_IPatch { } {
    global ay IPatch_props IPatchAttr IPatchAttrData

    set IPatch_props [list Transformations Attributes Material Tags Bevels\
			  Caps IPatchAttr]

    array set IPatchAttr {
	arr   IPatchAttrData
	sproc ""
	gproc ipatch_getAttr
	w     fIPatchAttr
    }

    array set IPatchAttrData {
	Knot-Type_U 0
	Knot-Type_V 0
	DisplayMode 1
	NPInfoBall "n/a"
	BoundaryNames { "U0" "U1" "V0" "V1" }
	BevelsChanged 0
	CapsChanged 0
    }

 return;
}
# init_IPatch
