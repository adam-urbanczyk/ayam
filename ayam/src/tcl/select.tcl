# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2001 by Randolf Schultz
# (randolf.schultz@gmail.com) and others.
#
# All rights reserved.
#
# See the file License for details.

# select.tcl - select objects Tcl code

set Select 1

proc init_Select { } {
global ay Select_props SelectAttr SelectAttrData

set Select_props { Transformations Attributes Material Tags SelectAttr }

array set SelectAttr {
arr   SelectAttrData
sproc ""
gproc ""
w     fSelectAttr
}

array set SelectAttrData {
Indices 0
}

# create SelectAttr-UI
set w [frame $ay(pca).$SelectAttr(w)]
addVSpace $w s1 2
addString $w SelectAttrData Indices

return;
}
# init_Select
