# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2001 by Randolf Schultz
# (rschultz@informatik.uni-rostock.de) and others.
#
# All rights reserved.
#
# See the file License for details.

# level.tcl - level objects Tcl code

set Level_props { Transformations Attributes Material Tags LevelAttr }


array set LevelAttr {
arr   LevelAttrData
sproc ""
gproc ""
w     fLevelAttr

}

array set LevelAttrData {
Type 0
}

# create LevelAttr-UI
set w [frame $ay(pca).$LevelAttr(w)]
addMenu $w LevelAttrData Type {Level Union Difference Intersection Primitive }

