# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2001 by Randolf Schultz
# (rschultz@informatik.uni-rostock.de) and others.
#
# All rights reserved.
#
# See the file License for details.

# pamesh.tcl - NURBS patchs objects Tcl code

set PatchMesh_props { Transformations Attributes Material Tags PatchMeshAttr }


array set PatchMeshAttr {
arr   PatchMeshAttrData
sproc ""
gproc ""
w     fPatchMeshAttr

}

array set PatchMeshAttrData {
Type 0
BType_U 0
BType_V 0
DisplayMode 1
}
# create PatchMeshAttr-UI
set w [frame $ay(pca).$PatchMeshAttr(w)]
addMenu $w PatchMeshAttrData Type [list Bilinear Bicubic] 
addParam $w PatchMeshAttrData Width
addParam $w PatchMeshAttrData Height
addCheck $w PatchMeshAttrData Close_U
addCheck $w PatchMeshAttrData Close_V
addMenu $w PatchMeshAttrData BType_U [list Bezier B-Spline CatmullRom Hermite Custom]
addMenu $w PatchMeshAttrData BType_V [list Bezier B-Spline CatmullRom Hermite Custom]
addParam $w PatchMeshAttrData Tolerance
global ay
set l $ay(npdisplaymodes)
set l [linsert $l 0 Global]
addMenu $w PatchMeshAttrData DisplayMode $l

