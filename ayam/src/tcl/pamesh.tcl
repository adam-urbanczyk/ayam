# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2001 by Randolf Schultz
# (rschultz@informatik.uni-rostock.de) and others.
#
# All rights reserved.
#
# See the file License for details.

# pamesh.tcl - PatchMesh objects Tcl code

set PatchMesh_props { Transformations Attributes Material Tags PatchMeshAttr }


array set PatchMeshAttr {
arr   PatchMeshAttrData
sproc ""
gproc pamesh_getAttr
w     fPatchMeshAttr

}

array set PatchMeshAttrData {
Type 0
BType_U 0
BType_V 0
Step_U 0
Step_V 0
DisplayMode 1
}

for {set i 0} {$i < 16} {incr i} {
    set PatchMeshAttrData(Basis_U_${i}) 0.0
    set PatchMeshAttrData(Basis_V_${i}) 0.0
}

# pamesh_getAttr:
#  get Attributes from C context and build new PropertyGUI
#
proc pamesh_getAttr { } {
global ay PatchMeshAttr PatchMeshAttrData

catch {destroy $ay(pca).$PatchMeshAttr(w)}
set w [frame $ay(pca).$PatchMeshAttr(w)]
getProp

# create new PatchMeshAttr-UI
addMenu $w PatchMeshAttrData Type [list Bilinear Bicubic] 
addParam $w PatchMeshAttrData Width
addParam $w PatchMeshAttrData Height
addCheck $w PatchMeshAttrData Close_U
addCheck $w PatchMeshAttrData Close_V
addMenu $w PatchMeshAttrData BType_U [list Bezier B-Spline CatmullRom Hermite Custom]
addMenu $w PatchMeshAttrData BType_V [list Bezier B-Spline CatmullRom Hermite Custom]

if { $PatchMeshAttrData(BType_U) == 4 } {
    addParam $w PatchMeshAttrData Step_U
    addMatrix $w PatchMeshAttrData Basis_U
}

if { $PatchMeshAttrData(BType_V) == 4 } {
    addParam $w PatchMeshAttrData Step_V
    addMatrix $w PatchMeshAttrData Basis_V
}

addParam $w PatchMeshAttrData Tolerance
global ay
set l $ay(npdisplaymodes)
set l [linsert $l 0 Global]
addMenu $w PatchMeshAttrData DisplayMode $l

$ay(pca) itemconfigure 1 -window $w

plb_resize
# adapt scrollregion
set width [expr [winfo reqwidth $w] + 10]
set height [expr [winfo reqheight $w] + 10]
$ay(pca) configure -scrollregion [list 0 5 $width $height]

}
# pamesh_getAttr