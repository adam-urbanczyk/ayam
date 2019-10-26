# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2001 by Randolf Schultz
# (randolf.schultz@gmail.com) and others.
#
# All rights reserved.
#
# See the file License for details.

# cone.tcl - cone objects Tcl code

proc init_Cone { } {
    global ay Cone_props ConeAttr ConeAttrData

    set Cone_props { Transformations Attributes Material Tags ConeAttr }

    array set ConeAttr {
	arr   ConeAttrData
	sproc ""
	gproc ""
	w     fConeAttr
    }

    array set ConeAttrData {
	Radius 1.0
	Height 0.0
	ThetaMax 1.0
    }

    # create ConeAttr-UI
    set w [frame $ay(pca).$ConeAttr(w)]
    set a $ConeAttr(arr)
    addVSpace $w s1 2
    addCheck $w $a Closed
    addParam $w $a Radius
    addParam $w $a Height
    addParam $w $a ThetaMax

 return;
}
# init_Cone
