# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2020 by Randolf Schultz
# (randolf.schultz@gmail.com) and others.
#
# All rights reserved.
#
# See the file License for details.

# curvature.tcl - display curvature diagram as property

array set Curvature {
    arr Curvature
    sproc ""
    gproc curvature_update
    w fCurvature
    Coordinates "n/a"
    umin 0.0
    umax 1.0
    numin 0.0
    numax 0.0
    wi 200
    he 200
    reset 0
    log 0
    update ""
}

#curvature_update:
# get property procedure of Curvature property
#
proc curvature_update { {numin ""} {numax ""} } {
    global ay Curvature NCurveAttrData

    if { $numin != "" && $numax != "" && $numin == $numax } {
	return;
    }

    getProp

    set ay(bok) $ay(appb)

    # create UI
    catch {destroy $ay(pca).$Curvature(w)}
    set w [frame $ay(pca).$Curvature(w)]
    addVSpace $w s1 12

    # create canvas
    set canvasw $Curvature(wi)
    set canvash $Curvature(he)

    if { $canvasw < 200 } { set canvasw 200 }
    if { $canvash < 200 } { set canvash 200 }

    set ca [canvas $w.ca -width [expr $canvasw+80] -height [expr $canvash+40]]
    pack $ca -side top -anchor nw
    set Curvature(ca) $ca

    # fill canvas
    set relative 0
    if { [getType] != "NCurve" } { set relative 1 }

    if { $relative } {
	set Curvature(umin) 0.0
    } else {
	set Curvature(umin)\
	    [lindex $NCurveAttrData(Knots) [expr $NCurveAttrData(Order)-1]]
    }
    set Curvature(tumin) $Curvature(umin)
    if { $numin != "" && $numin > $Curvature(umin) } {
	set Curvature(umin) $numin
    }

    if { $relative } {
	set Curvature(umax) 1.0
    } else {
	set Curvature(umax)\
	    [lindex $NCurveAttrData(Knots) $NCurveAttrData(Length)]
    }
    set Curvature(tumax) $Curvature(umax)
    if { $numax != "" && $numax <  $Curvature(umax)} {
	set Curvature(umax) $numax
    }

    set Curvature(ud) [expr [curvature_getU 1]-[curvature_getU 2]]

    set Curvature(pnts) ""
    set u $Curvature(umin)
    set ud [expr (${Curvature(umax)}-${Curvature(umin)})/${canvasw}]
    set cmin Inf
    set cmax 0.0
    set ui 0
    while { $ui < $canvasw } {
	if { $relative } {
	    set cu [curvatNC -r -u $u]
	} else {
	    set cu [curvatNC -u $u]
	}

	if { $Curvature(log) } { set cu [expr log($cu+1.0)] }

	if { $cu < $cmin } { set cmin $cu }
	if { $cu > $cmax } { set cmax $cu }
	lappend Curvature(pnts) $cu
	set u [expr $u+$ud]
	incr ui
    }
    if { $cmin < 10e-6 } { set cmin 0.0 }
    if { $cmin < 10e-3 } { set cmin 0.001 }
    if { $cmax < 10e-6 } { set cmax 0.0 }
    if { $cmax < 10e-3 } { set cmax 0.001 }

    normVar cmin
    normVar cmax

    #getCurvature -s $canvasw -r -umin $umin -umax $umax Curvature(pnts)
    #set cmin [lindex $Curvature(pnts) end-1]
    #set cmax [lindex $Curvature(pnts) end]

    set dx 40
    # vertical
    $ca create line $dx $canvash $dx 0 -arrow last -tags l
    $ca create text [expr $dx-5] 8 -anchor e -text [format "%.2g" $cmax]
    $ca create line [expr $dx-3] [expr $canvash/2] $dx [expr $canvash/2] -tags l
    set cmid [expr $cmin+($cmax-$cmin)*0.5]
    $ca create text [expr $dx-5] [expr $canvash/2] -anchor e\
	-text [format "%.2g" $cmid]
    set k "k"
    if { $Curvature(log) } { set k "log(k)" }
    $ca create text [expr $dx-18] [expr $canvash*0.25] -text $k -tag k
    $ca create text [expr $dx-5] $canvash -anchor e -text [format "%.2g" $cmin]

    # horizontal
    $ca create line $dx $canvash [expr $canvasw+$dx+20] $canvash\
	-arrow last -tags l
    if { $Curvature(umin) > $Curvature(tumin) } {set txt "... "}
    append txt $Curvature(umin)
    $ca create text [expr $dx+5] [expr $canvash+12] -text $txt
    $ca create line [expr $canvasw/2+$dx] [expr $canvash+4]\
	[expr $canvasw/2+$dx] $canvash -tags l
    $ca create text [expr $canvasw/2+$dx] [expr $canvash+12]\
	-text [expr $Curvature(umin)+($Curvature(umax)-$Curvature(umin))*0.5]
    $ca create text [expr $canvasw*0.75+$dx] [expr $canvash+20] -text u
    if { $relative } {
	$ca create text [expr $canvasw*0.75+$dx+6] [expr $canvash+26] -text r
    }
    set txt $Curvature(umax)
    if { $Curvature(umax) < $Curvature(tumax) } {append txt " ..."}
    $ca create text [expr $canvasw+$dx] [expr $canvash+12]\
	-text $txt
    $ca create line [expr $canvasw+$dx] [expr $canvash+4]\
	[expr $canvasw+$dx] $canvash -tags l

    set i 0
    set x $dx
    set ex [expr $canvasw+$dx]
    set cdiff [expr ${cmax}-${cmin}]
    if { $cdiff < 10e-6 } { set cdiff 1.0 }
    set y\
     [expr ${canvash}-(([lindex $Curvature(pnts) 0]-${cmin})/$cdiff)*$canvash]
    if { $y <= 2 } { set y 2 }
    set nx [expr $x+1]
    while { $x < $ex } {
	set cu [lindex $Curvature(pnts) $i]
	if { $cu == "" } { break; }
	set ny [expr ${canvash}-((${cu}-${cmin})/${cdiff})*${canvash}]
	if { $ny <= 2 } { set ny 2 }
	$ca create line $x $y $nx $ny -tags [list o "$cu"]
	incr i
	incr x
	incr nx
	set y $ny
    }

    # establish balloon binding
    if { 0 } {
	$ca bind o <Enter> curvature_showvalues
	$ca bind o <Leave> "destroy $ay(pca).$Curvature(w).ca.balloon"
    } else {
	bind $ca <Motion> {
	    catch {destroy $ay(pca).$Curvature(w).ca.balloon}
	    $Curvature(ca) delete s
	    if { %x < 40 || %x > [expr $Curvature(wi)+40] ||
		 %y > $Curvature(he) } break;
	    $Curvature(ca) create line %x 0 %x $Curvature(he) -tag s

	    set u [curvature_getU %x]
	    setMark [getPnt -eval $u]
	    catch {after cancel $Curvature(after)}
	    set Curvature(after) [after 100 curvature_showvalues %x]
	}
    }

    $ca lower l

    $ca bind k <1> curvature_togglelog
    $ca bind k <Enter> "$ca itemconf k -fill red"
    $ca bind k <Leave> "$ca itemconf k -fill black"

    # create resize handle for canvas widget
    if { $ay(ws) == "Win32" } {
	resizeHandle:Create $w.rsh $ca -bg SystemWindow
    } else {
	resizeHandle:Create $w.rsh $ca
    }
    if { [info exists Curvature(sx)] } {
	upvar #0 _resizeHandle$ay(pca).$Curvature(w).rsh ar
	set ar(startX) $Curvature(sx)
	set ar(startY) $Curvature(sy)
	set ar(oldWidth) $Curvature(ow)
	set ar(oldHeight) $Curvature(oh)
    }

    # zoom bindings
    bind $ca <ButtonPress-1> {
	if { %x < 40 || %x > [expr $Curvature(wi)+80] } break;
	set Curvature(numin) [curvature_getU %x]
	$Curvature(ca) create line %x 0 %x $Curvature(he) -tag s
    }
    bind $ca <B1-Motion> {
	if { ! [info exists Curvature(numin)] } break;
	$Curvature(ca) delete e
	$Curvature(ca) create line %x 0 %x $Curvature(he) -tag e
    }
    bind $ca <ButtonRelease-1> {
	if { ! [info exists Curvature(numin)] } break;
	set x %x
	if { $x < 40 } {set x 40}
	if { $x > [expr $Curvature(wi)+80] } {set x [expr $Curvature(wi)+80]}
	set Curvature(numax) [curvature_getU %x]
	$Curvature(ca) delete s e

	if { $Curvature(numin) < $Curvature(numax) } {
	    curvature_update $Curvature(numin) $Curvature(numax)
	}
	if { $Curvature(numin) > $Curvature(numax) } {
	    curvature_update $Curvature(numax) $Curvature(numin)
	}
    }

    # zoom by mouse wheel bindings
    bind $ca <ButtonPress-4> {curvature_zoom %x %D;break}
    bind $ca <ButtonPress-5> {curvature_zoom %x -%D;break}
    if { $ay(ws) == "Win32" || $ay(ws) == "Aqua" } {
	bind $ca <MouseWheel> {curvature_zoom %x %D;break}
    }

    # pan bindings
    bind $ca <ButtonPress-3> {
	%W delete s
	if { %x < 40 || %x > [expr $Curvature(wi)+80] } break;
	set Curvature(oldX) %x
	%W create line %x 0 %x $Curvature(he) -dash . -tag s
	catch {destroy %W.balloon}
    }
    bind $ca <B3-Motion> {
	if { %x < 40 || %x > [expr $Curvature(wi)+40] ||
	     %y > $Curvature(he) } break;
	if { [%W find withtag s] == "" } break;
	set dx [expr %x - $Curvature(oldX)]
        set Curvature(numin) [expr $Curvature(umin)+($Curvature(ud)*$dx)]
        set Curvature(numax) [expr $Curvature(umax)+($Curvature(ud)*$dx)]
	if { ($Curvature(numin) < $Curvature(tumin)) ||
	     ($Curvature(numax) > $Curvature(tumax)) } {
	    break;
	}
	$Curvature(ca) delete e
	$Curvature(ca) create line %x 0 %x $Curvature(he) -tag e
    }
    bind $ca <ButtonRelease-3> {
	%W delete s
	if { $Curvature(update) != "" } {
	    after cancel $Curvature(update)
	}
	set Curvature(update) \
	    [after 100 curvature_update $Curvature(numin) $Curvature(numax)]
    }

    # resize binding
    bind $ca <Configure> {
      if { (%w != [expr $Curvature(wi)+82]) ||
	   (%h != [expr $Curvature(he)+42]) } {
	  set Curvature(wi) [expr %w-82]
	  set Curvature(he) [expr %h-42]
	  upvar #0 _resizeHandle$ay(pca).$Curvature(w).rsh ar
	  if { [info exists ar] } {
	      set Curvature(sx) $ar(startX)
	      set Curvature(sy) $ar(startY)
	      set Curvature(ow) $ar(oldWidth)
	      set Curvature(oh) $ar(oldHeight)
	  }
	  if { $Curvature(update) != "" } {
	      after cancel $Curvature(update)
	  }
	  set Curvature(update)\
	      [after 100 curvature_update $Curvature(umin) $Curvature(umax)]
      }
    }

    after 200 "curvature_placeHandle $w.rsh $ca 0"

    set ::_resizeHandle$w.rsh(cw) 1
    set ::_resizeHandle$w.rsh(ch) 1

    plb_setwin $w ""

 return;
}
# curvature_update

