# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2017 by Randolf Schultz
# (randolf.schultz@gmail.com) and others.
#
# All rights reserved.
#
# See the file License for details.

# bcurve.tcl - BCurve objects Tcl code

global ay BCurve_props BCurveAttr BCurveAttrData

set BCurve_props { Transformations Attributes Tags BCurveAttr }

array set BCurveAttr {
arr   BCurveAttrData
sproc ""
gproc bcurve_getAttr
w     fBCurveAttr
}

array set BCurveAttrData {
BType 0
Step 1
DisplayMode 1
}

for {set i 0} {$i < 16} {incr i} {
    set BCurveAttrData(Basis_${i}) 0.0
}


# bcurve_getAttr:
#  get Attributes from C context and build new PropertyGUI
#
proc bcurve_getAttr { } {
    global ay BCurveAttr BCurveAttrData

    set oldfocus [focus]

    # remove old, create new BCurveAttr-UI
    catch {destroy $ay(pca).$BCurveAttr(w)}
    set w [frame $ay(pca).$BCurveAttr(w)]
    set a $BCurveAttr(arr)
    getProp

    set ay(bok) $ay(appb)

    addVSpace $w s1 2
    addCheck $w $a Closed
    addParam $w $a Length
    addInfo $w $a IsRat
    addMenu $w $a BType [list Bezier B-Spline CatmullRom Hermite Power Custom]

    if { $BCurveAttrData(BType) == 5 } {
	addParam $w $a Step
	addMatrix $w $a Basis
    }

    addParam $w $a Tolerance
    addMenu $w $a DisplayMode $ay(ncdisplaymodes)

    # advanced bindings for Length manipulation
    bind $w.fLength.b1 <Control-ButtonPress-1> "pamesh_updateWHL $w BCurveAttrData Length \$BCurveAttrData(BType) m;break"
    bind $w.fLength.b2 <Control-ButtonPress-1> "pamesh_updateWHL $w BCurveAttrData Length \$BCurveAttrData(BType) p;break"

    # add UI to property canvas
    plb_setwin $w $oldfocus

 return;
}
# bcurve_getAttr


# add menu entry to the "Create/Custom Object" sub-menu
mmenu_addcustom BCurve "crtOb BCurve;uS;sL;rV"

# tell the rest of Ayam (or other custom objects), that we are loaded
lappend ay(co) BCurve

# EOF
