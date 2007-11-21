# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2001 by Randolf Schultz
# (rschultz@informatik.uni-rostock.de) and others.
#
# All rights reserved.
#
# See the file License for details.

# select.tcl - select objects Tcl code

set Select_props { Transformations Attributes Material Tags SelectAttr }


array set SelectAttr {
arr   SelectAttrData
sproc ""
gproc ""
w     fSelectAttr

}

array set SelectAttrData {
Index 0
}

# create SelectAttr-UI
set w [frame $ay(pca).$SelectAttr(w)]
addParam $w SelectAttrData Index
