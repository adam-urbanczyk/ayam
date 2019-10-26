# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2001 by Randolf Schultz
# (randolf.schultz@gmail.com) and others.
#
# All rights reserved.
#
# See the file License for details.

# torus.tcl - torus objects Tcl code

proc init_Torus { } {
    global ay Torus_props  TorusAttr TorusAttrData

    set Torus_props { Transformations Attributes Material Tags TorusAttr }

    array set TorusAttr {
	arr   TorusAttrData
	sproc ""
	gproc ""
	w     fTorusAttr
    }

    array set TorusAttrData {
	Closed 1
    }

    # create TorusAttr-UI
    set w [frame $ay(pca).$TorusAttr(w)]
    set a $TorusAttr(arr)
    addVSpace $w s1 2
    addCheck $w $a Closed
    addParam $w $a MajorRad
    addParam $w $a MinorRad
    addParam $w $a PhiMin
    addParam $w $a PhiMax
    addParam $w $a ThetaMax

 return;
}
# init_Torus
