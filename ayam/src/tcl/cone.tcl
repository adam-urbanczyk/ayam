# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2001 by Randolf Schultz
# (randolf.schultz@gmail.com) and others.
#
# All rights reserved.
#
# See the file License for details.

# cone.tcl - cone objects Tcl code

set Cone 1

proc init_Cone { } {
global ay Cone_props ConeAttr ConeAttrData

set Cone_props { Transformations Attributes Material Tags ConeAttr }

array set ConeAttr {
arr   ConeAttrData
sproc ""
gproc ""
w     fConeAttr

}

# create ConeAttr-UI
set w [frame $ay(pca).$ConeAttr(w)]
addVSpace $w s1 2
addCheck $w ConeAttrData Closed
addParam $w ConeAttrData Radius
addParam $w ConeAttrData Height
addParam $w ConeAttrData ThetaMax

array set ConeAttrData {

Radius 1.0

Height 0.0
ThetaMax 1.0
}

return;
}
# init_Cone
