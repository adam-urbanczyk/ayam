#
# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2015 by Randolf Schultz
# (randolf.schultz@gmail.com) and others.
#
# All rights reserved.
#
# See the file License for details.

# cvview.tcl - display NURBS patch points as property

array set CVView {
    arr CVView
    sproc ""
    gproc cvview_update
    w fCVView
    wi 0
    he 0
}

# cvview_update:
#
#
proc cvview_update { } {
    global ay CVView

    set ay(bok) $ay(appb)

    # create UI
    catch {destroy $ay(pca).$CVView(w)}
    set w [frame $ay(pca).$CVView(w)]

    getProp
    getType t
    switch $t {
	NPatch {
	    global NPatchAttrData
	    set CVView(wi) $NPatchAttrData(Width)
	    set CVView(he) $NPatchAttrData(Height)
	    set CVView(stride) 4
	}
	PatchMesh {
	    global PatchMeshAttrData
	    set CVView(wi) $PatchMeshAttrData(Width)
	    set CVView(he) $PatchMeshAttrData(Height)
	    set CVView(stride) 4
	}
	IPatch {
	    global IPatchAttrData
	    set CVView(wi) $IPatchAttrData(Width)
	    set CVView(he) $IPatchAttrData(Height)
	    set CVView(stride) 3
	}
	NCurve {
	    global NCurveAttrData
	    set CVView(wi) $NCurveAttrData(Length)
	    set CVView(he) 1
	    set CVView(stride) 4
	}
	ICurve {
	    global ICurveAttrData
	    set CVView(wi) $ICurveAttrData(Length)
	    set CVView(he) 1
	    set CVView(stride) 3
	}
	ACurve {
	    global ACurveAttrData
	    set CVView(wi) $ACurveAttrData(Length)
	    set CVView(he) 1
	    set CVView(stride) 3
	}
	default {
	    # error message?
	    return;
	}
    }

    getPnt -all CVView(pnts)
    selPnts -get CVView(spnts)

    # create canvas
    set m 20;
    set canvasw 200
    if { [expr $CVView(wi)*$m] > 200 } {
	set canvasw [expr [winfo width $ay(pca)] - 20 ]
	# prepare a horizontal scrollbar
	# if { [expr $CVView(wi)*$m] > $canvasw } {}
	scrollbar $w.scx -orient horizontal\
	    -command {global ay; $ay(pca).$CVView(w).ca xview}\
	    -takefocus 0
    }

    # vertival scrolling realized via property canvas...
    set canvash [expr $CVView(he)*$m + 20 ]

    set ca [canvas $w.ca -width $canvasw -height $canvash]
    pack $ca

    # fill canvas
    set r [expr 0.25*$m];
    set r2 [expr $r*2.0];

    set i 0; set im $m;
    while { $i < $CVView(wi) } {
	set j 0; set jm $m;
	while { $j < $CVView(he) } {
	    set color black
	    if { [lsearch $CVView(spnts) [expr $i*$CVView(he)+$j]] != -1 } {
		set color red
	    }
	    $ca create oval $im $jm [expr {$im+$r2}] [expr {$jm+$r2}]\
		-tags [list o "$i,$j"] -fill $color
	    incr j; incr jm $m
	}
	# vertical lines
	$ca create line [expr {$im+$r}] [expr {$r+$m}]\
	    [expr {$im+$r}] [expr {$CVView(he)*$m+$r}] -tags l
	# width labels
	$ca create text [expr $im+$r] $r2 -text [expr $i+1]
	incr i; incr im $m
    }
    # horizontal lines
    set j 0; set jm $m
    while { $j < $CVView(he) } {
	$ca create line [expr {$r+$m}] [expr {$jm+$r}]\
	    [expr {$CVView(wi)*$m+$r}] [expr {$jm+$r}] -tags l
	# height labels
	$ca create text $r2 [expr $jm+$r] -text [expr $j+1]
	incr j; incr jm $m
    }
    # arrow
    set x [expr {$CVView(wi)*$m+$r}]
    set y [expr {$jm+$r-$m}]
    $ca create line [expr $x-7] [expr $y-7] $x $y -tags l
    $ca create line [expr $x+8] [expr $y-8] $x $y -tags l

    $ca bind o <Enter> cvview_showvalues
    $ca bind o <Leave> "destroy $ay(pca).$CVView(w).ca.balloon"

    $ca lower l
    $ca bind o <1> cvview_toggleselect

    if {  [winfo exists $w.scx] } {
	pack $w.scx -side bottom -fill x
	$ca configure -xscrollcommand "$w.scx set"
	$ca configure -scrollregion [$ca bbox all]
    }

    plb_setwin $w ""

 return;
}
# cvview_update


# cvview_showvalues:
#  get coordinates of NPatch control point that the mouse currently entered
#  and display them in a balloon/tool tip window
proc cvview_showvalues { } {
    global ay CVView
    set ca $ay(pca).$CVView(w).ca
    set s [lindex [$ca gettags current] 1]
    scan $s "%d,%d" x y

    set li [expr ($x*$CVView(he)+$y)*$CVView(stride)]
    incr x
    incr y
    set pntx [lindex $CVView(pnts) $li]
    incr li
    set pnty [lindex $CVView(pnts) $li]
    incr li
    set pntz [lindex $CVView(pnts) $li]
    if { $CVView(stride) > 3 } {
	incr li
	set pntw [lindex $CVView(pnts) $li]
	set txt "$x,$y: ($pntx, $pnty, $pntz, $pntw)"
    } else {
	set txt "$x,$y: ($pntx, $pnty, $pntz)"
    }
    # create a balloon window with txt at the mouse position
    set wx [expr [winfo pointerx $ca] + 10]
    set wy [expr [winfo pointery $ca] + 10]
    set top $ca.balloon
    catch {destroy $top}
    toplevel $top -bd 1 -bg black
    if { $ay(ws) == "Aqua" } {
	::tk::unsupported::MacWindowStyle style $top help noActivates
    } else {
	wm overrideredirect $top 1
    }
    pack [message $top.txt -width 100c -fg black -bg lightyellow -text $txt]
    wm geometry $top \
	    [winfo reqwidth $top.txt]x[winfo reqheight $top.txt]+$wx+$wy
    raise $top

 return;
}
# cvview_showvalues


# cvview_toggleselect:
#  toggle selection of the clicked control point
#
proc cvview_toggleselect { } {
    global ay CVView
    set ca $ay(pca).$CVView(w).ca
    set s [lindex [$ca gettags current] 1]
    scan $s "%d,%d" x y

    set li [expr ($x*$CVView(he)+$y)]
    set pos [lsearch $CVView(spnts) $li]
    if { $pos != -1 } {
	set CVView(spnts) [lreplace $CVView(spnts) $pos $pos]
	# pnt is selected => deselect
	selPnts
	eval "selPnts $CVView(spnts)"
	$ca itemconfigure current -fill black
    } else {
	# pnt is not selected => select
	selPnts $li
	$ca itemconfigure current -fill red
	selPnts -get CVView(spnts)
    }

 return;
}
# cvview_toggleselect

# attach to custom menu
global ay
$ay(cm) add command -label "Add CV View to Object" -command {
    addTag NP CVView; plb_update }

# EOF
