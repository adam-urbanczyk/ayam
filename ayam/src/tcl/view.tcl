# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2003 by Randolf Schultz
# (rschultz@informatik.uni-rostock.de) and others.
#
# All rights reserved.
#
# See the file License for details.

# view.tcl - view window management


##############################
# viewSetType:
proc viewSetType { w type } {
 global ay ayprefs tcl_platform

    undo save ViewType
    set togl $w.f3D.togl
    
    set w [winfo toplevel $togl]
    $togl mc
    switch $type {
	0 {
	    $togl setconf -type 0 -fromx 0.0 -fromy 0.0 -fromz 10.0\
		    -tox 0.0 -toy 0.0 -toz 0.0 -upx 0.0 -upy 1.0 -upz 0.0
	    viewTitle $w Front ""
	}
	1 {
	    $togl setconf -type 1 -fromx 10.0 -fromy 0.0 -fromz 0.0\
		    -tox 0.0 -toy 0.0 -toz 0.0 -upx 0.0 -upy 1.0 -upz 0.0
	    viewTitle $w Side ""
	}
	2 {
	    $togl setconf -type 2 -fromx 0.0 -fromy 10.0 -fromz 0.0\
		    -tox 0.0 -toy 0.0 -toz 0.0 -upx 0.0 -upy 0.0 -upz -1.0
	    viewTitle $w Top ""
	}
	3 {
	    $togl setconf -type 3 -fromx 0.0 -fromy 0 -fromz 15.0\
		    -tox 0.0 -toy 0.0 -toz 0.0  -upx 0.0 -upy 1.0 -upz 0.0\
		    -drawg 1 -grid 1 -drotx 45 -droty -30
	    viewTitle $w Persp ""
	    viewSetGridIcon $w 1.0
	}
	4 {
	    $togl setconf -type 4 -fromx 0.0 -fromy 0.0 -fromz 10.0\
		    -tox 0.0 -toy 0.0 -toz 0.0 -upx 0.0 -upy 1.0 -upz 0.0\
		    -zoom 1.0
	    viewTitle $w Trim ""
	}
    }
    #switch

    $togl render
    $ay(currentView) mc
    update
    #actionClear $w

 return;
}
#viewSetType


##############################
# viewRender:
proc viewRender { w type } {
    global env ayprefs ay tcl_platform tmpfile

    set togl $w.f3D.togl

    if { $ayprefs(ShadowMaps) < 1 } {
	tmpGet $ayprefs(TmpDir) tmpfile

	if { $tcl_platform(platform) == "windows" } {
	    # Windows sucks big time!
	    regsub -all {\\} $tmpfile {/} tmpfile
	}

	if { $ayprefs(RenderMode) == 0 } {
	    $togl wrib -file $tmpfile -image ${tmpfile}.tif -temp
	} else {
	    $togl wrib -file $tmpfile -temp
	}

	lappend ay(tmpfiles) [list $tmpfile]
    } else {

	set ribname [io_getRIBName]
	set tmpfile [lindex $ribname 0]
	set imagename [lindex $ribname 1]
	if { $ayprefs(RenderMode) == 0 } {
	    $togl wrib -file $tmpfile -image $imagename -temp
	} else {
	    $togl wrib -file $tmpfile -temp
	}
    }

    # $tcl_platform(platform) == "windows" ||
    set renderui 0
    if { $type == 1 } {
        if { $ayprefs(QRenderUI) == 1} { set renderui 1 }
    } else {
        if { $ayprefs(RenderUI) == 1} { set renderui 1 }
    }
 
    if { $renderui != 1} {
	set command "exec "

	if { $type == 1 } {
	    regsub -all {%s} $ayprefs(QRender) $tmpfile command2
	} else {
	    regsub -all {%s} $ayprefs(Render) $tmpfile command2
	}

	append command $command2
	append command " &"

	eval [subst "$command"]
    } else {
     
	regsub -all {%s} $ayprefs(Render) $tmpfile command

	runRenderer "$command" "$ayprefs(RenderPT)"

    }
    # if

    update
    tmp_clean 0

 return;
}
#viewRender


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
#	set cw $ay(currentView)
	$w.f3D.togl mc
	$w.f3D.togl setconf -pos $x $y
	$ay(currentView) mc
    }

 return;
}
# viewUPos

