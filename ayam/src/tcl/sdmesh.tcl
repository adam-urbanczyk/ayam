# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2001 by Randolf Schultz
# (randolf.schultz@gmail.com) and others.
#
# All rights reserved.
#
# See the file License for details.

# sdmesh.tcl - SDMesh objects Tcl code

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
    set a $SDMeshAttr(arr)
    addVSpace $w s1 2
    addMenu $w $a Scheme {Catmull-Clark Loop}
    addParam $w $a Level
    addCheck $w $a DrawSub
    addInfo $w $a NFaces
    addInfo $w $a NControls
    addInfo $w $a NTags

 return;
}
# init_SDMesh
