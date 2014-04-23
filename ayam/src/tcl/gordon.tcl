# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2009 by Randolf Schultz
# (randolf.schultz@gmail.com) and others.
#
# All rights reserved.
#
# See the file License for details.

# gordon.tcl - Gordon objects Tcl code

set Gordon_props { Transformations Attributes Material Tags Bevels Caps GordonAttr }

array set GordonAttr {
    arr   GordonAttrData
    sproc {setProp undo}
    gproc ""
    w     fGordonAttr
}

array set GordonAttrData {
    DisplayMode 1
    NPInfoBall "n/a"
    BoundaryNames { "U0" "U1" "V0" "V1" }
    BoundaryIDs { 0 1 2 3 }
}

set w [frame $ay(pca).$GordonAttr(w)]
addVSpace $w s1 2
addCheck $w GordonAttrData WatchCorners
addParam $w GordonAttrData Order_U
addParam $w GordonAttrData Order_V

addParam $w GordonAttrData Tolerance
addMenu $w GordonAttrData DisplayMode $ay(npdisplaymodes)

addText $w GordonAttrData "Created NURBS Patch:"
addInfo $w GordonAttrData NPInfo
