# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2001 by Randolf Schultz
# (randolf.schultz@gmail.com) and others.
#
# All rights reserved.
#
# See the file License for details.

# sdmesh.tcl - SDMesh objects Tcl code

set SDMesh 1

proc init_SDMesh { } {
global ay SDMesh_props SDMeshAttr SDMeshAttrData

set SDMesh_props { Transformations Attributes Material Tags SDMeshAttr }

array set SDMeshAttr {
arr   SDMeshAttrData
sproc ""
gproc ""
w     fSDMeshAttr
}

array set SDMeshAttrData {
Scheme 0
}
# create SDMeshAttr-UI
set w [frame $ay(pca).$SDMeshAttr(w)]
addVSpace $w s1 2
addMenu $w SDMeshAttrData Scheme {Catmull-Clark Loop}
addParam $w SDMeshAttrData Level
addCheck $w SDMeshAttrData DrawSub
addInfo $w SDMeshAttrData NFaces
addInfo $w SDMeshAttrData NControls

return;
}
# init_SDMesh
