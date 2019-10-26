# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2001 by Randolf Schultz
# (randolf.schultz@gmail.com) and others.
#
# All rights reserved.
#
# See the file License for details.

# sphere.tcl - sphere objects Tcl code

proc init_Sphere { } {
    global ay Sphere_props SphereAttr SphereAttrData

    set Sphere_props { Transformations Attributes Material Tags SphereAttr }

    array set SphereAttr {
	arr   SphereAttrData
	sproc ""
	gproc ""
	w     fSphereAttr
    }

    array set SphereAttrData {
	Closed 1
	Radius 1.0
	ZMin -1.0
	ZMax 1.0
	ThetaMax 1.0
    }

    # create SphereAttr-UI
    set w [frame $ay(pca).$SphereAttr(w)]
    set a $SphereAttr(arr)
    addVSpace $w s1 2
    addCheck $w $a Closed
    addParam $w $a Radius
    addParam $w $a ZMin
    addParam $w $a ZMax
    addParam $w $a ThetaMax

 return;
}
# init_Sphere
