# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2003 by Randolf Schultz
# (randolf.schultz@gmail.com) and others.
#
# All rights reserved.
#
# See the file License for details.

# view.tcl - view window management


##############################
# viewSetType:
#  set the type of the view <w>
proc viewSetType { w type {redraw 1} } {
 global ay ayprefs tcl_platform

    undo save ViewType
    set togl $w.f3D.togl

    $togl mc
    switch $type {
	0 {
	    $togl setconf -type 0 -fromx 0.0 -fromy 0.0 -fromz 10.0\
		-tox 0.0 -toy 0.0 -toz 0.0 -upx 0.0 -upy 1.0 -upz 0.0\
		-redraw $redraw
	    viewTitle $w Front ""
	}
	1 {
	    $togl setconf -type 1 -fromx 10.0 -fromy 0.0 -fromz 0.0\
		-tox 0.0 -toy 0.0 -toz 0.0 -upx 0.0 -upy 1.0 -upz 0.0\
		-redraw $redraw
	    viewTitle $w Side ""
	}
	2 {
	    $togl setconf -type 2 -fromx 0.0 -fromy 10.0 -fromz 0.0\
		-tox 0.0 -toy 0.0 -toz 0.0 -upx 0.0 -upy 0.0 -upz -1.0\
		-redraw $redraw
	    viewTitle $w Top ""
	}
	3 {
	    $togl setconf -type 3 -fromx 0.0 -fromy 0 -fromz 15.0\
		-tox 0.0 -toy 0.0 -toz 0.0  -upx 0.0 -upy 1.0 -upz 0.0\
		-drawg 1 -grid 1 -drotx 45 -droty -30 -redraw $redraw
	    viewTitle $w Persp ""
	    viewSetGridIcon $w 1.0
	}
	4 {
	    $togl setconf -type 4 -fromx 0.0 -fromy 0.0 -fromz 10.0\
		-tox 0.0 -toy 0.0 -toz 0.0 -upx 0.0 -upy 1.0 -upz 0.0\
		-zoom 1.0 -redraw $redraw
	    viewTitle $w Trim ""
	}
    }
    # switch

    if { $redraw } {
	$togl render
    }

    set ay(cVType) $type

    if { [info exists ay(cVMMode)] } {
	viewSetMModeIcon $w $ay(cVMMode)
    } else {
	viewSetMModeIcon $w 0
    }

 return;
}
# viewSetType


##############################
# viewCycleType:
proc viewCycleType { w dir {recover 1} } {
    global ay

    set togl $w.f3D.togl

    $togl mc

    set oldfromx $ay(cVFromX)
    set oldfromy $ay(cVFromY)
    set oldfromz $ay(cVFromZ)

    set oldtox $ay(cVToX)
    set oldtoy $ay(cVToY)
    set oldtoz $ay(cVToZ)

    set oldzoom $ay(cVZoom)

    set oldtype $ay(cVType)

    set realign 0
    if { $ay(cVMMode) > 0 } {
	set recover 0
	set realign 1
    }

    update

    set type [expr $ay(cVType) + $dir]

    # wrap around
    if { $type < 0 } { set type 3 }
    # wrap around; omit 4 (trim view)
    if { $type > 3 } { set type 0 }
    # set new view type (also completely resets the camera)
    viewSetType $w $type 0

    if { $recover } {
	# recover old view aim-point and zoom
	if { $oldtype != 3 } {
	    if { ($oldtype == 0) && ($type == 1) } {
		# from front to side
		$togl setconf -fromx [expr {10.0 + $oldfromx}] \
		    -fromy $oldfromy -fromz $oldtoz \
		    -tox $oldtox -toy $oldtoy -toz $oldtoz \
		    -zoom $oldzoom -redraw 0
	    }
	    if { $oldtype == 1 && $type == 2 } {
		# from side to top
		$togl setconf -fromy [expr {10.0 + $oldfromy}] \
		    -fromz $oldfromz -fromx $oldtox \
		    -tox $oldtox -toy $oldtoy -toz $oldtoz \
		    -zoom $oldzoom -redraw 0
	    }
	    if { $oldtype == 2 && $type == 1 } {
		# from top to side
		$togl setconf -fromx [expr {10.0 + $oldfromx}] \
		    -fromy $oldtoy -fromz $oldfromz \
		    -tox $oldtox -toy $oldtoy -toz $oldtoz \
		    -zoom $oldzoom -redraw 0
	    }
	    if { $oldtype == 1 && $type == 0 } {
		# from side to front
		$togl setconf -fromx $oldtox -fromz [expr {10.0 + $oldfromz}] \
		    -fromy $oldfromy -tox $oldtox -toy $oldtoy -toz $oldtoz \
		    -zoom $oldzoom -redraw 0
	    }
	} else {
	    # from perspective to front or top
	    if { $type == 0 } {
		$togl setconf -fromx $oldtox -fromy $oldtoy \
		    -tox $oldtox -toy $oldtoy -toz $oldtoz \
		    -grid 0.0 -redraw 0
	    }
	    if { $type == 2 } {
		$togl setconf -fromx $oldtox -fromz $oldtoz \
		    -tox $oldtox -toy $oldtoy -toz $oldtoz \
		    -grid 0.0 -redraw 0
	    }
	}
	# if
    }
    # if

    if { $realign } {
	$togl align
    } else {
	$togl redraw
    }

 return;
}
# viewCycleType


##############################
# viewCycleDrawMode:
proc viewCycleDrawMode { w dir } {
    global ay ayprefs

    set togl $w.f3D.togl

    $togl mc

    set mode [expr $ay(cVDMode) + $dir]

    if { $ayprefs(CycleHiddenWire) } {
	set last 3
    } else {
	set last 2
    }

    # wrap around
    if { $mode < 0 } { set mode $last }
    if { $mode > $last } { set mode 0 }
    # set new drawing mode
    $togl setconf -shade $mode
    # set icon
    viewSetDModeIcon $w $mode

    $togl redraw

 return;
}
# viewCycleDrawMode


