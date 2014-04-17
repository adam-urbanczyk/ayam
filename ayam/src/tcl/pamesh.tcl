# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2009 by Randolf Schultz
# (randolf.schultz@gmail.com) and others.
#
# All rights reserved.
#
# See the file License for details.

# pamesh.tcl - PatchMesh objects Tcl code

set PatchMesh 1

proc init_PatchMesh { } {
global ay PatchMesh_props PatchMeshAttr PatchMeshAttrData

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

}
# init_PatchMesh


# pamesh_getAttr:
#  get Attributes from C context and build new PropertyGUI
#
proc pamesh_getAttr { } {
    global ay PatchMeshAttr PatchMeshAttrData

    set oldfocus [focus]

    # remove old, create new PatchMeshAttr-UI
    catch {destroy $ay(pca).$PatchMeshAttr(w)}
    set w [frame $ay(pca).$PatchMeshAttr(w)]
    getProp

    set ay(bok) $ay(appb)
    addVSpace $w s1 2
    addMenu $w PatchMeshAttrData Type [list Bilinear Bicubic]
    addParam $w PatchMeshAttrData Width
    addParam $w PatchMeshAttrData Height
    addCheck $w PatchMeshAttrData Close_U
    addCheck $w PatchMeshAttrData Close_V

    addInfo $w PatchMeshAttrData IsRat

    if { $PatchMeshAttrData(Type) == 1 } {
	set l [list Bezier B-Spline CatmullRom Hermite Power Custom]
	addMenu $w PatchMeshAttrData BType_U $l
	addMenu $w PatchMeshAttrData BType_V $l

	if { $PatchMeshAttrData(BType_U) == 5 } {
	    addParam $w PatchMeshAttrData Step_U
	    addMatrix $w PatchMeshAttrData Basis_U
	}

	if { $PatchMeshAttrData(BType_V) == 5 } {
	    addParam $w PatchMeshAttrData Step_V
	    addMatrix $w PatchMeshAttrData Basis_V
	}
    }

    addParam $w PatchMeshAttrData Tolerance
    addMenu $w PatchMeshAttrData DisplayMode $ay(npdisplaymodes)

    # add UI to property canvas
    plb_setwin $w $oldfocus

 return;
}
# pamesh_getAttr