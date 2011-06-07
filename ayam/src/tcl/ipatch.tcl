# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2011 by Randolf Schultz
# (randolf.schultz@gmail.com) and others.
#
# All rights reserved.
#
# See the file License for details.

# ipatch.tcl - IPatch objects Tcl code

set IPatch 1

proc init_IPatch { } {
    global ay IPatch_props IPatchAttr IPatchAttrData

    set IPatch_props { Transformations Attributes Material Tags IPatchAttr }

    array set IPatchAttr {
	arr   IPatchAttrData
	sproc ""
	gproc ""
	w     fIPatchAttr

    }

    array set IPatchAttrData {
	Knot-Type_U 0
	Knot-Type_V 0
	DisplayMode 1
	NPInfoBall "n/a"
    }

    # create IPatchAttr-UI
    set w [frame $ay(pca).$IPatchAttr(w)]

    addParam $w IPatchAttrData Width
    addParam $w IPatchAttrData Height

    addParam $w IPatchAttrData Order_U
    addParam $w IPatchAttrData Order_V

    addCheck $w IPatchAttrData Close_U
    addCheck $w IPatchAttrData Close_V

    addMenu $w IPatchAttrData Knot-Type_U [list Chordal Centripetal]

    addMenu $w IPatchAttrData Knot-Type_V [list Chordal Centripetal]


    addParam $w IPatchAttrData Tolerance
    addMenu $w IPatchAttrData DisplayMode $ay(npdisplaymodes)

    addText $w IPatchAttrData "Created NURBS Patch:"
    addInfo $w IPatchAttrData NPInfo

 return;
}
# init_IPatch