# curvature_zoom
# realize zoom by mouse wheel
proc curvature_zoom { x d } {
    global Curvature
    if { $x < 40 } {return;set x 40}
    if { $x > [expr $Curvature(wi)+80] } {return;set x [expr $Curvature(wi)+80]}

    if { $d < 0 } {
	# zoom out
	set Curvature(numin) [curvature_getU [expr $x - $Curvature(wi)*2]]
	set Curvature(numax) [curvature_getU [expr $x + $Curvature(wi)*2]]
    } else {
	# zoom in
	set Curvature(numin) [curvature_getU [expr $x - $Curvature(wi)*0.25]]
	set Curvature(numax) [curvature_getU [expr $x + $Curvature(wi)*0.25]]
    }
    if { $Curvature(numin) < $Curvature(tumin) } {
	set Curvature(numin) $Curvature(tumin)
    }
    if { $Curvature(numax) > $Curvature(tumax) } {
	set Curvature(numax) $Curvature(tumax)
    }
    curvature_update $Curvature(numin) $Curvature(numax)
 return;
}
# curvature_zoom

proc curvature_print { } {
    set w $ay(pca).$Curvature(w)

    if { ![info exists tk::ensure_psenc_is_loaded] } {
	wrap::source tk8.4/lib/mkpsenc.tcl
    }

    $w postscript -file curvature.ps

 return;
}

