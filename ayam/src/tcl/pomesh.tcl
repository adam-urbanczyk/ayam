# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2001 by Randolf Schultz
# (rschultz@informatik.uni-rostock.de) and others.
#
# All rights reserved.
#
# See the file License for details.

# pomesh.tcl - PolyMesh objects Tcl code

set PolyMesh_props { Transformations Attributes Material Tags PolyMeshAttr }

array set PolyMeshAttr {
arr   PolyMeshAttrData
sproc ""
gproc ""
w     fPolyMeshAttr

}

array set PolyMeshAttrData {
Type 0
}
# create PolyMeshAttr-UI
set w [frame $ay(pca).$PolyMeshAttr(w)]

addMenu $w PolyMeshAttrData Type [list Unused]
