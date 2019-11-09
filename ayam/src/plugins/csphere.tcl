# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2001 by Randolf Schultz
# (randolf.schultz@gmail.com) and others.
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

set ay(bok) $ay(appb)

# create CSphereAttr-UI
set w [frame $ay(pca).$CSphereAttr(w)]
set a $CSphereAttr(arr)

addVSpace $w s1 2
addCheck $w $a Closed
addParam $w $a Radius
addParam $w $a ZMin
addParam $w $a ZMax
addParam $w $a ThetaMax

# add menu entry to the "Create/Custom Object" sub-menu
mmenu_addcustom CSphere "crtOb CSphere;uCR;sL;rV"

# tell the rest of Ayam (or other custom objects), that we are loaded
lappend ay(co) CSphere
