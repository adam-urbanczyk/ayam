# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2001 by Randolf Schultz
# (rschultz@informatik.uni-rostock.de) and others.
#
# All rights reserved.
#
# See the file License for details.

# sphere.tcl - sphere objects Tcl code

set Paraboloid_props { Transformations Attributes Material Tags ParaboloidAttr }


array set ParaboloidAttr {
arr   ParabAttrData
sproc ""
gproc ""
w     fParaboloidAttr

}

# create ParaboloidAttr-UI
set w [frame $ay(pca).$ParaboloidAttr(w)]

addCheck $w ParabAttrData Closed
addParam $w ParabAttrData RMax
addParam $w ParabAttrData ZMin
addParam $w ParabAttrData ZMax
addParam $w ParabAttrData ThetaMax

array set ParabAttrData {
Closed 1
Radius 1.0
ZMin -1.0
ZMax 1.0
ThetaMax 1.0
}