# viewTitle:
# set title bar of window w (for views only)
# type = "" -> no change, action = "" -> no change
proc viewTitle { w type action } {
    global ay

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
	set m $ay(confm)
	if { $type == "Persp" } {
	    $w.$m entryconfigure 16 -state normal
	} else {
	    $w.$m entryconfigure 16 -state disabled
	}

    }
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
global ay
set w .setFov
catch {destroy $w}
toplevel $w
wm title $w "Set FOV"
wm iconname $w "Ayam"
wm transient $w [winfo toplevel $view]

set f [frame $w.f1]
pack $f -in $w -side top -fill x

set ay(FOV) $ay(cVFOV)
addParam $f ay FOV

set f [frame $w.f2]
button $f.bok -text "Ok" -pady $ay(pady) -width 5 -command "global ay;\
	$view mc;\
	$view setconf -fovx \$ay(FOV);\
	$view render;\
	update;\
	grab release .setFov;\
	focus $view;\
	destroy .setFov"

button $f.bca -text "Cancel" -pady $ay(pady) -width 5 -command "\
	set ay(FOV) $ay(FOV);\
	grab release .setFov;\
	focus $view;\
	destroy $w"

pack $f.bok $f.bca -in $f -side left -fill x -expand yes
pack $f -in $w -side bottom -fill x

winCenter $w
grab $w
focus $w.f1.fFOV.e
tkwait window $w

return;
}
# viewSetFOV


# viewSetGrid:
proc viewSetGrid { view } {
global ay
set w .setGrid
catch {destroy $w}
toplevel $w
wm title $w "Set GridSize"
wm iconname $w "Ayam"
wm transient $w [winfo toplevel $view]

set f [frame $w.f1]
pack $f -in $w -side top -fill x

addParam $f ay GridSize

set f [frame $w.f2]
button $f.bok -text "Ok" -pady $ay(pady) -width 5 -command "\
	global ay;\
	$view mc;\
	$view setconf -grid \$ay(GridSize);\
	$view render;\
	viewSetGridIcon [winfo toplevel $view] \$ay(GridSize);\
	update;\
	grab release .setGrid;\
	focus $view;\
	destroy .setGrid"


button $f.bca -text "Cancel" -pady $ay(pady) -width 5 -command "\
	global ay;
	set ay(GridSize) $ay(GridSize);\
	grab release .setGrid;\
	focus $view;\
	destroy $w"

pack $f.bok $f.bca -in $f -side left -fill x -expand yes
pack $f -in $w -side bottom -fill x

winCenter $w
grab $w
focus $w.f1.fGridSize.e
tkwait window $w

return;
}
# viewSetGrid