##############################
# viewRender:
proc viewRender { w type } {
    global env ayprefs ay tcl_platform

    set togl $w.f3D.togl

    if { $ayprefs(ShadowMaps) < 1 } {
	tmpGet $ayprefs(TmpDir) tmpfile .rib

	if { $tcl_platform(platform) == "windows" } {
	    # Windows sucks big time!
	    regsub -all {\\} $tmpfile {/} tmpfile
	}
	set imagename [file rootname ${tmpfile}].tif
	if { $type < 2 } {
	    if { $ayprefs(RenderMode) == 0 } {
		$togl wrib -file $tmpfile -image $imagename -temp
	    } else {
		$togl wrib -file $tmpfile -temp
	    }
	} else {
	    # render to image file
	    set ribname [io_getRIBName]
	    set imagename [lindex $ribname 1]
	    ayError 4 "viewRender" "Rendering to \"$imagename\"..."
	    $togl wrib -file $tmpfile -image $imagename -temp -rtf
	}

	lappend ay(tmpfiles) [list $tmpfile]
    } else {
	set ribname [io_getRIBName]
	set tmpfile [lindex $ribname 0]
	set imagename [lindex $ribname 1]
	if { $type < 2 } {
	    if { $ayprefs(RenderMode) == 0 } {
		$togl wrib -file $tmpfile -image $imagename -temp
	    } else {
		$togl wrib -file $tmpfile -temp
	    }
	} else {
	    # render to image file
	    ayError 4 "viewRender" "Rendering to \"$imagename\"..."
	    $togl wrib -file $tmpfile -image $imagename -temp -rtf
	}
    }
    # if

    set renderui 0
    if { $type == 0 } {
        if { $ayprefs(RenderUI) == 1 } { set renderui 1 }
	regsub -all {%s} $ayprefs(Render) "\"$tmpfile\"" command
	set pt $ayprefs(RenderPT)
    }
    if { $type == 1 } {
        if { $ayprefs(QRenderUI) == 1 } { set renderui 1 }
	regsub -all {%s} $ayprefs(QRender) "\"$tmpfile\"" command
	set pt $ayprefs(QRenderPT)
    }
    if { $type == 2 } {
        if { $ayprefs(FRenderUI) == 1 } { set renderui 1 }
	regsub -all {%s} $ayprefs(FRender) "\"$tmpfile\"" command
	set pt $ayprefs(FRenderPT)
    }

    if { $renderui != 1} {
	set command2 "exec "
	append command2 $command
	append command2 " &"
	eval [subst "$command2"]
    } else {
	runRenderer $w "$command" "$pt"
    }
    # if

    update
    tmp_clean 0

 return;
}
# viewRender


##############################
# viewUPos:
# get the positions of all views
# put them in the C context view_info
proc viewUPos { } {
 global ay ayprefs tcl_platform

    update idletasks

    foreach w $ay(views) {

	if { ($tcl_platform(platform) != "windows" ) &&\
	    ($ayprefs(TwmCompat) != 1) } {
	    set x [winfo rootx $w]
	    set y [winfo rooty $w]
	} else {
	    set oldgeom [wm geom [winfo toplevel $w]]
	    regexp {([0-9]+)?x?([0-9]+)?(\+|\-)?([0-9]+)?(\+|\-)?([0-9]+)?} $oldgeom blurb width height blurb2 x blurb3 y
	}

	set isicon 0
	set state [wm state [winfo toplevel $w]]
	if {  ($state == "iconic") || ($state == "withdrawn") } {
	    set isicon 1
	}
	$w.f3D.togl mc
	$w.f3D.togl setconf -ico $isicon -pos $x $y
	$ay(currentView) mc
    }
    # foreach

 return;
}
# viewUPos


##############################
# viewTitle:
# set title bar of view window w
# type = "" -> no change, action = "" -> no change
proc viewTitle { w type action } {
    global ay AYWITHAQUA

    if { [string first ".view" $w] != 0 } {
	# $w does not start with ".view" => view is internal, no title to set
	return;
    }

    # guard against calling with sub-widget
    set w [winfo toplevel $w]

    set oldname [wm title $w]

    set vname ""
    set oldtype ""
    set oldaction ""

    catch {scan $oldname "%s - %s - %s" vname oldtype oldaction}

    scan $w ".view%d" number

    set newvname [format "View%d" $number]

    if { $type != "" } {
	set oldtype $type

	if { (! $AYWITHAQUA) || ([winfo toplevel $w] != $w) } {
	    set confm $w.fMenu.c.m
	} else {
	    set confm $w.menubar.mconf
	}
    }
    # if
    if { $action != "" } { set oldaction $action }
    set newname [format "%s - %s - %s" $newvname $oldtype $oldaction]
    wm title $w $newname
    update idletasks

 return;
}
# viewTitle


