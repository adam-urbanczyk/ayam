# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2001 by Randolf Schultz
# (rschultz@informatik.uni-rostock.de) and others.
#
# All rights reserved.
#
# See the file License for details.

# disk.tcl - disk objects Tcl code

set Disk_props { Transformations Attributes Material Tags DiskAttr }


array set DiskAttr {
arr   DiskAttrData
sproc ""
gproc ""
w     fDiskAttr

}

# create DiskAttr-UI
set w [frame $ay(pca).$DiskAttr(w)]


addParam $w DiskAttrData Radius
addParam $w DiskAttrData Height
addParam $w DiskAttrData ThetaMax

array set DiskAttrData {

Radius 1.0

Height 0.0
ThetaMax 1.0
}
