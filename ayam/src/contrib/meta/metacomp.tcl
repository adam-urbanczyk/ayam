# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2001 by Randolf Schultz
# (rschultz@informatik.uni-rostock.de) and others.
#
# All rights reserved.
#
# See the file License for details.

# metaobj.tcl - Metaobj objects Tcl code
# Frank Pagels 2001

global ay MetaComp_props MetaCompAttr MetaCompAttrData

set MetaComp_props { Transformations Attributes Material Tags MetaCompAttr }

proc formula_getAttr { } {
global ay MetaCompAttr MetaCompAttrData

catch {destroy $ay(pca).$MetaCompAttr(w)}
set w [frame $ay(pca).$MetaCompAttr(w)]
getProp


# create new UI
addMenu $w MetaCompAttrData Formula {MetaBalls_1 MetaBalls_2 MetaBalls_3 Torus}
addCheck $w MetaCompAttrData Negative

if { $MetaCompAttrData(Formula) == 0 } {
	addParam $w MetaCompAttrData Radius
	addParam $w MetaCompAttrData EnergyCoeffA
	addParam $w MetaCompAttrData EnergyCoeffB
	addParam $w MetaCompAttrData EnergyCoeffC
}
if { $MetaCompAttrData(Formula) == 1 } {

	addParam $w MetaCompAttrData Radius
	addParam $w MetaCompAttrData Strenght
}
if { $MetaCompAttrData(Formula) == 2 } {

	addParam $w MetaCompAttrData Radius
}
if { $MetaCompAttrData(Formula) == 3 } {

	addParam $w MetaCompAttrData Ri
	addParam $w MetaCompAttrData Ro
	addCheck $w MetaCompAttrData Rotate
}
$ay(pca) itemconfigure 1 -window $w

plb_resize
# adapt scrollregion
set width [expr [winfo reqwidth $w] + 10]
set height [expr [winfo reqheight $w] + 10]
$ay(pca) configure -scrollregion [list 0 5 $width $height]

}





array set MetaCompAttr {
arr   MetaCompAttrData
sproc ""
gproc formula_getAttr
w     fMetaCompAttr

}

array set MetaCompAttrData {
Formula  0

}

# create MetaCompAttr-UI
set w [frame $ay(pca).$MetaCompAttr(w)]

addCheck $w MetaCompAttrData Negative

# add menu entry to Create/Custom sub-menu
mmenu_addcustom MetaComp "crtOb MetaComp;uS;sL;rV"

# tell the rest of Ayam (or other custom objects), that we are loaded
lappend ay(co) MetaComp
