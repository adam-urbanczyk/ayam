# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2001 by Randolf Schultz
# (rschultz@informatik.uni-rostock.de) and others.
#
# All rights reserved.
#
# See the file License for details.

# view.tcl - view window management

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

    set w [winfo toplevel $w]

    set oldname [wm title $w] 

    catch {scan $oldname "%s - %s - %s" vname oldtype oldaction}

    scan $w ".view%d" number

    set newvname [format "View%d" $number]

    if { $type != "" } {
	set oldtype $type

	if { $type == "Persp" } {
	    $w.fMenu.c.m entryconfigure 16 -state normal
	} else {
	    $w.fMenu.c.m entryconfigure 16 -state disabled

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
addParam $f ay FOV

set f [frame $w.f2]
button $f.bok -text "Ok" -width 5 -command "global ay;\
	$view mc;\
	$view setconf -fovx \$ay(FOV);\
	$view render;\
	$ay(currentView) mc;\
	update;\
	grab release .setFov;\
	focus $view;\
	destroy .setFov"

button $f.bca -text "Cancel" -width 5 -command "\
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
button $f.bok -text "Ok" -width 5 -command "\
	global ay;\
	$view mc;\
	$view setconf -grid \$ay(GridSize);\
	$view render;\
	viewSetGridIcon [winfo toplevel $view] \$ay(GridSize);\
	$ay(currentView) mc;\
	update;\
	grab release .setGrid;\
	focus $view;\
	destroy .setGrid"


button $f.bca -text "Cancel" -width 5 -command "\
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
proc viewOpen { width height } {
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
toplevel $w

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

viewTitle $w Front None

set ay(currentView) $w.f3D.togl

# internal bindings
bind $w <Enter> {
    global ay
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
}


wm protocol $w WM_DELETE_WINDOW "viewClose $w;\
global ay; set ay(ul) root:0; uS"


# bind menu shortcuts
shortcut_view $w

# bind actions
shortcut_viewactions $w

 update
 set ay(cviewsema) 0
 update

return;
}
# viewOpen


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

  catch {$ay(currentView) makecurrent}

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

addText $w e5 "Misc:"
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
    .${o}.f3D.togl mc

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
	viewTitle .${o} $typename None
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
    global ay 

    if { $gridsize == 0.1 } {
	    $w.fMenu.g configure -image ay_Grid01_img
	} else {
	    if { $gridsize == 0.25 } {
		$w.fMenu.g configure -image ay_Grid025_img
	    } else {
		if { $gridsize == 0.5 } {
		    $w.fMenu.g configure -image ay_Grid05_img
		} else {
		    if { $gridsize == 1.0 } {
			$w.fMenu.g configure -image ay_Grid10_img
		    } else {
			if { $gridsize == 0.0 } {
			    $w.fMenu.g configure -image ay_Grid_img
			} else {
			    $w.fMenu.g configure -image ay_GridX_img
			}
		    } 
		}
	    }
	}

 return;
}
# viewSetGridIcon


# create ViewAttr-UI
set w [frame $ay(pca).$ViewAttrib(w)]

addMenu $w ViewAttribData Type [list Front Side Top Persp Trim]

addText $w e1 "Window:"
addParam $w ViewAttribData Width
addParam $w ViewAttribData Height

addText $w e2 "Drawing:"
addCheck $w ViewAttribData Redraw
addCheck $w ViewAttribData Shade
addCheck $w ViewAttribData DrawSel
addCheck $w ViewAttribData DrawLevel

addText $w e3 "Grid:"
addParam $w ViewAttribData Grid
addCheck $w ViewAttribData DrawGrid
addCheck $w ViewAttribData UseGrid
addCheck $w ViewAttribData Local

