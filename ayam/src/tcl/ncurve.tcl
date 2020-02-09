# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2001 by Randolf Schultz
# (randolf.schultz@gmail.com) and others.
#
# All rights reserved.
#
# See the file License for details.

# ncurve.tcl - NURBS curves objects Tcl code

set NCurve_props { Transformations Attributes Tags NCurveAttr }


array set NCurveAttr {
arr   NCurveAttrData
sproc ""
gproc ""
w     fNCurveAttr
}

array set NCurveAttrData {
Type 0
Knot-Type 1
DisplayMode 0
Knots-Modified 0
Knots ""
}

trace variable NCurveAttrData(Knots) w markPropModified

# create NCurveAttr-UI
set w [frame $ay(pca).$NCurveAttr(w)]
addVSpace $w s1 2
addMenu $w NCurveAttrData Type [list Open Closed Periodic]
addParam $w NCurveAttrData Length
addParam $w NCurveAttrData Order
addMenu $w NCurveAttrData Knot-Type [list Bezier B-Spline NURB Custom\
					 Chordal Centripetal]
addString $w NCurveAttrData Knots

addCheck $w NCurveAttrData CreateMP

addInfo $w NCurveAttrData IsRat
addParam $w NCurveAttrData Tolerance
addMenu $w NCurveAttrData DisplayMode $ay(ncdisplaymodes)

proc ncurve_open {} {
    forAll -type ncurve -recursive 0 {
	getProperty NCurveAttr(Type) oldtype
	setProperty NCurveAttr(Type) 0
    }
 return;
}
# ncurve_open

proc ncurve_close {} {
    forAll -type ncurve -recursive 0 {
	getProperty NCurveAttr(Type) oldtype
	setProperty NCurveAttr(Type) 1
    }
 return;
}
# ncurve_close

proc ncurve_makeperiodic {} {
    forAll -type ncurve -recursive 0 {
	getProperty NCurveAttr(Type) oldtype
	setProperty NCurveAttr(Type) 2
    }
 return;
}
# ncurve_makeperiodic

# compute suggested new knots for refine knots with operation
proc ncurve_getrknots {} {
    global ay NCurveAttrData
    set ay(refinekn) ""
    set ui [expr $NCurveAttrData(Order) - 1]
    while { $ui < [expr $NCurveAttrData(Length)] } {
	set u1 [lindex $NCurveAttrData(Knots) $ui]
	incr ui
	set u2 [lindex $NCurveAttrData(Knots) $ui]
	if { $u1 != $u2 } {
	    lappend ay(refinekn) [expr $u1 + ($u2-$u1)*0.5]
	}
    }
    if { [llength $ay(refinekn)] == 1 } {
	lappend ay(refinekn) [lindex $ay(refinekn) 0]
    }
}
# ncurve_getrknots

# split curve at parametric value
proc ncurve_split { u r {a 0} } {
    global ay
    undo save Split
    getSel sel
    set cmd splitNC
    if { $r } {
	append cmd " -r"
    }
    if { $a } {
	append cmd " -a"
    }
    append cmd " $u"
    eval $cmd
    cS

    if { $a } {
	uCR; sL; getSel lsel
	lappend sel $lsel
    } else {
	set ay(ul) $ay(CurrentLevel); uS
	if { [llength $sel] == 1 } {
	    lappend sel [expr $sel + 1]
	}
    }
    eval "selOb $sel"
    if { $ay(lb) == 0 } {
	tree_sync $ay(tree)
    } else {
	olb_sync $ay(lb)
    }
    rV
    plb_update

 return;
}
# ncurve_split