##############################
# viewSetFOV:
#  set the Field Of View of a view
proc viewSetFOV { view } {
 global ay ayprefs

    winAutoFocusOff

    set w .setFov
    set t "Set FOV"
    winDialog $w $t

    set f [frame $w.f1]
    pack $f -in $w -side top -fill x

    set ay(FOV) $ay(cVFOV)

    set ay(bca) $w.f2.bca
    set ay(bok) $w.f2.bok

    # disable instant apply bindings
    set oldiapplydisable $ay(iapplydisable)
    set ay(iapplydisable) 1

    if { $ayprefs(FixDialogTitles) == 1 } {
	addText $f ay $t
    }

    addParam $f ay FOV

    set ay(iapplydisable) $oldiapplydisable

    set f [frame $w.f2]
    button $f.bok -text "Ok" -pady $ay(pady) -width 5 -command "\
        global ay;\
	$view mc;\
	undo save SetFOV;\
	$view setconf -fovx \$ay(FOV);\
	update;\
	grab release $w;\
	focus $view;\
	destroy .setFov"

    button $f.bca -text "Cancel" -pady $ay(pady) -width 5 -command "\
	grab release $w;\
	focus $view;\
	destroy $w"

    pack $f.bok $f.bca -in $f -side left -fill x -expand yes
    pack $f -in $w -side bottom -fill x

    # center dialog on screen or inside the view (if external)
    set t [winfo toplevel $view]
    if { $t != "." } {
	wm transient $w $t
	winCenter $w $t
    } else {
	winCenter $w
    }

    if { $ay(ws) != "Aqua" } {
	grab $w
    }

    focus $w.f1.fFOV.e
    tkwait window $w

    winAutoFocusOn

 return;
}
# viewSetFOV


##############################
# viewSetGrid:
proc viewSetGrid { view } {
 global ay ayprefs

    winAutoFocusOff

    set w .setGrid
    set t "Set GridSize"
    winDialog $w $t

    set f [frame $w.f1]
    pack $f -in $w -side top -fill x

    set ay(GridSize) $ay(cVGridSize)

    set ay(bca) $w.f2.bca
    set ay(bok) $w.f2.bok

    # disable instant apply bindings
    set oldiapplydisable $ay(iapplydisable)
    set ay(iapplydisable) 1

    if { $ayprefs(FixDialogTitles) == 1 } {
	addText $f ay $t
    }

    addParam $f ay GridSize [list 0.001 0.01 0.1 0.25 0.5 1 10]

    set ay(iapplydisable) $oldiapplydisable

    set f [frame $w.f2]
    button $f.bok -text "Ok" -pady $ay(pady) -width 5 -command "\
	global ay;\
	$view mc;\
	undo save SetGrid;\
        if \{ \$ay(GridSize) != 0.0 \} \{\
	  $view setconf -drawg 1 -ugrid 1 -grid \$ay(GridSize);\
        \} else \{\
	  $view setconf -grid \$ay(GridSize);\
        \};\
        set w \[winfo parent \[winfo parent $view\]\];\
	viewSetGridIcon \$w \$ay(GridSize);\
	update;\
	grab release $w;\
	focus $view;\
	destroy $w"

    button $f.bca -text "Cancel" -pady $ay(pady) -width 5 -command "\
	grab release $w;\
	focus $view;\
	destroy $w"

    pack $f.bok $f.bca -in $f -side left -fill x -expand yes
    pack $f -in $w -side bottom -fill x

    # center dialog on screen or inside the view (if external)
    set t [winfo toplevel $view]
    if { $t != "." } {
	wm transient $w $t
	winCenter $w $t
    } else {
	winCenter $w
    }

    if { $ay(ws) != "Aqua" } {
	grab $w
    }

    focus $w.f1.fGridSize.e
    tkwait window $w

    winAutoFocusOn

 return;
}
# viewSetGrid


##############################
# viewOpen:
proc viewOpen { width height {establish_bindings 1} {internal_view 0} } {
    global ay ayprefs

    set ay(viewlock) 1

    # calculate view name
    if { [llength $ay(views)] == 0 } {
	set w .view1
	set lastid 0
    } else {
	set lastname [lindex $ay(views) end]
	if { [string first ".view" $lastname] == 0 } {
	    scan $lastname ".view%d" lastid
	} else {
	    if { [string first ".fv" $lastname] == 0 } {
		scan $lastname ".fv.fViews.fview%d" lastid
	    } else {
		set lastid 3
	    }
	}
	set w .view[expr $lastid + 1]
    }

    # create internal view frame or toplevel window?
    if { $internal_view == 0 } {
	# toplevel
	catch {destroy $w}
	if { $ay(truecolor) == 1 } {
	    toplevel $w -class Ayam -visual truecolor
	} else {
	    toplevel $w -class Ayam
	}

	scan $w ".%s" name
	wm iconname $w $name
    } else {
	# internal
	scan $w ".%s" name
	if { $lastid < 2 } {
	    set w [frame .fv.fViews.f$name -takefocus 1 -highlightthickness 1]
	} else {
	    # the third internal view is a little bit different
	    set w .fu.fMain.fview3
	}
    }

    # create view menu
    frame $w.fMenu -bd 2 -relief raised
    vmenu_open $w
    pack $w.fMenu -side top -fill x

    # create the 3D widget
    frame $w.f3D -takefocus 0

    if { $ayprefs(AddViewParams) != "" } {
	eval [subst "togl $w.f3D.togl -rgba true -double true -depth true\
		-ident $name -width $width -height $height\
		$ayprefs(AddViewParams)"]
    } else {
	togl $w.f3D.togl -rgba true -double true -depth true\
		-ident $name -width $width -height $height
    }

    # realize the GUI
    pack $w.f3D -in $w -fill both -expand yes
    pack $w.f3D.togl -in $w.f3D -fill both -expand yes
    update

    # add to list of views
    lappend ay(views) $w

    $w.f3D.togl mc
    set ay(currentView) $w.f3D.togl

    $w.f3D.togl setconf -name $name

    # set mark
    if {$ayprefs(GlobalMark)} {
	$w.f3D.togl setconf -panx 0
    }

    if { $internal_view == 0 } {
	viewTitle $w Front Pick

	wm protocol $w WM_DELETE_WINDOW \
      "after 100 \{viewUnBind $w;viewClose $w;global ay;set ay(ul) root:0;uS\}"
    }
    # if

    if { $establish_bindings == 1 } {
	viewBind $w
    } else {
	$w configure -cursor watch
    }

    bind $w <Configure> {
	global ay
	set lb $ay(plb)
	set tmp ""
	set tmp [$lb curselection]
	if { ($tmp != "") && ([$lb get $tmp] == "ViewAttrib") } {
	    getProp
	    plb_update
	}
    }
    # bind

    if { $internal_view == 1 } {
	bind $w.f3D.togl <FocusIn> {
	    focus -force [winfo parent [winfo parent %W]]
	}
	# bind
    }

    # accept objects from the tree view to be dropped into the view window
    DropSite::register $w.f3D.togl -dropcmd viewDrop\
	    -droptypes {TREE_NODE {copy {}} IMAGE { copy {}}}

    # avoid clash of normal action hot keys with menu mnemonics
    if { $internal_view == 1 } {
	bindtags $w [list all Frame $w]
    } else {
	set ayclass [lindex [bindtags $w] 1]
	bindtags $w [list ayview all $ayclass $w]
	bindtags $w.f3D.togl [list all $w.f3D.togl Togl $w]
    }

    # switch modelling mode menu to an own (widget dependent) image
    viewSetMModeIcon $w 0

    set ay(viewlock) 0

 return;
}
# viewOpen


