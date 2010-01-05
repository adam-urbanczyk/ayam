# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2001 by Randolf Schultz
# (randolf.schultz@gmail.com) and others.
#
# All rights reserved.
#
# See the file License for details.

# cylinder.tcl - cylinder objects Tcl code

set Cylinder 1

proc init_Cylinder { } {
global ay Cylinder_props CylinderAttr CylinderAttrData

set Cylinder_props { Transformations Attributes Material Tags CylinderAttr }


array set CylinderAttr {
arr   CylinderAttrData
sproc ""
gproc ""
w     fCylinderAttr

}

# create CylinderAttr-UI
set w [frame $ay(pca).$CylinderAttr(w)]

addCheck $w CylinderAttrData Closed
addParam $w CylinderAttrData Radius
addParam $w CylinderAttrData ZMin
addParam $w CylinderAttrData ZMax
addParam $w CylinderAttrData ThetaMax

array set CylinderAttrData {
Closed 1
Radius 1.0
ZMin -1.0
ZMax 1.0
ThetaMax 1.0
}

return;
}
# init_Cylinder
