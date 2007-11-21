# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2001 by Randolf Schultz
# (rschultz@informatik.uni-rostock.de) and others.
#
# All rights reserved.
#
# See the file License for details.

# swing.tcl - Swing objects Tcl code

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

addCheck $w SwingAttrData UpperCap
addCheck $w SwingAttrData LowerCap
addCheck $w SwingAttrData StartCap
addCheck $w SwingAttrData EndCap

addParam $w SwingAttrData Tolerance
addMenu $w SwingAttrData DisplayMode $ay(npdisplaymodes)

addText $w SwingAttrData "Created NURBS Patch:"
addInfo $w SwingAttrData NPInfo