#
#
#
proc viewOpen { width height {establish_bindings 1} } {
    global ay

    update
    set ay(cviewsema) 1
    update

    if { [llength $ay(views)] == 0 } {
	set w .view1
    } else {
	set lastname [lindex $ay(views) end]
	scan $lastname ".view%d" lastid
	set w .view[expr $lastid + 1]
    }

    catch {destroy $w}
    if { $ay(truecolor) == 1 } {
	toplevel $w  -visual truecolor
    } else {
	toplevel $w
    }

    scan $w ".%s" name
    wm iconname $w $name

    frame $w.fMenu -bd 2 -relief raised
    vmenu_open $w
    pack $w.fMenu -side top -fill x


    frame $w.f3D

    togl $w.f3D.togl -rgba true -double true -depth true\
	    -ident $name -width $width -height $height


    pack $w.f3D -in $w -fill both -expand yes
    pack $w.f3D.togl -in $w.f3D -fill both -expand yes
    update

    # add to list of views
    lappend ay(views) $w

    $w.f3D.togl setconf -name $name

    viewTitle $w Front Pick

    set ay(currentView) $w.f3D.togl

    wm protocol $w WM_DELETE_WINDOW "viewClose $w;\
	    global ay; set ay(ul) root:0; uS"

    if { $establish_bindings == 1 } {

	# bind internal bindings
	bind $w <Enter> {
	    global ay ayprefs
	    if { $ay(cviewsema) != 1 } {
		update
		set ay(cviewsema) 1
		update
		set w [winfo toplevel %W]
		$w.f3D.togl mc
		set ay(currentView) $w.f3D.togl
		set ay(cviewsema) 0
		update
	    }

	    bind [winfo toplevel %W] <$ayviewshortcuts(RotMod)-Motion> {
		set w [winfo toplevel %W]
		$w.f3D.togl configure -cursor exchange
		bind $w <$ayviewshortcuts(RotMod)-Motion> {}
		bind $w <Motion> {}
	    }
	    bind [winfo toplevel %W] <Motion> {
		set w [winfo toplevel %W]
		$w.f3D.togl configure -cursor left_ptr
		bind $w <Motion> {}
		bind $w <$ayviewshortcuts(RotMod)-Motion> {}
	    }

	    if { $ayprefs(AutoFocus) == 1 } {
		focus [winfo toplevel %W].f3D.togl
	    }

	}
	#bind

	# bind menu shortcuts
	shortcut_view $w

	# bind actions
	shortcut_viewactions $w

	# bind default action
	actionClear $w.f3D.togl
    } else {
	$w configure -cursor watch
    }

    update
    set ay(cviewsema) 0
    update

return;
}
# viewOpen

proc viewBind { w } {

    # internal bindings
    bind $w <Enter> {
	global ay ayprefs
	if { $ay(cviewsema) != 1 } {
	    update
	    set ay(cviewsema) 1
	    update
	    set w [winfo toplevel %W]
	    $w.f3D.togl mc
	    set ay(currentView) $w.f3D.togl
	    set ay(cviewsema) 0
	    update
	}

	bind [winfo toplevel %W] <$ayviewshortcuts(RotMod)-Motion> {
	    set w [winfo toplevel %W]
	    $w.f3D.togl configure -cursor exchange
	    bind $w <$ayviewshortcuts(RotMod)-Motion> {}
	    bind $w <Motion> {}
	}

	bind [winfo toplevel %W] <Motion> {
	    set w [winfo toplevel %W]
	    $w.f3D.togl configure -cursor left_ptr
	    bind $w <Motion> {}
	    bind $w <$ayviewshortcuts(RotMod)-Motion> {}
	}

	if { $ayprefs(AutoFocus) == 1 } {
	    focus [winfo toplevel %W].f3D.togl
	}
    }
    #bind

    # bind menu shortcuts
    shortcut_view $w

    # bind actions
    shortcut_viewactions $w

    # bind default action
    actionClear $w.f3D.togl

    $w configure -cursor {}
    $w.f3D.togl configure -cursor {}

 return;
}
# viewBind

proc viewUnBind { w } {

    foreach v [ bind $w ] { bind $w $v {} }
    foreach v [ bind $w.f3D.togl ] { bind $w.f3D.togl $v {} }
    update
    $w configure -cursor watch
    $w.f3D.togl configure -cursor watch

 return;
}
# viewUnBind

#
proc viewCloseAll { } {
  global ay

  foreach view $ay(views) {
      viewClose $view
  }

 return;
}
# viewCloseAll


#
proc viewClose { w } {
  global ay

  set ay(draw) 0
  update

  # remove bindings that could accidentally fire while closing
  bind $w <Enter> ""
  bind $w <Motion> ""
  global ayviewshortcuts
  bind $w <$ayviewshortcuts(RotMod)-Motion> ""

  # delete view from list
  set temp ""
  set ay(currentView) ""
  foreach view $ay(views) {
    if { $view != $w } {
	set temp [concat $temp $view]
	set ay(currentView) ${view}.f3D.togl
    }
  }

  set ay(views) $temp

  catch {$ay(currentView) mc}

  # now delete the window
  destroy $w.f3D.togl
  destroy $w

  # clear undo buffer
  undo clear

  set ay(draw) 1

 return;
}
# viewClose


