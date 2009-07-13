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