###############################
# viewDrop:
# helper procedure that calls the togl callback for the drop
# operation and throws all the useless arguments away
proc viewDrop { w tree dropx dropy currentoperation datatype data } {

    #puts "viewDrop: $currentoperation $datatype $data"

    $w drop
    after 100 "focus -force $w"

 return;
}
# viewDrop


##############################
# viewPostMenu:
# helper that posts a view menu, taking care
# of the focus (as ::tk::TraverseToMenu does)
proc viewPostMenu { m } {
    set p [winfo parent $m]
    $m post [winfo rootx $p] [expr [winfo rooty $p]+[winfo height $p]]
    set w [winfo parent [winfo parent $p]]
    bind $m <Unmap> "focus $w"
    focus $m
}
# viewPostMenu


##############################
# viewBindMenus:
#  bind the menus of an internal view window
proc viewBindMenus { w } {
    global aymainshortcuts
    # arrange for our bindings to override the all-tag bindings
    bindtags $w [list $w all Frame]
    # bind View-menu
    bind $w <$aymainshortcuts(MenuMod)-v> "viewPostMenu %W.fMenu.v.m;break"
    $w.fMenu.v conf -underline 0
    # bind Type-menu
    bind $w <$aymainshortcuts(MenuMod)-y> "viewPostMenu %W.fMenu.t.m;break"
    $w.fMenu.t conf -underline 1
    # bind Configure-menu
    bind $w <$aymainshortcuts(MenuMod)-o> "viewPostMenu %W.fMenu.c.m;break"
    $w.fMenu.c conf -underline 1
    # bind Grid-menu
    bind $w <$aymainshortcuts(MenuMod)-g> "viewPostMenu %W.fMenu.g.m;break"
}
# viewBindMenus


##############################
# viewBind:
# establish all keyboard and mouse bindings for view window w
proc viewBind { w } {
    global ayviewshortcuts

    # internal bindings
    bind $w <Enter> {
	global ay ayprefs
	if { $ay(viewlock) != 1 } {
	    set ay(viewlock) 1

	    if { [string first ".view" %W] == 0 } {
		set w [winfo toplevel %W]
	    } else {
		set w %W
	    }
	    $w.f3D.togl mc
	    set ay(currentView) $w.f3D.togl
	    $w.f3D.togl configure -cursor left_ptr

	    set ay(viewlock) 0
	}
	#if

	if { $ayprefs(AutoFocus) == 1 } {
	    if { [string first ".view" %W] == 0 } {
		focus [winfo toplevel %W].f3D.togl
	    } else {
		focus %W
	    }
	}
	break;
    }
    #bind

    bind $w <$ayviewshortcuts(RotMod)-Enter> {
	global ay ayprefs ayviewshortcuts
	if { $ay(viewlock) != 1 } {
	    set ay(viewlock) 1

	    if { [string first ".view" %W] == 0 } {
		set w [winfo toplevel %W]
	    } else {
		set w %W
	    }
	    $w.f3D.togl mc
	    set ay(currentView) $w.f3D.togl
	    $w.f3D.togl configure -cursor exchange

	    set ay(viewlock) 0
	}
	#if

	if { $ayprefs(AutoFocus) == 1 } {
	    if { [string first ".view" %W] == 0 } {
		focus [winfo toplevel %W].f3D.togl
	    } else {
		focus %W
	    }
	}
	break;
    }
    #bind

    bind $w <$ayviewshortcuts(ZoomRMod)-Enter> {
	global ay ayprefs ayviewshortcuts
	if { $ay(viewlock) != 1 } {
	    set ay(viewlock) 1

	    if { [string first ".view" %W] == 0 } {
		set w [winfo toplevel %W]
	    } else {
		set w %W
	    }
	    $w.f3D.togl mc
	    set ay(currentView) $w.f3D.togl
	    $w.f3D.togl configure -cursor sizing

	    set ay(viewlock) 0
	}
	#if

	if { $ayprefs(AutoFocus) == 1 } {
	    if { [string first ".view" %W] == 0 } {
		focus [winfo toplevel %W].f3D.togl
	    } else {
		focus %W
	    }
	}
	break;
    }
    #bind

    # bind menu shortcuts
    shortcut_view $w

    # bind actions
    shortcut_viewactions $w

    # bind default action
    actionClear $w.f3D.togl

    # bind menus
    if { [string first ".view" $w] != 0 } {
	viewBindMenus $w
    }

    $w configure -cursor {}
    $w.f3D.togl configure -cursor {}

 return;
}
# viewBind


