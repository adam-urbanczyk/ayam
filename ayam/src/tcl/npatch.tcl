# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2001 by Randolf Schultz
# (randolf.schultz@gmail.com) and others.
#
# All rights reserved.
#
# See the file License for details.

# npatch.tcl - NURBS patchs objects Tcl code

set NPatch_props { Transformations Attributes Material Tags Bevels Caps NPatchAttr }

array set NPatchAttr {
arr   NPatchAttrData
sproc ""
gproc ""
w     fNPatchAttr
}

array set NPatchAttrData {
Knot-Type_U 1
Knot-Type_V 1
DisplayMode 1
Knots_U ""
Knots_U-Modified 0
Knots_V ""
Knots_V-Modified 0
BoundaryNames { "U0" "U1" "V0" "V1" }
BevelsChanged 0
CapsChanged 0
}

# create NPatchAttr-UI
set w [frame $ay(pca).$NPatchAttr(w)]
addVSpace $w s1 2
addParam $w NPatchAttrData Width
addParam $w NPatchAttrData Height
addParam $w NPatchAttrData Order_U
addParam $w NPatchAttrData Order_V

addMenu $w NPatchAttrData Knot-Type_U [list Bezier B-Spline NURB Custom\
					 Chordal Centripetal]
addString $w NPatchAttrData Knots_U
addMenu $w NPatchAttrData Knot-Type_V [list Bezier B-Spline NURB Custom\
					 Chordal Centripetal]
addString $w NPatchAttrData Knots_V

addCheck $w NPatchAttrData CreateMP

addInfo $w NPatchAttrData IsRat

addParam $w NPatchAttrData Tolerance
addMenu $w NPatchAttrData DisplayMode $ay(npdisplaymodes)

trace variable NPatchAttrData(Knots_U) w markPropModified
trace variable NPatchAttrData(Knots_V) w markPropModified

# npatch_break:
#
#
proc npatch_break { } {
    global ay ayprefs ay_error npatchbrk_options

    if { ! [info exists npatchbrk_options] } {
	set npatchbrk_options(Direction) 0
    }

    winAutoFocusOff

    set npatchbrk_options(oldfocus) [focus]

    set w .npatchbrk
    set t "Break NPatch"
    winDialog $w $t

    set f [frame $w.f1]
    pack $f -in $w -side top -fill x

    set ay(bca) $w.f2.bca
    set ay(bok) $w.f2.bok

    if { $ayprefs(FixDialogTitles) == 1 } {
	addText $f e1 $t
    }

    addMenu $f npatchbrk_options Direction {"U" "V"}
    addCheck $f npatchbrk_options ApplyTrafo

    set f [frame $w.f2]
    button $f.bok -text "Ok" -width 5 -command {
	global ay_error npatchbrk_options
	set cmd "breakNP"
	if { $npatchbrk_options(ApplyTrafo) == 1 } {
	    append cmd " -a"
	}
	if { $npatchbrk_options(Direction) == 1 } {
	    append cmd " -v"
	}
	set ay_error ""
	eval $cmd
	uCR; rV; set ay(sc) 1
	if { $ay_error > 1 } {
	    ayError 2 "Break" "There were errors while breaking!"
	}

	grab release .npatchbrk
	restoreFocus $npatchbrk_options(oldfocus)
	destroy .npatchbrk
    }

    button $f.bca -text "Cancel" -width 5 -command "\
	    grab release .npatchbrk;\
	    restoreFocus $npatchbrk_options(oldfocus);\
	    destroy .npatchbrk"

    pack $f.bok $f.bca -in $f -side left -fill x -expand yes
    pack $f -in $w -side bottom -fill x

    # Esc-key && close via window decoration == Cancel button
    bind $w <Escape> "$f.bca invoke"
    wm protocol $w WM_DELETE_WINDOW "$f.bca invoke"

    shortcut_addcshelp $w ayam-5.html breaknpt

    winRestoreOrCenter $w $t
    grab $w
    focus $w.f2.bok
    tkwait window $w

    winAutoFocusOn

    after idle viewMouseToCurrent

 return;
}
# npatch_break

# compute suggested new knots for refine knots with operation
proc npatch_getrknotsu {} {
    global ay NPatchAttrData
    set ay(refineknu) ""
    set ui [expr $NPatchAttrData(Order_U) - 1]
    while { $ui < [expr $NPatchAttrData(Width)] } {
	set u1 [lindex $NPatchAttrData(Knots_U) $ui]
	incr ui
	set u2 [lindex $NPatchAttrData(Knots_U) $ui]
	if { $u1 != $u2 } {
	    lappend ay(refineknu) [expr $u1 + ($u2-$u1)*0.5]
	}
    }
    if { [llength $ay(refineknu)] == 1 } {
	lappend ay(refineknu) [lindex $ay(refineknu) 0]
    }
}
# npatch_getrknotsu

# compute suggested new knots for refine knots with operation
proc npatch_getrknotsv {} {
    global ay NPatchAttrData
    set ay(refineknv) ""
    set vi [expr $NPatchAttrData(Order_V) - 1]
    while { $vi < [expr $NPatchAttrData(Height)] } {
	set v1 [lindex $NPatchAttrData(Knots_V) $vi]
	incr vi
	set v2 [lindex $NPatchAttrData(Knots_V) $vi]
	if { $v1 != $v2 } {
	    lappend ay(refineknv) [expr $v1 + ($v2-$v1)*0.5]
	}
    }
    if { [llength $ay(refineknv)] == 1 } {
	lappend ay(refineknv) [lindex $ay(refineknv) 0]
    }
}
# npatch_getrknotsv

