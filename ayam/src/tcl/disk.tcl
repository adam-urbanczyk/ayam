# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2001 by Randolf Schultz
# (randolf.schultz@gmail.com) and others.
#
# All rights reserved.
#
# See the file License for details.

# disk.tcl - disk objects Tcl code

proc init_Disk { } {
    global ay Disk_props DiskAttr DiskAttrData

    set Disk_props { Transformations Attributes Material Tags DiskAttr }

    array set DiskAttr {
	arr   DiskAttrData
	sproc ""
	gproc ""
	w     fDiskAttr
    }

    array set DiskAttrData {
	Radius 1.0
	Height 0.0
	ThetaMax 1.0
    }

    # create DiskAttr-UI
    set w [frame $ay(pca).$DiskAttr(w)]
    set a $DiskAttr(arr)
    addVSpace $w s1 2
    addParam $w $a Radius
    addParam $w $a Height
    addParam $w $a ThetaMax

 return;
}
# init_Disk
