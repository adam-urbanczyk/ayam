# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2011 by Randolf Schultz
# (randolf.schultz@gmail.com) and others.
#
# All rights reserved.
#
# See the file License for details.

# mirror.tcl - mirror objects Tcl code

set Mirror_props { Transformations Attributes Material Tags MirrorAttr }

array set MirrorAttr {
arr   MirrorAttrData
sproc ""
gproc ""
w     fMirrorAttr

}

array set MirrorAttrData {
    Plane 0
}

# create MirrorAttr-UI
set w [frame $ay(pca).$MirrorAttr(w)]
addVSpace $w s1 2
addMenu $w MirrorAttrData Plane [list "YZ-Plane" "XZ-Plane"\
				      "XY-Plane" ]
