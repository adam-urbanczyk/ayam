# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2001 by Randolf Schultz
# (rschultz@informatik.uni-rostock.de) and others.
#
# All rights reserved.
#
# See the file License for details.

# box.tcl - box objects Tcl code

set Box_props { Transformations Attributes Material Tags BoxAttr }


array set BoxAttr {
arr   BoxAttrData
sproc ""
gproc ""
w     fBoxAttr

}

# create BoxAttr-UI
set w [frame $ay(pca).$BoxAttr(w)]
addText $w e1 "Dimensions:"
addParam $w BoxAttrData Width
addParam $w BoxAttrData Length
addParam $w BoxAttrData Height

array set BoxAttrData {
Width 1.0
Length 1.0
Height 1.0
}
