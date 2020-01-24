# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2001 by Randolf Schultz
# (randolf.schultz@gmail.com) and others.
#
# All rights reserved.
#
# See the file License for details.

# metacomp.tcl - Metaobj objects Tcl code
# Frank Pagels 2001

global ay MetaComp_props MetaCompAttr MetaCompAttrData

set MetaComp_props { Transformations Attributes Tags MetaCompAttr }

proc metacomp_setAttr { } {
    global ay MetaCompAttr MetaCompAttrData
    set t $ay(pca).$MetaCompAttr(w).tScript
    if { [winfo exists $t] } {
	set MetaCompAttrData(Expression) [$t get 1.0 end]
    }
    setProp
 return;
}

proc metacomp_getAttr { } {
    global ay ayprefs MetaCompAttr MetaCompAttrData

    set sw 40
    set sh 6
    if { [winfo exists $ay(pca).$MetaCompAttr(w).tScript] } {
	set sw [$ay(pca).$MetaCompAttr(w).tScript cget -width]
	set sh [$ay(pca).$MetaCompAttr(w).tScript cget -height]
    }

    catch {destroy $ay(pca).$MetaCompAttr(w)}
    set w [frame $ay(pca).$MetaCompAttr(w)]
    getProp

    set ay(bok) $ay(appb)
    set f $MetaCompAttr(arr)

    addVSpace $w s1 2
    addMenu $w $f Formula {MetaBalls Torus Cube Heart Custom}
    addCheck $w $f Negative

    if { $MetaCompAttrData(Formula) == 0 } {
	addParam $w $f Radius
	addParam $w $f EnergyCoeffA
	addParam $w $f EnergyCoeffB
	addParam $w $f EnergyCoeffC
    }
    if { $MetaCompAttrData(Formula) == 1 } {
	addParam $w $f Ri
	addParam $w $f Ro
	addCheck $w $f Rotate
    }
    if { $MetaCompAttrData(Formula) == 2 } {
	addParam $w $f EdgeX
	addParam $w $f EdgeY
	addParam $w $f EdgeZ
    }
    if { $MetaCompAttrData(Formula) == 3 } {

    }

    if { $MetaCompAttrData(Formula) == 4 } {
	addText $w t1 "Expression:"

	set t $w.tScript
	pack [text $t -undo 1 -width $sw -height $sh]
	if { $ay(ws) == "Aqua" } {
	    $t conf -relief sunken -bd 2
	}
	eval [subst "bindtags $t \{$t Text all\}"]
	bind $t <Key-Escape> "resetFocus;break"

	# create resize handle for text widget
	if { $ay(ws) == "Win32" } {
	    resizeHandle:Create $w.rsh $t -bg SystemWindow
	} else {
	    resizeHandle:Create $w.rsh $t
	}

	$t delete 1.0 end
	$t insert 1.0 $MetaCompAttrData(Expression)
	if { $MetaCompAttrData(Expression) != "" } {
	    $t delete "end - 1 chars"
	}

	# create popup menu
	set m [menu $t.popup -tearoff 0]
	$m add command -label "Clear All" -command "$t delete 1.0 end"
	$m add command -label "Paste (Replace)" -command "_pasteToText $t"
	$m add command -label "Load from file" -command "_loadToText $t"
	$m add separator
	foreach expression $MetaCompAttrData(Expressions) {
	    $m add command -label [lindex $expression 0] -command \
	      "$t delete 1.0 end; $t insert end \{[lindex $expression 1]\}"
	}

	# bind popup menu
	set mb 3
	if { $ay(ws) == "Aqua" && $ayprefs(SwapMB) } {
	    set mb 2
	}
	bind $t <$mb> {
	    global ay MetaCompAttr
	    set ay(xy) [winfo pointerxy .]
	    tk_popup $ay(pca).${MetaCompAttr(w)}.tScript.popup\
		[lindex $ay(xy) 0] [lindex $ay(xy) 1]
	}
	# bind

	after 100 "resizeHandle:PlaceHandle $w.rsh $t"
    }
    # if custom

    $ay(pca) itemconfigure 1 -window $w

    plb_resize

    # adapt scrollregion
    set width [expr [winfo reqwidth $w] + 10]
    set height [expr [winfo reqheight $w] + 10]
    $ay(pca) configure -scrollregion [list 0 5 $width $height]

 return;
}
# metacomp_getAttr


array set MetaCompAttr {
    arr   MetaCompAttrData
    sproc metacomp_setAttr
    gproc metacomp_getAttr
    w     fMetaCompAttr
}

array set MetaCompAttrData {
    Formula  0
    Expression ""
    Expressions {
	    { RoundCube "expr {pow($x,4)+pow($y,4)+pow($z,4)}" }
	    { OpenCylinder "expr {sqrt(pow($y,2)+pow($z,2))}" }
    }
}
# { set r 1.0;set f [expr {sqrt(pow($y,2)+pow($z,2))}];if {$x < -$r} {set f [expr {sqrt(pow(($x+$r),2)+pow($y,2)+pow($z,2))}]}; if {$x > $r} {set f [expr {sqrt(pow(($x-$r),2)+pow($y,2)+pow($z,2))}]} } }

# add menu entry to Create/Custom sub-menu
mmenu_addcustom MetaComp "crtOb MetaComp;uS;sL;notifyOb;rV"

# tell the rest of Ayam (or other custom objects), that we are loaded
lappend ay(co) MetaComp
