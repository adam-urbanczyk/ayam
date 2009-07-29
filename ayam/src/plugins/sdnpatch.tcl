# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2009 by Randolf Schultz
# (randolf.schultz@gmail.com) and others.
#
# All rights reserved.
#
# See the file License for details.

# sdnpatch.tcl - SDNPatch objects Tcl code

global ay SDNPatch_props SDNPatchAttr SDNPatchAttrData

set SDNPatch_props { Transformations Attributes Material Tags SDNPatchAttr }

array set SDNPatchAttr {
arr   SDNPatchAttrData
sproc ""
gproc ""
w     fSDNPatchAttr

}

# create SDNPatchAttr-UI
set w [frame $ay(pca).$SDNPatchAttr(w)]


addParam $w SDNPatchAttrData Degree
addParam $w SDNPatchAttrData Level



# add menu entry to the "Create/Custom Object" sub-menu
mmenu_addcustom SDNPatch "crtOb SDNPatch;uS;sL;rV"

global ay
$ay(cm) add command -label "To SDNPatch" -command {
    sdnconvertNP; uS; rV; }

# tell the rest of Ayam (or other custom objects), that we are loaded
lappend ay(co) SDNPatch