##############################
# viewUnBind:
# remove all keyboard and mouse bindings from view window w
proc viewUnBind { w } {

    foreach v [ bind $w ] { bind $w $v "" }
    foreach v [ bind $w.f3D.togl ] { bind $w.f3D.togl $v "" }
    update
    $w configure -cursor watch
    $w.f3D.togl configure -cursor watch

 return;
}
# viewUnBind


##############################
# viewCloseAll:
proc viewCloseAll { } {
  global ay

  foreach view $ay(views) {
      viewClose $view
  }

 return;
}
# viewCloseAll


##############################
# viewClose:
proc viewClose { w } {
  global ay ayviewshortcuts

    if { [string first ".view" $w] != 0 } {
	# $w does not start with ".view" => view is internal, can not close
	return;
    }

    # first remove bindings that could accidentally fire while closing
    bind $w <Enter> ""
    bind $w <Motion> ""
    bind $w <$ayviewshortcuts(RotMod)-Motion> ""
    bind $w <$ayviewshortcuts(ZoomRMod)-Motion> ""

    # now delete the view from the view list
    set ay(currentView) ""
    set temp ""
    foreach view $ay(views) {
	if { $view != $w } {
	    lappend temp $view
	    set ay(currentView) ${view}.f3D.togl
	}
    }

    set ay(views) $temp

    # make a different view current
    catch {$ay(currentView) mc}

    # finally delete the window
    destroy ${w}.f3D.togl
    destroy $w

    # clear undo buffer
    undo clear

 return;
}
# viewClose


##############################
# setCameraAttr:
proc setCameraAttr { } {
    global ay CameraData
    set cw $ay(currentView)

    set t ""
    catch {getType t}

    # do not switch current view for Camera objects
    if { $t == "View" } {
	$CameraData(togl) mc
    }

    setProp

    if { $t == "View" } {
	$ay(currentView) mc
    }

 return;
}
# setCameraAttr


##############################
# setViewAttr:
proc setViewAttr { } {
 global ay ViewAttribData pclip_reset

    set cw $ay(currentView)

    set togl $ViewAttribData(togl)

    $togl mc

    if { $ViewAttribData(Width) != $pclip_reset(Width) ||
	 $ViewAttribData(Height) != $pclip_reset(Height) } {

	if { [string first ".view" $togl] != 0 } {
	    # XXXX insert special handling for internal views
	    set ViewAttribData(Width) $pclip_reset(Width)
	    set ViewAttribData(Height) $pclip_reset(Height)
	    ayError 2 "setViewAttr" "Can not resize internal views!"
	} else {
	    $togl configure -width $ViewAttribData(Width)\
		-height $ViewAttribData(Height)
	    update
	}
    }
    # if

    if { $ViewAttribData(Type) != $pclip_reset(Type) } {
	#    setupActionKeys $w clear
	set typename [lindex $ay(viewtypenames) $ViewAttribData(Type)]
	viewTitle $togl $typename Pick
    }

    viewSetGridIcon [winfo parent [winfo parent $togl]]\
	$ViewAttribData(GridSize)
    viewSetDModeIcon [winfo parent [winfo parent $togl]]\
	$ViewAttribData(DrawingMode)
    viewSetMModeIcon [winfo parent [winfo parent $togl]]\
	$ViewAttribData(ModellingMode)

    setProp

    $ay(currentView) mc

 return;
}
# setViewAttr


##############################
# viewRepairTitle:
#  after undo/redo, set correct view title
proc viewRepairTitle { w type } {
 global ay ViewAttribData

    set typename [lindex $ay(viewtypenames) $type]

    viewTitle .${w} $typename ""

 return;
}
# viewRepairTitle


##############################
# viewSetGridIcon:
#  set correct grid icon according to gridsize
proc viewSetGridIcon { w gridsize } {
 global ay tcl_platform AYWITHAQUA

    if { [string first ".view" $w] == 0 } {
	# guard against calling with sub-widget
	set w [winfo toplevel $w]
    }

    if { (! $AYWITHAQUA) || ([winfo toplevel $w] != $w) } {
	set menubar 0
	set m fMenu.g
	set conf "$w.$m configure"
    } else {
	# external views on Aqua
	set menubar 1
	set m menubar.mgrid
	set conf "$w.menubar entryconfigure 6"
    }

    if { $gridsize == 0.1 } {
	eval "$conf -image ay_Grid0.1_img"
	if {$menubar} { eval "$conf -image {} -label \"Grid 0.1\"" }
	} else {
	    if { $gridsize == 0.25 } {
		eval "$conf -image ay_Grid0.25_img"
	       if {$menubar} { eval "$conf -image {} -label \"Grid 0.25\"" }
	    } else {
		if { $gridsize == 0.5 } {
		    eval "$conf -image ay_Grid0.5_img"
		if {$menubar} { eval "$conf -image {} -label \"Grid 0.5\"" }
		} else {
		    if { $gridsize == 1.0 } {
			eval "$conf -image ay_Grid1.0_img"
		if {$menubar} { eval "$conf -image {} -label \"Grid 1.0\"" }
		    } else {
			if { $gridsize == 0.0 } {
			    eval "$conf -image ay_Grid_img"
		    if {$menubar} { eval "$conf -image {} -label \"No Grid\"" }
			} else {
			    # non-standard size, see if we have a matching icon
			    set iconName ay_Grid
			    append iconName $gridsize
			    append iconName _img
			    if { [lsearch [image names] $iconName] != -1 } {
				eval "$conf -image $iconName"
			    } else {
				eval "$conf -image ay_GridX_img"
			    }
                  if {$menubar} { eval "$conf -image {} -label \"Grid X\"" }
			}
		    }
		}
	    }
	}

 return;
}
# viewSetGridIcon


