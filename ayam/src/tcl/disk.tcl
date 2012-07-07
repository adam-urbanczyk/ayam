# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2001 by Randolf Schultz
# (randolf.schultz@gmail.com) and others.
#
# All rights reserved.
#
# See the file License for details.

# disk.tcl - disk objects Tcl code

set Disk 1

proc init_Disk { } {
global ay Disk_props DiskAttr DiskAttrData

set Disk_props { Transformations Attributes Material Tags DiskAttr }

array set DiskAttr {
arr   DiskAttrData
sproc ""
gproc ""
w     fDiskAttr
}

# create DiskAttr-UI
set w [frame $ay(pca).$DiskAttr(w)]
addVSpace $w s1 2
addParam $w DiskAttrData Radius
addParam $w DiskAttrData Height
addParam $w DiskAttrData ThetaMax

array set DiskAttrData {
Radius 1.0
Height 0.0
ThetaMax 1.0
}

return;
}
# init_Disk
