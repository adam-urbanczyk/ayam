# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2001 by Randolf Schultz
# (randolf.schultz@gmail.com) and others.
#
# All rights reserved.
#
# See the file License for details.

# hyperboloid.tcl - hyperboloid objects Tcl code

set Hyperboloid 1

proc init_Hyperboloid { } {
global ay Hyperboloid_props HyperboloidAttr HyperbAttrData

set Hyperboloid_props { Transformations Attributes Material Tags HyperboloidAttr }


array set HyperboloidAttr {
arr   HyperbAttrData
sproc ""
gproc ""
w     fHyperboloidAttr
}

# create HyperboloidAttr-UI
set w [frame $ay(pca).$HyperboloidAttr(w)]
addVSpace $w s1 2
addCheck $w HyperbAttrData Closed
addParam $w HyperbAttrData ThetaMax
addText $w e1 "Point 1:"
addParam $w HyperbAttrData P1_X
addParam $w HyperbAttrData P1_Y
addParam $w HyperbAttrData P1_Z
addText $w e2 "Point 2:"
addParam $w HyperbAttrData P2_X
addParam $w HyperbAttrData P2_Y
addParam $w HyperbAttrData P2_Z

return;
}
# init_Hyperboloid
