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

global ay MetaObj_props MetaObjAttr MetaObjAttrData

set MetaObj_props { Transformations Attributes Material Tags MetaObjAttr }


proc stdReleaseBind { w } {
     bind $w <ButtonRelease-1> {

	if { $ayprefs(LazyNotify) == 1 } { forceNot }
	rV
	plb_update
     
      set meta_mouseup 1
	 forceNot
      update
	 rV	 
     }
}
# stdReleaseBind


array set MetaObjAttr {
arr   MetaObjAttrData
sproc ""
gproc ""
w     fMetaObjAttr

}

array set MetaObjAttrData {
Formula  0

}

# create MetaObjAttr-UI
set w [frame $ay(pca).$MetaObjAttr(w)]

addParam $w MetaObjAttrData GridWith {20 40 60 80 120 140 160 180 200}
addParam $w MetaObjAttrData IsoLevel 0.6
addCheck $w MetaObjAttrData "ShowWorld"

global meta_mouseup; set meta_mouseup 1

# add menu entry to Create/Custom sub-menu
mmenu_addcustom MetaObj "crtOb MetaObj;uS;sL;rV"

# tell the rest of Ayam (or other custom objects), that we are loaded
lappend ay(co) MetaObj
