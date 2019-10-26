# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2001 by Randolf Schultz
# (randolf.schultz@gmail.com) and others.
#
# All rights reserved.
#
# See the file License for details.

# cylinder.tcl - cylinder objects Tcl code

proc init_Cylinder { } {
    global ay Cylinder_props CylinderAttr CylinderAttrData

    set Cylinder_props { Transformations Attributes Material Tags CylinderAttr }

    array set CylinderAttr {
	arr   CylinderAttrData
	sproc ""
	gproc ""
	w     fCylinderAttr
    }

    array set CylinderAttrData {
	Closed 1
	Radius 1.0
	ZMin -1.0
	ZMax 1.0
	ThetaMax 1.0
    }

    # create CylinderAttr-UI
    set w [frame $ay(pca).$CylinderAttr(w)]
    set a $CylinderAttr(arr)
    addVSpace $w s1 2
    addCheck $w $a Closed
    addParam $w $a Radius
    addParam $w $a ZMin
    addParam $w $a ZMax
    addParam $w $a ThetaMax

 return;
}
# init_Cylinder
