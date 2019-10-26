# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2001 by Randolf Schultz
# (randolf.schultz@gmail.com) and others.
#
# All rights reserved.
#
# See the file License for details.

# hyperboloid.tcl - hyperboloid objects Tcl code

proc init_Hyperboloid { } {
    global ay Hyperboloid_props HyperboloidAttr HyperbAttrData

    set Hyperboloid_props [list Transformations Attributes Material Tags\
			       HyperboloidAttr]

    array set HyperboloidAttr {
	arr   HyperbAttrData
	sproc ""
	gproc ""
	w     fHyperboloidAttr
    }

    # create HyperboloidAttr-UI
    set w [frame $ay(pca).$HyperboloidAttr(w)]
    set a $HyperboloidAttr(arr)
    addVSpace $w s1 2
    addCheck $w $a Closed
    addParam $w $a ThetaMax
    addText $w e1 "Point 1:"
    addParam $w $a P1_X
    addParam $w $a P1_Y
    addParam $w $a P1_Z
    addText $w e2 "Point 2:"
    addParam $w $a P2_X
    addParam $w $a P2_Y
    addParam $w $a P2_Z

 return;
}
# init_Hyperboloid
