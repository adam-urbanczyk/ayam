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
global ay AYWITHAQUA

# View Menu
if { ! $AYWITHAQUA } {
    menubutton $w.fMenu.v -text "View" -menu $w.fMenu.v.m -padx 3

    set m [menu $w.fMenu.v.m -tearoff 0]
    set ay(viewm) fMenu.v.m
} else {
    set mb [menu $w.menubar -tearoff 0 -type menubar]
    $w configure -menu $mb
    set m [menu $mb.mview -tearoff 0]
    $mb add cascade -label "View" -menu $m
    set ay(viewm) menubar.mview
}

$m add command -label "Quick Render" -command "viewRender $w 1;\
	\$ay(currentView) mc"

$m add command -label "Render" -command "viewRender $w 0;\
	\$ay(currentView) mc"

$m add command -label "Redraw" -command "\
	global ay;\
	$w.f3D.togl redraw;\
	\$ay(currentView) mc"

$m add command -label "Export RIB" -command {
    global ay
    io_exportRIB [winfo toplevel $w]
}

global AYENABLEPPREV
if { $AYENABLEPPREV == 1 } {
    $m add separator

    $m add command -label "Open PPrev" -command {
	set togl $ay(currentView)
	set w [winfo toplevel $togl]
	$togl setconf -pprev 1
    }

    $m add command -label "Close PPrev" -command {
	set togl $ay(currentView)
	set w [winfo toplevel $togl]
	$togl setconf -pprev 0
    }
}
$m add separator
$m add command -label "Create ShadowMaps" -command "io_RenderSM"

$m add separator

# "after 100" because on Win32 the <Enter>-binding fires when the menu
# is closed and runs parallel to "viewClose" resulting in an error
$m add command -label "Close" -command "after 100 \{viewClose $w;\
	global ay; set ay(ul) root:0; uS\}"

# Type Menu
if { ! $AYWITHAQUA } {
    menubutton $w.fMenu.t -text "Type" -menu $w.fMenu.t.m -padx 3
    set m [menu $w.fMenu.t.m -tearoff 0]
    set ay(typem) fMenu.t.m
} else {
    set m [menu $mb.mtype -tearoff 0]
    $mb add cascade -label "Type" -menu $m
    set ay(typem) menubar.mtype
}

$m add command -label "Front" -command "viewSetType $w 0"

$m add command -label "Side" -command "viewSetType $w 1"
$m add command -label "Top" -command "viewSetType $w 2"
$m add separator
$m add command -label "Perspective" -command "viewSetType $w 3"
$m add separator
$m add command -label "Trim" -command  "viewSetType $w 4"

# Configure Menu
if { ! $AYWITHAQUA } {
    menubutton $w.fMenu.c -text "Configure" -menu $w.fMenu.c.m -padx 3
    set m [menu $w.fMenu.c.m -tearoff 0]
    set ay(confm) fMenu.c.m
} else {
    set m [menu $mb.mconf -tearoff 0]
    $mb add cascade -label "Configure" -menu $m
    set ay(confm) menubar.mconf
}

$m add check -label "Automatic Redraw" -variable ay(cVRedraw) -command "\
	global ay;\
	$w.f3D.togl setconf -draw \$ay(cVRedraw);\
	\$ay(currentView) mc"
set cm [menu $m.mmode -tearoff 0]
$m add cascade -label "Mode" -menu $cm

$cm add radio -label "Draw" -variable ay(cVMode) -value 0 -command "\
	global ay;\
	$w.f3D.togl setconf -shade \$ay(cVMode);\
	\$ay(currentView) mc"

$cm add radio -label "Shade" -variable ay(cVMode) -value 1 -command "\
	global ay;\
	$w.f3D.togl setconf -shade \$ay(cVMode);\
	\$ay(currentView) mc"

$cm add radio -label "ShadeAndDraw" -variable ay(cVMode) -value 2 -command "\
	global ay;\
	$w.f3D.togl setconf -shade \$ay(cVMode);\
	\$ay(currentView) mc"

$m add check -label "Draw Selection only" -variable ay(cVDrawSel) -command "\
	global ay;\
	$w.f3D.togl setconf -dsel \$ay(cVDrawSel);\
	\$ay(currentView) mc"

$m add check -label "Draw Level only" -variable ay(cVDrawLevel) -command "\
	global ay;\
	$w.f3D.togl setconf -dlev \$ay(cVDrawLevel);\
	\$ay(currentView) mc"

$m add separator
$m add check -label "Draw BGImage" -variable ay(cVDrawBG) -command "\
	global ay;\
	$w.f3D.togl setconf -dbg \$ay(cVDrawBG);\
	\$ay(currentView) mc"

$m add separator
$m add check -label "Draw Grid" -variable ay(cVDrawGrid) -command "\
	global ay;\
	$w.f3D.togl setconf -drawg \$ay(cVDrawGrid);\
	\$ay(currentView) mc"

$m add check -label "Use Grid" -variable ay(cVUseGrid) -command "\
	global ay;\
	$w.f3D.togl setconf -ugrid \$ay(cVUseGrid);\
	\$ay(currentView) mc"

$m add command -label "Set GridSize" -command "viewSetGrid $w.f3D.togl"