set View_props { Camera ViewAttrib }


array set Camera {
arr   CameraData
sproc setCameraAttr
gproc ""
w     fCamera
}
# create Camera-UI
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


array set ViewAttrib {
arr   ViewAttribData
sproc setViewAttr
gproc ""
w     fViewAttr
}

array set ViewAttribData {
Type 0
}


# setCameraAttr:
#
#
proc setCameraAttr { } {
    global ay
    set cw $ay(currentView)
    getName o
    getType t
    if { $t == "View" } {
	.${o}.f3D.togl mc
    }
    setProp

    $ay(currentView) mc

}
# setCameraAttr


# setViewAttr:
#
#
proc setViewAttr { } {
    global ay ViewAttribData pclip_reset

    set cw $ay(currentView)
    getName o
    .${o}.f3D.togl mc

    if { $ViewAttribData(Width) != $pclip_reset(Width) ||
    $ViewAttribData(Height) != $pclip_reset(Height) } {

	.${o}.f3D.togl configure -width $ViewAttribData(Width) -height $ViewAttribData(Height)
	update
    }
    if { $ViewAttribData(Type) != $pclip_reset(Type) } {
	#    setupActionKeys $w clear

	set typename [lindex $ay(viewtypenames) $ViewAttribData(Type)]
	viewTitle .${o} $typename Pick
    }

    viewSetGridIcon .${o} $ViewAttribData(Grid)

    setProp

    $ay(currentView) mc
 return;
}
# setViewAttr

# viewRepairTitle:
#  after undo/redo, set correct view title
proc viewRepairTitle { w type } {
    global ay ViewAttribData

    set typename [lindex $ay(viewtypenames) $type]

    viewTitle .${w} $typename ""
   
}
# viewRepairTitle

# viewSetGridIcon:
#  set correct grid icon according to gridsize
proc viewSetGridIcon { w gridsize } {
    global ay tcl_platform AYWITHAQUA

    if { ! $AYWITHAQUA } {
	set m fMenu.g
	set conf "$w.$m configure"
    } else {
	set m menubar.mgrid
	set conf "$w.menubar entryconfigure 4"
    }

    if { $gridsize == 0.1 } {
	eval "$conf -image ay_Grid01_img"
	} else {
	    if { $gridsize == 0.25 } {
		eval "$conf -image ay_Grid025_img"
	    } else {
		if { $gridsize == 0.5 } {
		    eval "$conf -image ay_Grid05_img"
		} else {
		    if { $gridsize == 1.0 } {
			eval "$conf -image ay_Grid10_img"
		    } else {
			if { $gridsize == 0.0 } {
			    eval "$conf -image ay_Grid_img"
			} else {
			    eval "$conf -image ay_GridX_img"
			}
		    } 
		}
	    }
	}

 return;
}
# viewSetGridIcon

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

}
# viewMouseToCurrent


# create ViewAttr-UI
set w [frame $ay(pca).$ViewAttrib(w)]
array set ViewAttribData {
    Mode 0
}

addMenu $w ViewAttribData Type [list Front Side Top Persp Trim]

addText $w e1 "Window:"
addParam $w ViewAttribData Width
addParam $w ViewAttribData Height

addText $w e2 "Drawing:"
addCheck $w ViewAttribData Redraw
addMenu $w ViewAttribData Mode [list Draw Shade ShadeAndDraw]
addCheck $w ViewAttribData DrawSel
addCheck $w ViewAttribData DrawLevel

addText $w e3 "Grid:"
addParam $w ViewAttribData Grid
addCheck $w ViewAttribData DrawGrid
addCheck $w ViewAttribData UseGrid
addCheck $w ViewAttribData Local
addText $w e4 "Background:"
addCheck $w ViewAttribData DrawBG
addFile $w ViewAttribData BGImage
