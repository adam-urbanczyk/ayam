# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2001 by Randolf Schultz
# (rschultz@informatik.uni-rostock.de) and others.
#
# All rights reserved.
#
# See the file License for details.

# sdmesh.tcl - SDMesh objects Tcl code

set SDMesh_props { Transformations Attributes Material Tags SDMeshAttr }

array set SDMeshAttr {
arr   SDMeshAttrData
sproc ""
gproc ""
w     fSDMeshAttr

}

array set SDMeshAttrData {
Type 0
}
# create SDMeshAttr-UI
set w [frame $ay(pca).$SDMeshAttr(w)]

addInfo $w SDMeshAttrData Scheme
addInfo $w SDMeshAttrData NFaces
addInfo $w SDMeshAttrData NControls

