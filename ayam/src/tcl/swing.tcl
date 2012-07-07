# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2001 by Randolf Schultz
# (randolf.schultz@gmail.com) and others.
#
# All rights reserved.
#
# See the file License for details.

# swing.tcl - Swing objects Tcl code

set Swing 1

proc init_Swing { } {
global ay Swing_props SwingAttr SwingAttrData

set Swing_props { Transformations Attributes Material Tags SwingAttr }

array set SwingAttr {
arr   SwingAttrData
sproc ""
gproc ""
w     fSwingAttr
}

array set SwingAttrData {
DisplayMode 1
NPInfoBall "n/a"
}
# create SwingAttr-UI
set w [frame $ay(pca).$SwingAttr(w)]
addVSpace $w s1 2
addCheck $w SwingAttrData UpperCap
addCheck $w SwingAttrData LowerCap
addCheck $w SwingAttrData StartCap
addCheck $w SwingAttrData EndCap

addParam $w SwingAttrData Tolerance
addMenu $w SwingAttrData DisplayMode $ay(npdisplaymodes)

addText $w SwingAttrData "Created NURBS Patch:"
addInfo $w SwingAttrData NPInfo

return;
}
# init_Swing