$m add separator
$m add command -label "Half Size" -command "\
    global ay;\
    set neww \[expr (\[winfo width $w\] / 2)\];\
    set newh \[expr (\[winfo height $w\] / 2)\];\
    wm geometry $w \"\";\
    $w.f3D.togl mc;\
    $w.f3D.togl configure -width \$neww -height \$newh;\
    after 500 \"$w.f3D.togl mc; $w.f3D.togl setconf -dfromx 0.0\";\
    \$ay(currentView) mc"

$m add command -label "Double Size" -command "\
    global ay;\
    set neww \[expr (\[winfo width $w\] * 2)\];\
    set newh \[expr (\[winfo height $w\] * 2)\];\
    wm geometry $w \"\";\
    $w.f3D.togl mc;\
    $w.f3D.togl configure -width \$neww -height \$newh;\
    after 500 \"$w.f3D.togl mc; $w.f3D.togl setconf -dfromx 0.0\";\
    \$ay(currentView) mc"

$m add separator

$m add command -label "From Camera" -command "\
	global ay;\
	$w.f3D.togl mc; $w.f3D.togl fromcam; \$ay(currentView) mc"

$m add command -label "To Camera" -command "\
	global ay;\
	$w.f3D.togl mc; $w.f3D.togl tocam; \$ay(currentView) mc"

$m add command -label "Set FOV" -command "viewSetFOV $w.f3D.togl"

$m add separator
$m add command -label "Zoom to Object" -command "\
	global ay;\
	$w.f3D.togl mc; $w.f3D.togl zoomob; \$ay(currentView) mc"

$m add command -label "Align to Object" -command "\
	global ay;\
	$w.f3D.togl mc; $w.f3D.togl align; \$ay(currentView) mc"

$m add check -label "Edit Local" -variable ay(cVLocal) -command "\
	global ay;\
	$w.f3D.togl setconf -local \$ay(cVLocal);\
	\$ay(currentView) mc"

# XXXX This could be just a label or a menu displaying current action
# or even allowing to start modeling actions, but which actions?
#menubutton $w.fMenu.a -image ay_Move_img -menu $w.fMenu.a.m -padx 3
#menu $w.fMenu.a.m -tearoff 0
#$w.fMenu.a.m add command\
#-label "Quick Render"\
#-command {exit}

# Drawing Mode Menu
if { ! $AYWITHAQUA } {
    menubutton $w.fMenu.dm -image ay_DMDraw_img -menu $w.fMenu.dm.m\
	    -padx 0 -pady 0 -borderwidth 0
    set m [menu $w.fMenu.dm.m -tearoff 0]
    set ay(dmodem) fMenu.dm.m
} else {
    set m [menu $mb.dm -tearoff 0]
    $mb add cascade -image ay_DMDraw_img -menu $m
    set ay(dmodem) menubar.dm
}


$m add command -image ay_DMDraw_img -hidemargin 1 -command "\
        global ay; set ay(cVMode) 0;\
	$w.f3D.togl setconf -shade \$ay(cVMode);\
	viewSetModeIcon $w 0;\
	\$ay(currentView) mc"

$m add command -image ay_DMShade_img -hidemargin 1 -command "\
        global ay; set ay(cVMode) 1;\
	$w.f3D.togl setconf -shade \$ay(cVMode);\
	viewSetModeIcon $w 1;\
	\$ay(currentView) mc"
$m add command -image ay_DMShadeDraw_img -hidemargin 1 -command "\
        global ay; set ay(cVMode) 2;\
	$w.f3D.togl setconf -shade \$ay(cVMode);\
	viewSetModeIcon $w 2;\
	\$ay(currentView) mc"


# Grid
if { ! $AYWITHAQUA } {
    menubutton $w.fMenu.g -image ay_Grid_img -menu $w.fMenu.g.m\
	    -padx 0 -pady 0 -borderwidth 0
    set m [menu $w.fMenu.g.m -tearoff 0]
    set ay(gridm) fMenu.g.m
} else {
    set m [menu $mb.mgrid -tearoff 0]
    $mb add cascade -image ay_Grid_img -menu $m
    set ay(gridm) menubar.gridm
}


$m add command -image ay_Grid01_img -hidemargin 1 -command "\
    $w.f3D.togl setconf -grid 0.1 -drawg 1 -ugrid 1;\
    $w.f3D.togl render;\
    viewSetGridIcon $w 0.1"
$m add command -image ay_Grid025_img -hidemargin 1 -command "\
    $w.f3D.togl setconf -grid 0.25 -drawg 1 -ugrid 1;\
    $w.f3D.togl render;\
    viewSetGridIcon $w 0.25"
$m add command -image ay_Grid05_img -hidemargin 1 -command "\
    $w.f3D.togl setconf -grid 0.5 -drawg 1 -ugrid 1;\
    $w.f3D.togl render;\
    viewSetGridIcon $w 0.5"
$m add command -image ay_Grid10_img -hidemargin 1 -command "\
    $w.f3D.togl setconf -grid 1.0 -drawg 1 -ugrid 1;\
    $w.f3D.togl render;\
    viewSetGridIcon $w 1.0"
$m add command -image ay_Grid_img -hidemargin 1 -command "\
    $w.f3D.togl setconf -grid 0.0 -drawg 0 -ugrid 0;\
    $w.f3D.togl render;\
    viewSetGridIcon $w 0.0"

if { ! $AYWITHAQUA } {
    pack $w.fMenu.v $w.fMenu.t $w.fMenu.c -in $w.fMenu -side left

    pack $w.fMenu.g -in $w.fMenu -side right

    pack $w.fMenu.dm -in $w.fMenu -side right

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


}
# vmenu_open
