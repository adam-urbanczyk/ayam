# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2001 by Randolf Schultz
# (rschultz@informatik.uni-rostock.de) and others.
#
# All rights reserved.
#
# See the file License for details.

# sphere.tcl - sphere objects Tcl code

set Sphere_props { Transformations Attributes Material Tags SphereAttr }


array set SphereAttr {
arr   SphereAttrData
sproc ""
gproc ""
w     fSphereAttr

}

# create SphereAttr-UI
set w [frame $ay(pca).$SphereAttr(w)]

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
