# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2001 by Randolf Schultz
# (rschultz@informatik.uni-rostock.de) and others.
#
# All rights reserved.
#
# See the file License for details.

# csphere.tcl - CSphere objects Tcl code

global ay CSphere_props CSphereAttr CSphereAttrData

set CSphere_props { Transformations Attributes Material Tags CSphereAttr }

array set CSphereAttr {
arr   CSphereAttrData
sproc ""
gproc ""
w     fCSphereAttr

}

array set CSphereAttrData {
Closed 1
Radius 1.0
ZMin -1.0
ZMax 1.0
ThetaMax 1.0
}

# create CSphereAttr-UI
set w [frame $ay(pca).$CSphereAttr(w)]

addCheck $w CSphereAttrData Closed
addParam $w CSphereAttrData Radius
addParam $w CSphereAttrData ZMin
addParam $w CSphereAttrData ZMax
addParam $w CSphereAttrData ThetaMax

# add menu entry to the "Create/Custom Object" sub-menu
mmenu_addcustom CSphere "crtOb CSphere;uS;sL;rV"

# tell the rest of Ayam (or other custom objects), that we are loaded
lappend ay(co) CSphere