##############################
# viewSetDModeIcon:
#  set correct drawing mode icon according to mode
proc viewSetDModeIcon { w mode } {
 global ay AYWITHAQUA

    if { [string first ".view" $w] == 0 } {
	# guard against calling with sub-widget
	set w [winfo toplevel $w]
    }

    if { (! $AYWITHAQUA) || ([winfo toplevel $w] != $w) } {
	set menubar 0
	set m fMenu.dm
	set conf "$w.$m configure"
    } else {
	# external views on Aqua
	set menubar 1
	set m menubar.dm
	set conf "$w.menubar entryconfigure 5"
    }
    switch $mode {
	0 {
	    eval "$conf -image ay_DMDraw_img"
	    if {$menubar} { eval "$conf -image {} -label \"Draw\"" }
	}
	1 {
	    eval "$conf -image ay_DMShade_img"
	    if {$menubar} { eval "$conf -image {} -label \"Shade\"" }
	}
	2 {
	    eval "$conf -image ay_DMShadeDraw_img"
	    if {$menubar} { eval "$conf -image {} -label \"Shade&Draw\"" }
	}
	3 {
	    eval "$conf -image ay_DMHiddenWire_img"
	    if {$menubar} { eval "$conf -image {} -label \"HiddenWire\"" }
	}
    }

 return;
}
# viewSetDModeIcon


##############################
# viewSetMModeIcon:
#  set correct modelling mode (global/local) icon according to mode
proc viewSetMModeIcon { w mode } {
    global ay AYWITHAQUA

    set i [string first "view" $w]
    set vimage ay_[string range $w $i end]_img

    if { [string first ".view" $w] == 0 } {
	# guard against calling with sub-widget
	set w [winfo toplevel $w]
    }

    if { (! $AYWITHAQUA) || ([winfo toplevel $w] != $w) } {

	if { $mode == 0 } {
	    set image ay_MMGlob_img
	} else {
	    if { $mode == 1 } {
		set image ay_MMLocLev_img
	    } else {
		if { $mode == 2 } {
		    set image ay_MMLocObj_img
		}
	    }
	}

	catch {	image create photo $vimage -w 25 -h 25 }
	$vimage copy $image

	# add red dot for pnts-trafo mode?
	if { $ay(cVPnts) } {
	    set col \#af0000
	    $vimage put $col -to 20 1 24 5
	}

	# add view type as letter (FSTP) in the upper left corner
	if { [info exists ay(cVType)] } {
	    set type $ay(cVType)
	} else {
	    set type 0
	}
	set col \#000000
	switch $type {
	    0 {
		# F
		$vimage put $col -to 2 1 8 3
		$vimage put $col -to 2 1 4 11
		$vimage put $col -to 2 5 6 7
	    }
	    1 {
		# S
		$vimage put $col -to 3 1 7 3
		$vimage put $col -to 3 5 7 7
		$vimage put $col -to 3 9 7 11
		$vimage put $col -to 2 2 4 6
		$vimage put $col -to 6 6 8 10
		$vimage put $col -to 7 2 8 4
		$vimage put $col -to 2 8 3 10
	    }
	    2 {
		# T
		$vimage put $col -to 1 1 9 3
		$vimage put $col -to 4 3 6 11
	    }
	    3 {
		# P
		$vimage put $col -to 2 1 7 3
		$vimage put $col -to 2 5 7 7
		$vimage put $col -to 6 2 8 6
		$vimage put $col -to 2 1 4 11
	    }
	}
	# switch
	set m fMenu.mm
	set conf "$w.$m configure"

	eval "$conf -image $vimage"
    } else {
	# external views on Aqua
	set conf "$w.menubar entryconfigure 4"

	if { [info exists ay(cVType)] } {
	    set type $ay(cVType)
	} else {
	    set type 0
	}
	set lab ""
	switch $type {
	    0 {
		set lab "F"
	    }
	    1 {
		set lab "S"
	    }
	    2 {
		set lab "T"
	    }
	    3 {
		set lab "P"
	    }
	}

	switch $mode {
	    0 {
		append lab "-Glob"
	    }
	    1 {
		append lab "-LocLev"
	    }
	    2 {
		append lab "-LocObj"
	    }
	}

	eval "$conf -label $lab"
    }
    # if

 return;
}
# viewSetMModeIcon


##############################
# viewSetMAIcon:
#  set modelling action icon
proc viewSetMAIcon { w image balloon } {
 global ay tcl_platform AYWITHAQUA

    # external views on Aqua do not display any such icon
    if { (! $AYWITHAQUA) || ([string first ".view" $w] != 0) } {

	set m fMenu.a

	set w [winfo parent [winfo parent $w]]

	set conf "$w.$m configure"
	eval "$conf -image $image"
	if { $balloon != "" } {
	    eval [subst "balloon_set $w.$m $balloon"]
	} else {
	    eval [subst "balloon_clear $w.$m"]
	}
    }

 return;
}
# viewSetMAIcon


##############################
# viewMouseToCurrent:
#  set current view to the view that has the focus;
#  used after startup and loading of scenes
proc viewMouseToCurrent { } {
 global ay

    set focused [focus -displayof .]

    if { $focused != "" } {
 	foreach view $ay(views) {
 	    if { [string compare $view $focused] == 0 } {
 		${view}.f3D.togl mc
		set ay(currentView) ${view}.f3D.togl
 	    }
 	}
    }

 return;
}
# viewMouseToCurrent


##############################
# viewToggleDMode:
#  toggle drawing mode from shade (Shade _or_ ShadeAndDraw) to draw
proc viewToggleDMode { w } {
    global ay

    set togl $w.f3D.togl

    if { [string first ".view" $w] == 0 } {
	set w [winfo toplevel $togl]
    }

    if { $ay(cVDMode) > 0 } {
	$togl setconf -shade 0
	viewSetDModeIcon $w 0
	set ay(cVDMode) 0
    } else {
	$togl setconf -shade 1
	viewSetDModeIcon $w 1
	set ay(cVDMode) 1
    }

 return;
}
# viewToggleDMode


