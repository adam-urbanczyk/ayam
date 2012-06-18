# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2001 by Randolf Schultz
# (randolf.schultz@gmail.com) and others.
#
# All rights reserved.
#
# See the file License for details.

# cap.tcl - cap objects Tcl code

set Cap 1

proc init_Cap { } {
global ay Cap_props CapAttr CapAttrData

set Cap_props { Transformations Attributes Material Tags CapAttr }

array set CapAttr {
arr   CapAttrData
sproc ""
gproc ""
w     fCapAttr

}

array set CapAttrData {
Type 1
DisplayMode 0
NPInfoBall "n/a"
}
# create CapAttr-UI
set w [frame $ay(pca).$CapAttr(w)]

addMenu $w CapAttrData Type {Trim Simple Gordon}
addParam $w CapAttrData Tolerance
addMenu $w CapAttrData DisplayMode $ay(npdisplaymodes)

addText $w CapAttrData "Created NURBS Patch:"
addInfo $w CapAttrData NPInfo

return;
}
# init_Cap
