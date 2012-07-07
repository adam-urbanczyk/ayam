# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2001 by Randolf Schultz
# (randolf.schultz@gmail.com) and others.
#
# All rights reserved.
#
# See the file License for details.

# sphere.tcl - sphere objects Tcl code

set Sphere 1

proc init_Sphere { } {
global ay Sphere_props SphereAttr SphereAttrData

set Sphere_props { Transformations Attributes Material Tags SphereAttr }

array set SphereAttr {
arr   SphereAttrData
sproc ""
gproc ""
w     fSphereAttr
}

# create SphereAttr-UI
set w [frame $ay(pca).$SphereAttr(w)]
addVSpace $w s1 2
addCheck $w SphereAttrData Closed
addParam $w SphereAttrData Radius
addParam $w SphereAttrData ZMin
addParam $w SphereAttrData ZMax
addParam $w SphereAttrData ThetaMax

array set SphereAttrData {
Closed 1
Radius 1.0
ZMin -1.0
ZMax 1.0
ThetaMax 1.0
}

return;
}
# init_Sphere
