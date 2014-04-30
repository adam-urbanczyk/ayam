# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2009 by Randolf Schultz
# (randolf.schultz@gmail.com) and others.
#
# All rights reserved.
#
# See the file License for details.

# trim.tcl - Trim objects Tcl code

set Trim 1

proc init_Trim { } {
global ay Trim_props TrimAttr TrimAttrData

set Trim_props { Transformations Attributes Material Tags TrimAttr }

array set TrimAttr {
arr   TrimAttrData
sproc ""
gproc ""
w     fTrimAttr
}

array set TrimAttrData {
PatchNum 0
ScaleMode 0
}

# create TrimAttr-UI
set w [frame $ay(pca).$TrimAttr(w)]
addVSpace $w s1 2
addParam $w TrimAttrData PatchNum
addMenu $w TrimAttrData ScaleMode {Absolute Relative Automatic}
return;
}
# init_Trim