##############################
# viewCycleMMode:
#  cycle modelling mode (> global > local(lev) > local(obj) >)
proc viewCycleMMode { w } {
    global ay

    set togl $w.f3D.togl

    if { [string first ".view" $w] == 0 } {
	set w [winfo toplevel $togl]
    }

    if { $ay(cVMMode) == 0 } {
	$togl setconf -local 1
        $togl align
	viewSetMModeIcon $w 1
	set ay(cVMMode) 1
    } else {
	if { $ay(cVMMode) == 1 } {
	    $togl setconf -local 2
	    $togl align
	    viewSetMModeIcon $w 2
	    set ay(cVMMode) 2
	} else {
	    if { $ay(cVMMode) == 2 } {
		$togl setconf -local 0
		viewSetType $w $ay(cVType) 1
		viewSetMModeIcon $w 0
		set ay(cVMMode) 0
	    }
	}
    }

 return;
}
# viewCycleMMode


##############################
# viewCycleGrid:
#  cycle grid size
proc viewCycleGrid { w cycle dir } {
    global ay ayprefs

    set togl $w.f3D.togl

    if { [string first ".view" $w] == 0 } {
	set w [winfo toplevel $togl]
    }

    if { $cycle } {
	set index [lsearch $ayprefs(Grids) $ay(cVGridSize)]

	if { $index == -1 } {
	    if { $dir == -1 } {
		set index [llength $ayprefs(Grids)]
		incr index -1
	    } else {
		set index 0
	    }
	} else {
	    if { $dir == -1 } {
		incr index -1
		if { $index < 0 } {
		    set index [llength $ayprefs(Grids)]
		    incr index -1
		}
	    } else {
		incr index
		if { $index >= [llength $ayprefs(Grids)] } {
		    set index 0
		}
	    }
	}

	set grid [lindex $ayprefs(Grids) $index]

    } else {
	if { $dir == -1 } {
	    set grid [expr $ay(cVGridSize) * 0.5]
	} else {
	    set grid [expr $ay(cVGridSize) * 2.0]
	}
    }

    $w.f3D.togl setconf -grid $grid

    viewSetGridIcon $w $grid

    set ay(cVGridSize) $grid

 return;
}
# viewCycleGrid


##############################
# viewSetPMode:
#  toggle object/point transformation mode
proc viewSetPMode { w on } {
    global ay AYWITHAQUA

    set togl $w.f3D.togl

    if { [string first ".view" $w] == 0 } {
	set w [winfo toplevel $togl]
    }

    if { $on } {
	$togl setconf -pnts 1
	set ay(cVPnts) 1
	set col \#af0000
    } else {
	$togl setconf -pnts 0
	set ay(cVPnts) 0
	set col \#bdbdbd

    }

    if { (! $AYWITHAQUA) || ([string first ".view" $w] != 0) } {
	set m fMenu.mm

	set image [$w.$m cget -image]
	$image put $col -to 20 1 24 5
    }

 return;
}
# viewSetPMode


##############################
# viewSetBGImage:
proc viewSetBGImage { view } {
    global ay ayprefs

    $view mc
    set ay(ImageFile) $ay(cVBGImage)
    update

    winAutoFocusOff

    set w .setBGI
    set t "Set BGImage"
    winDialog $w $t

    set f [frame $w.f1]
    pack $f -in $w -side top -fill x

    set ay(bca) $w.f2.bca
    set ay(bok) $w.f2.bok

    if { $ayprefs(FixDialogTitles) == 1 } {
	addText $f ay $t
    }

    addFileT $f ay ImageFile \
	{ {"TIF" ".tif"} {"TIFF" ".tiff"} {"All files" *} }

    set f [frame $w.f2]
    button $f.bok -text "Ok" -pady $ay(pady) -width 15 -command "\
	global ay;\
	$view mc;\
	$view setconf -bgimage \$ay(ImageFile);\
	if { \$ay(cVDrawBG) == 0 } {\
	 set ay(cVDrawBG) 1;\
	 $view setconf -dbg \$ay(cVDrawBG);\
        };\
        set ay(sc) 1;\
	update;\
	grab release $w;\
	focus $view;\
	destroy $w"

    button $f.bca -text "Cancel" -pady $ay(pady) -width 15 -command "\
	grab release $w;\
	focus $view;\
	destroy $w"

    pack $f.bok $f.bca -in $f -side left -fill x -expand yes
    pack $f -in $w -side bottom -fill x

    # Esc-key && close via window decoration == Cancel button
    bind $w <Escape> "$f.bca invoke"
    wm protocol $w WM_DELETE_WINDOW "$f.bca invoke"

    # center dialog on screen or inside the view (if external)
    set t [winfo toplevel $view]
    if { $t != "." } {
	wm transient $w $t
	winCenter $w $t
    } else {
	winCenter $w
    }

    if { $ay(ws) != "Aqua" } {
	grab $w
    }

    focus $w.f1.fImageFile.e
    tkwait window $w

    winAutoFocusOn

 return;
}
# viewSetBGImage


