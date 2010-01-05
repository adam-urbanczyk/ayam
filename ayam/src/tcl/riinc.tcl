# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2001 by Randolf Schultz
# (randolf.schultz@gmail.com) and others.
#
# All rights reserved.
#
# See the file License for details.

# riinc.tcl - riinc (RIB-Include) objects Tcl code

set RiInc 1

proc init_RiInc { } {
global ay RiInc_props RiIncAttr RiIncAttrData

set RiInc_props { Transformations Attributes Material Tags RiIncAttr }

array set RiIncAttr {
arr   RiIncAttrData
sproc ""
gproc ""
w     fRiIncAttr

}

# create RiIncAttr-UI
set w [frame $ay(pca).$RiIncAttr(w)]
addText $w e0 "Include:"
addFile $w RiIncAttrData File
addText $w e1 "Dimensions:"
addParam $w RiIncAttrData Width
addParam $w RiIncAttrData Length
addParam $w RiIncAttrData Height

array set RiIncAttrData {
File ""
Width 1.0
Length 1.0
Height 1.0
}

return;
}
# init_RiInc
