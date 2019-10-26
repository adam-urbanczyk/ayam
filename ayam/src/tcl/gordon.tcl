# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2009 by Randolf Schultz
# (randolf.schultz@gmail.com) and others.
#
# All rights reserved.
#
# See the file License for details.

# gordon.tcl - Gordon objects Tcl code

proc init_Gordon { } {
    global ay Gordon_props GordonAttr GordonAttrData

    set Gordon_props [list Transformations Attributes Material Tags Bevels\
			  Caps GordonAttr]

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
    set a $GordonAttr(arr)
    addVSpace $w s1 2
    addCheck $w $a WatchCorners
    addParam $w $a Order_U
    addParam $w $a Order_V

    addParam $w $a Tolerance
    addMenu $w $a DisplayMode $ay(npdisplaymodes)

    addText $w $a "Created NURBS Patch:"
    addInfo $w $a NPInfo

    rename init_Gordon ""

 return;
}
# init_Gordon