##############################
# viewPan:
proc viewPan { togl dir } {
 global ayprefs

    $togl mc
    $togl movevac -start 0 0

    set l [$togl configure -width]
    set w [lindex $l 4]
    set l [$togl configure -height]
    set h [lindex $l 4]

    set d $ayprefs(PanDist)
    if { $d < 0 } {
       	switch $dir {
	    0 { $togl setconf -undokb ViewPanX -panx [expr $w/$d] }
	    1 { $togl setconf -undokb ViewPanX -panx [expr -$w/$d] }
	    2 { $togl setconf -undokb ViewPanY -pany [expr $h/$d] }
	    3 { $togl setconf -undokb ViewPanY -pany [expr -$h/$d] }
	}
    } else {
	switch $dir {
	    0 { $togl setconf -undokb ViewPanX -panx [expr -$d] }
	    1 { $togl setconf -undokb ViewPanX -panx [expr $d] }
	    2 { $togl setconf -undokb ViewPanY -pany [expr -$d] }
	    3 { $togl setconf -undokb ViewPanY -pany [expr $d] }
	}
    }

 return;
}
# viewPan


##############################
# viewZoom:
proc viewZoom { w d } {
 global ay ayprefs

    if { [string first ".view" $w] != 0 } {
	# internal view
	set togl $w
	if { [string first ".togl" $togl] == 0 } {
	    return;
	}
    } else {
	# external view
	set togl [winfo toplevel $w].f3D.togl
    }

    $togl mc
    if { $ay(cVUndo) } {
	if { $d < 0.0 } {
	    $togl setconf -undokb ZoomView -dzoom [expr 1.0/$ayprefs(WheelZoom)]
	} else {
	    $togl setconf -undokb ZoomView -dzoom $ayprefs(WheelZoom)
	}
    } else {
	if { $d < 0.0 } {
	    $togl setconf -dzoom [expr 1.0/$ayprefs(WheelZoom)]
	} else {
	    $togl setconf -dzoom $ayprefs(WheelZoom)
	}
    }
    update
    $togl reshape
    $togl render

 return;
}


##############################
# setMark:
proc setMark { px {y 0} {z 0} } {
 global ay

    if { $ay(views) == "" } {
	return;
    }

    if { [llength $px] >= 2 } {
	set x [lindex $px 0]
	set y [lindex $px 1]
	set z [lindex $px 2]
    } else {
	set x $px
    }
    set w [lindex $ay(views) 0]
    ${w}.f3D.togl setconf -smark $x $y $z

 return;
}
# setMark


##############################
# warpMouse:
# warp mouse pointer to new position
proc warpMouse { dx dy } {
 global ay

    set w [winfo toplevel $ay(currentView)]

    set x [expr [winfo pointerx $w] - [winfo rootx $w]]
    set y [expr [winfo pointery $w] - [winfo rooty $w]]

    set newx [expr $x + $dx]
    set newy [expr $y + $dy]

    # never warp out of current view window
    set ww $ay(currentView)
    set x0 [expr [winfo rootx $ww] - [winfo rootx $w]]
    set xn [expr $x0 + [winfo width $ww]]
    set y0 [expr [winfo rooty $ww] - [winfo rooty $w]]
    set yn [expr $y0 + [winfo height $ww]]
    if { $newx < $x0 || $newx > $xn || $newy < $y0 || $newy > $yn } {
	return;
    }

    # redirect all events to the tree widget so that no view receives
    # stray <Motion> events potentially caused by the warp
    grab $ay(tree)
    # do the warp
    event generate $w <Motion> -warp 1 -x $newx -y $newy
    # restore event processing
    grab release $ay(tree)

 return;
}
# warpMouse


# some code, that has to be executed in global context when this
# module is sourced the first time (on application startup):

# objects of type View have just two editable properties:
set View_props { Camera ViewAttrib }

# create Camera-UI
array set Camera {
arr   CameraData
sproc setCameraAttr
gproc ""
w     fCamera
}

set w [frame $ay(pca).$Camera(w)]

addText $w e2 "From:"
addParam $w CameraData From_X
addParam $w CameraData From_Y
addParam $w CameraData From_Z

addText $w e3 "To:"
addParam $w CameraData To_X
addParam $w CameraData To_Y
addParam $w CameraData To_Z

addText $w e4 "Up:"
addParam $w CameraData Up_X
addParam $w CameraData Up_Y
addParam $w CameraData Up_Z

addText $w e5 "Clipping Planes:"
addParam $w CameraData Near
addParam $w CameraData Far

addText $w e6 "Misc:"
addParam $w CameraData Roll
addParam $w CameraData Zoom

# create ViewAttr-UI
array set ViewAttrib {
arr   ViewAttribData
sproc setViewAttr
gproc ""
w     fViewAttr
}

array set ViewAttribData {
 Type 0
 DrawingMode 0
 ModellingMode 0
}

set w [frame $ay(pca).$ViewAttrib(w)]
addVSpace $w s1 2
addMenu $w ViewAttribData Type [list Front Side Top Persp Trim]

addText $w e1 "Window:"
addParam $w ViewAttribData Width
addParam $w ViewAttribData Height

addText $w e2 "Drawing:"
addCheck $w ViewAttribData Redraw
addMenu $w ViewAttribData DrawingMode [list Draw Shade ShadeAndDraw HiddenWire]
addCheck $w ViewAttribData DrawSel
addCheck $w ViewAttribData DrawLevel
addCheck $w ViewAttribData DrawObjectCS
addCheck $w ViewAttribData AALines

addText $w e3 "Grid:"
addParam $w ViewAttribData GridSize
addCheck $w ViewAttribData DrawGrid
addCheck $w ViewAttribData UseGrid

addText $w e4 "Space:"
addMenu $w ViewAttribData ModellingMode\
    [list "Global" "Local (Level)" "Local (Object)" ]


addText $w e5 "Background:"
addCheck $w ViewAttribData DrawBG
addFile $w ViewAttribData BGImage

addText $w e6 "Mark:"
addParam $w ViewAttribData Mark_X
addParam $w ViewAttribData Mark_Y
addParam $w ViewAttribData Mark_Z
addCheck $w ViewAttribData SetMark

addText $w e7 "Undo:"
addCheck $w ViewAttribData EnableUndo