proc curvature_placeHandle { w ca co } {
    if { $co < 5 } {
	if { [winfo exists $w] } {
	    resizeHandle:PlaceHandle $w $ca
	} else {
	    after 200 curvature_placeHandle $w $ca [expr $co + 1]
	}
    }
}

proc curvature_getU { x } {
    global Curvature
    set sx [expr ($x - 40) / $Curvature(wi).0]
    return [expr $Curvature(umin)+(($Curvature(umax)-$Curvature(umin))*$sx)]
}

# curvature_showvalues:
#  get coordinates of the control point that the mouse currently entered
#  and display them in a balloon / tool tip window
proc curvature_showvalues { {x ""} } {
    global ay Curvature

    set ca $ay(pca).$Curvature(w).ca

    # no balloon outside the canvas/diagram
    set cx [expr [winfo pointerx $ca] - [winfo rootx $ca]]
    set cy [expr [winfo pointery $ca] - [winfo rooty $ca]]
    if { $cx < 40 || $cx > [expr $Curvature(wi) + 40] ||
	 $cy < 0 || $cy > [expr $Curvature(he)] } {
	return;
    }

    if { $x == "" } {
	set x [expr [winfo pointerx $ca] - [winfo rootx $ca]]
	set u [curvature_getU $x]
	setMark [getPnt -eval $u]
	set txt [lindex [$ca gettags current] 1]
    } else {
	set txt [lindex $Curvature(pnts) [expr $x - 40]]
    }

    # create a balloon window at the mouse position
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
# curvature_showvalues

# switch log()-mode
proc curvature_togglelog { } {
    global Curvature
    if { $Curvature(log) } {
	set Curvature(log) 0
    } else {
	set Curvature(log) 1
    }
    curvature_update $Curvature(umin) $Curvature(umax)
 return;
}
# curvature_togglelog

# attach to custom menu
global ay
$ay(cm) add command -label "Add Curvature View to Object" -command {
    addTag NP Curvature; plb_update; plb_showprop 0
}

proc curvature_reset { args } {
    global Curvature
    set Curvature(wi) 200
    set Curvature(he) 200
    set Curvature(log) 0
    curvature_update
}

trace variable Curvature(reset) w curvature_reset

# EOF
