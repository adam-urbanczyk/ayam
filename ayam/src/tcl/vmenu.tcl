# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2001 by Randolf Schultz
# (rschultz@informatik.uni-rostock.de) and others.
#
# All rights reserved.
#
# See the file License for details.

# vmenu.tcl - the view menu

proc vmenu_open { w } {
# View
menubutton $w.fMenu.v -text "View" -menu $w.fMenu.v.m -padx 3
menu $w.fMenu.v.m -tearoff 0
$w.fMenu.v.m add command\
-label "Quick Render"\
-command {
 global env ayprefs ay tcl_platform

 tmpGet $ayprefs(TmpDir) tmpfile

 if { $tcl_platform(platform) == "windows" } {
    # Windows sucks big time!
     
    regsub -all {\\} $tmpfile {/} tmpfile
    #was: regsub -all {\\} $tmpfile {\\\\\\\\\\\\\\\\} tmpfile
 }

 $ay(currentView) wrib -file $tmpfile -image ${tmpfile}.tif -temp


 if { $ayprefs(QRenderUI) != 1} {
     set command "exec "

     regsub -all {%s} $ayprefs(QRender) $tmpfile command2

     append command $command2
     append command " &"

     eval [subst "$command"]
 } else {
     
     regsub -all {%s} $ayprefs(QRender) $tmpfile command

     runRenderer "$command" $ayprefs(QRenderPT)

 }

 update
 tmp_clean 0

}

$w.fMenu.v.m add command\
-label "Render"\
-command {\
global env ayprefs ay tcl_platform

 tmpGet $ayprefs(TmpDir) tmpfile

 if { $tcl_platform(platform) == "windows" } {
    # Windows sucks big time!
    regsub -all {\\} $tmpfile {/} tmpfile
 }

 $ay(currentView) wrib -file $tmpfile -image ${tmpfile}.tif -temp

 lappend ay(tmpfiles) [list $tmpfile]
# $tcl_platform(platform) == "windows" || 
 if { $ayprefs(RenderUI) != 1} {
     set command "exec "

     regsub -all {%s} $ayprefs(Render) $tmpfile command2
     puts here
     append command $command2
     append command " &"

     eval [subst "$command"]
 } else {
     
     regsub -all {%s} $ayprefs(Render) $tmpfile command

     runRenderer "$command" "$ayprefs(RenderPT)"

 }

 update
 tmp_clean 0

}

$w.fMenu.v.m add command -label "Redraw" -command {
    global ay
    $ay(currentView) redraw
}

$w.fMenu.v.m add command -label "Export RIB" -command {
    global ay
    io_exportRIB [winfo toplevel $w]
}

$w.fMenu.v.m add separator

# "after 100" because on Win32 the <Enter>-binding fires when the menu
# is closed and runs parallel to "viewClose" resulting in an error
$w.fMenu.v.m add command -label "Close" -command "after 100 \{viewClose $w;\
	global ay; set ay(ul) root:0; uS\}"

# Type

menubutton $w.fMenu.t -text "Type" -menu $w.fMenu.t.m -padx 3
set m [menu $w.fMenu.t.m -tearoff 0]
$m add command -label "Front" -command {
    global ay
    undo save
    set togl $ay(currentView)
    set w [winfo toplevel $togl]

    $togl setconf -type 0 -fromx 0.0 -fromy 0.0 -fromz 10.0\
	    -tox 0.0 -toy 0.0 -toz 0.0 -upx 0.0 -upy 1.0 -upz 0.0

    update
#    actionClear $w
    viewTitle $w Front ""
    $togl render
}
$m add command -label "Side" -command {
    global ay
    undo save
    set togl $ay(currentView)
    set w [winfo toplevel $togl]

    $togl setconf -type 1 -fromx 10.0 -fromy 0.0 -fromz 0.0\
	    -tox 0.0 -toy 0.0 -toz 0.0 -upx 0.0 -upy 1.0 -upz 0.0
    update
#    actionClear $w
    viewTitle $w Side ""
    $togl render
}
$m add command -label "Top" -command {
    global ay
    undo save
    set togl $ay(currentView)
    set w [winfo toplevel $togl]

    $togl setconf -type 2 -fromx 0.0 -fromy 10.0 -fromz 0.0\
	    -tox 0.0 -toy 0.0 -toz 0.0 -upx 0.0 -upy 0.0 -upz -1.0
    update
#    actionClear $w
    viewTitle $w Top ""
    $togl render
}
$m add separator
$m add command -label "Perspective" -command {
    global ay
    undo save
    set togl $ay(currentView)
    set w [winfo toplevel $togl]
    $togl setconf -type 3 -fromx 0.0 -fromy 0 -fromz 15.0\
	    -tox 0.0 -toy 0.0 -toz 0.0  -upx 0.0 -upy 1.0 -upz 0.0\
	    -drawg 1 -grid 1 -drotx 45 -droty -30
    update
#    actionClear $w
    viewTitle $w Persp ""
    $togl render
}
$m add separator
$m add command -label "Trim" -command {
    global ay
    undo save
    set togl $ay(currentView)
    set w [winfo toplevel $togl]
    $togl setconf -type 4 -fromx 0.0 -fromy 0.0 -fromz 10.0\
	    -tox 0.0 -toy 0.0 -toz 0.0 -upx 0.0 -upy 1.0 -upz 0.0\
	    -zoom 1.0
    update
#    actionClear $w
    viewTitle $w Trim ""
    update
    $togl render
}

# Configure

menubutton $w.fMenu.c -text "Configure" -menu $w.fMenu.c.m -padx 3
set m [menu $w.fMenu.c.m -tearoff 0]

$m add check -label "Automatic Redraw" -variable ay(cVRedraw)\
	-command {
    global ay
    $ay(currentView) setconf -draw $ay(cVRedraw)
}
$m add check -label "Shade" -variable ay(cVShade) -command {
    global ay
    $ay(currentView) setconf -shade $ay(cVShade)
    set w [winfo toplevel $ay(currentView)]
#    if { $ay(cVShade) == 1 } {
#	$w.fMenu.g configure -state disabled
#    } else {
#	$w.fMenu.g configure -state enabled
#    }
}
$m add check -label "Draw Selection only" -variable ay(cVDrawSel)\
	-command {
    global ay
    $ay(currentView) setconf -dsel $ay(cVDrawSel)
}
$m add check -label "Draw Level only" -variable ay(cVDrawLevel)\
	-command {
    global ay
    $ay(currentView) setconf -dlev $ay(cVDrawLevel) 
}
$m add separator
$m add check -label "Draw Grid" -variable ay(cVDrawGrid) -command {
    global ay
    $ay(currentView) setconf -drawg $ay(cVDrawGrid)
}
$m add check -label "Use Grid" -variable ay(cVUseGrid) -command {
    global ay
    $ay(currentView) setconf -ugrid $ay(cVUseGrid)
}
set m $w.fMenu.c.m
$m add command -label "Set GridSize" -command {
    global ay;
    viewSetGrid $ay(currentView)
}

$m add separator
$m add check -label "Local" -variable ay(cVLocal) -command {
    global ay
    $ay(currentView) setconf -local $ay(cVLocal)
}
$m add separator
$m add command -label "Half Size" -command {
    global ay

    set v $ay(currentView)

    set neww [expr ([winfo width $v] / 2)]
    set newh [expr ([winfo height $v] / 2)]
    wm geometry [winfo toplevel $v] ""
    $v configure -width $neww -height $newh
    update
    $v setconf -dfromx 0.0
}

$m add command -label "Double Size" -command {
    global ay

    set v $ay(currentView)

    set neww [expr ([winfo width $v] * 2)]
    set newh [expr ([winfo height $v] * 2)]
    wm geometry [winfo toplevel $v] ""
    $v configure -width $neww -height $newh
    update
    $v setconf -dfromx 0.0
}

$m add separator

$m add command -label "From Camera" -command { global ay;
$ay(currentView) fromcam }

$m add command -label "To Camera" -command { global ay;
$ay(currentView) tocam }

$m add command -label "Set FOV" -command { global ay; viewSetFOV $ay(currentView)}

$m add separator
$m add command -label "Zoom to Object" -command { global ay;
$ay(currentView) zoomob }

$m add command -label "Align" -command { global ay;
$ay(currentView) align }

# XXXX This could be just a label or a menu displaying current action
# or even allowing to start modeling actions, but which actions?
#menubutton $w.fMenu.a -image ay_Move_img -menu $w.fMenu.a.m -padx 3
#menu $w.fMenu.a.m -tearoff 0
#$w.fMenu.a.m add command\
#-label "Quick Render"\
#-command {exit}

menubutton $w.fMenu.g -image ay_Grid_img -menu $w.fMenu.g.m -padx 0 -pady 0\
-borderwidth 0
menu $w.fMenu.g.m -tearoff 0
$w.fMenu.g.m add command -image ay_Grid01_img -hidemargin 1 -command "\
    $w.f3D.togl setconf -grid 0.1 -drawg 1 -ugrid 1;\
    $w.f3D.togl render;\
    viewSetGridIcon $w 0.1"
$w.fMenu.g.m add command -image ay_Grid025_img -hidemargin 1 -command "\
    $w.f3D.togl setconf -grid 0.25 -drawg 1 -ugrid 1;\
    $w.f3D.togl render;\
    viewSetGridIcon $w 0.25"
$w.fMenu.g.m add command -image ay_Grid05_img -hidemargin 1 -command "\
    $w.f3D.togl setconf -grid 0.5 -drawg 1 -ugrid 1;\
    $w.f3D.togl render;\
    viewSetGridIcon $w 0.5"
$w.fMenu.g.m add command -image ay_Grid10_img -hidemargin 1 -command "\
    $w.f3D.togl setconf -grid 1.0 -drawg 1 -ugrid 1;\
    $w.f3D.togl render;\
    viewSetGridIcon $w 1.0"
$w.fMenu.g.m add command -image ay_Grid_img -hidemargin 1 -command "\
    $w.f3D.togl setconf -grid 0.0 -drawg 0 -ugrid 0;\
    $w.f3D.togl render;\
    viewSetGridIcon $w 0.0"


pack $w.fMenu.v $w.fMenu.t $w.fMenu.c -in $w.fMenu -side left

pack $w.fMenu.g -in $w.fMenu -side right

#pack $w.fMenu.a -in $w.fMenu -side right

# XXXX Win32 Menus are a bit to tall
global tcl_platform
if { $tcl_platform(platform) == "windows" } {
    set children [winfo children $w.fMenu]
    foreach child $children {
	$child configure -pady 1
    }
}


}
# vmenu_open
