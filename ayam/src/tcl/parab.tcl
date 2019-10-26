# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2001 by Randolf Schultz
# (randolf.schultz@gmail.com) and others.
#
# All rights reserved.
#
# See the file License for details.

# sphere.tcl - sphere objects Tcl code

proc init_Paraboloid { } {
    global ay Paraboloid_props ParaboloidAttr ParabAttrData

    set Paraboloid_props [list Transformations Attributes Material Tags\
			      ParaboloidAttr]

    array set ParaboloidAttr {
	arr   ParabAttrData
	sproc ""
	gproc ""
	w     fParaboloidAttr
    }

    array set ParabAttrData {
	Closed 1
	Radius 1.0
	ZMin -1.0
	ZMax 1.0
	ThetaMax 1.0
    }

    # create ParaboloidAttr-UI
    set w [frame $ay(pca).$ParaboloidAttr(w)]
    set a $ParaboloidAttr(arr)
    addVSpace $w s1 2
    addCheck $w $a Closed
    addParam $w $a RMax
    addParam $w $a ZMin
    addParam $w $a ZMax
    addParam $w $a ThetaMax

 return;
}
# init_Paraboloid
