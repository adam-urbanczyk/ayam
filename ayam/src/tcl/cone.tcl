# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2001 by Randolf Schultz
# (rschultz@informatik.uni-rostock.de) and others.
#
# All rights reserved.
#
# See the file License for details.

# cone.tcl - cone objects Tcl code

set Cone_props { Transformations Attributes Material Tags ConeAttr }


array set ConeAttr {
arr   ConeAttrData
sproc ""
gproc ""
w     fConeAttr

}

# create ConeAttr-UI
set w [frame $ay(pca).$ConeAttr(w)]

addCheck $w ConeAttrData Closed
addParam $w ConeAttrData Radius
addParam $w ConeAttrData Height
addParam $w ConeAttrData ThetaMax

array set ConeAttrData {

Radius 1.0

Height 0.0
ThetaMax 1.0
}
