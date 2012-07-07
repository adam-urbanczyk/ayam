# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2001 by Randolf Schultz
# (randolf.schultz@gmail.com) and others.
#
# All rights reserved.
#
# See the file License for details.

# torus.tcl - torus objects Tcl code

set Torus 1

proc init_Torus { } {
global ay Torus_props  TorusAttr TorusAttrData

set Torus_props { Transformations Attributes Material Tags TorusAttr }

array set TorusAttr {
arr   TorusAttrData
sproc ""
gproc ""
w     fTorusAttr
}

# create TorusAttr-UI
set w [frame $ay(pca).$TorusAttr(w)]
addVSpace $w s1 2
addCheck $w TorusAttrData Closed
addParam $w TorusAttrData MajorRad
addParam $w TorusAttrData MinorRad
addParam $w TorusAttrData PhiMin
addParam $w TorusAttrData PhiMax
addParam $w TorusAttrData ThetaMax

array set TorusAttrData {
Closed 1

}

return;
}
# init_Torus
