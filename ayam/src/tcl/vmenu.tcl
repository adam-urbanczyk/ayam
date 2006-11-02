# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2005 by Randolf Schultz
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

    # correct application menu (about entry)
    menu $w.menubar.apple
    $w.menubar add cascade -menu $w.menubar.apple
    $w.menubar.apple add command -label "About Ayam" -command "aboutAyam"
}

$m add command -label "Quick Render" -command "viewRender $w 1;\
	\$ay(currentView) mc"

$m add command -label "Render" -command "viewRender $w 0;\
	\$ay(currentView) mc"

$m add command -label "Render To File" -command "viewRender $w 2;\
	\$ay(currentView) mc"

$m add command -label "Redraw" -command "\
	global ay;\
	$w.f3D.togl mc;\
	$w.f3D.togl reshape;\
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
$m add command -label "Create All ShadowMaps" -command "io_RenderSM 1"
$m add command -label "Create ShadowMap" -command "io_RenderSM 0"

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
$m add cascade -label "Drawing Mode" -menu $cm

$cm add radio -label "Draw" -variable ay(cVDMode) -value 0 -command "\
	global ay;\
	$w.f3D.togl setconf -shade \$ay(cVDMode);\
	\$ay(currentView) mc"

$cm add radio -label "Shade" -variable ay(cVDMode) -value 1 -command "\
	global ay;\
	$w.f3D.togl setconf -shade \$ay(cVDMode);\
	\$ay(currentView) mc"

$cm add radio -label "ShadeAndDraw" -variable ay(cVDMode) -value 2 -command "\
	global ay;\
	$w.f3D.togl setconf -shade \$ay(cVDMode);\
	\$ay(currentView) mc"

$m add check -label "Draw Selection only" -variable ay(cVDrawSel) -command "\
	global ay;\
	$w.f3D.togl setconf -dsel \$ay(cVDrawSel);\
	\$ay(currentView) mc"

$m add check -label "Draw Level only" -variable ay(cVDrawLevel) -command "\
	global ay;\
	$w.f3D.togl setconf -dlev \$ay(cVDrawLevel);\
	\$ay(currentView) mc"

$m add check -label "Draw Object CS" -variable ay(cVDrawOCS) -command "\
	global ay;\
	$w.f3D.togl setconf -docs \$ay(cVDrawOCS);\
	\$ay(currentView) mc"

$m add separator
$m add check -label "Draw BGImage" -variable ay(cVDrawBG) -command "\
	global ay;\
	$w.f3D.togl setconf -dbg \$ay(cVDrawBG);\
	\$ay(currentView) mc"
$m add command -label "Set BGImage" -command "viewSetBGImage $w.f3D.togl"

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
    set neww \[expr (\[winfo reqwidth $w.f3D.togl\] / 2)\];\
    set newh \[expr (\[winfo reqheight $w.f3D.togl\] / 2)\];\
    wm geometry $w \"\";\
    $w.f3D.togl mc;\
    $w.f3D.togl configure -width \$neww -height \$newh;\
    after 500 \"$w.f3D.togl mc; $w.f3D.togl setconf -dfromx 0.0\";\
    \$ay(currentView) mc"

$m add command -label "Double Size" -command "\
    global ay;\
    set neww \[expr (\[winfo reqwidth $w.f3D.togl\] * 2)\];\
    set newh \[expr (\[winfo reqheight $w.f3D.togl\] * 2)\];\
    wm geometry $w \"\";\
    $w.f3D.togl mc;\
    $w.f3D.togl configure -width \$neww -height \$newh;\
    after 500 \"$w.f3D.togl mc; $w.f3D.togl setconf -dfromx 0.0\";\
    \$ay(currentView) mc"

$m add separator

$m add command -label "From Camera" -command "\
	global ay;\
	undo save FromCamera;\
	$w.f3D.togl mc; $w.f3D.togl fromcam; \$ay(currentView) mc"

$m add command -label "To Camera" -command "\
	global ay;\
	undo save ToCamera;\
	$w.f3D.togl mc; $w.f3D.togl tocam; \$ay(currentView) mc"

$m add command -label "Set FOV" -command "viewSetFOV $w.f3D.togl"

$m add separator
$m add command -label "Zoom to Object" -command "\
	global ay;\
	undo save ZoomToObj;\
	$w.f3D.togl mc; $w.f3D.togl zoomob; \$ay(currentView) mc"

$m add command -label "Align to Object" -command "\
	global ay;\
	undo save AlignToObj;\
	$w.f3D.togl mc; $w.f3D.togl align; \$ay(currentView) mc"

# XXXX This could be just a label or a menu displaying current action
# or even allowing to start modeling actions, but which actions?
#menubutton $w.fMenu.a -image ay_Move_img -menu $w.fMenu.a.m -padx 3
#menu $w.fMenu.a.m -tearoff 0
#$w.fMenu.a.m add command\
#-label "Quick Render"\
#-command {exit}

# Modelling Mode Menu
if { ! $AYWITHAQUA } {
    menubutton $w.fMenu.mm -image ay_MMGlobLoc_img -menu $w.fMenu.mm.m\
	    -padx 0 -pady 0 -borderwidth 0
    balloon_set $w.fMenu.mm "change global/local mode"
    set m [menu $w.fMenu.mm.m -tearoff 0]

    set ay(mmodem) fMenu.mm.m
} else {
    set m [menu $mb.mm -tearoff 0]
    $mb add cascade -label Local -menu $m
    set ay(mmodem) menubar.mm
}


$m add command -image ay_MMGlobLoc_img -hidemargin 1 -command "\
        global ay; set ay(cVMMode) 0;\
	$w.f3D.togl setconf -local \$ay(cVMMode);\
	viewSetMModeIcon $w 0;\
	\$ay(currentView) mc"

$m add command -image ay_MMLocGlob_img -hidemargin 1 -command "\
        global ay; set ay(cVMMode) 1;\
	$w.f3D.togl setconf -local \$ay(cVMMode);\
	viewSetMModeIcon $w 1;\
	\$ay(currentView) mc"

if { $AYWITHAQUA } {
    $m entryconfigure 0 -image {} -label Local
    $m entryconfigure 1 -image {} -label Global
}

# Drawing Mode Menu
if { ! $AYWITHAQUA } {
    menubutton $w.fMenu.dm -image ay_DMDraw_img -menu $w.fMenu.dm.m\
	    -padx 0 -pady 0 -borderwidth 0
    balloon_set $w.fMenu.dm "change drawing mode"
    set m [menu $w.fMenu.dm.m -tearoff 0]

    set ay(dmodem) fMenu.dm.m
} else {
    set m [menu $mb.dm -tearoff 0]
    $mb add cascade -label Draw -menu $m
    set ay(dmodem) menubar.dm
}


$m add command -image ay_DMDraw_img -hidemargin 1 -command "\
        global ay; set ay(cVDMode) 0;\
	$w.f3D.togl setconf -shade \$ay(cVDMode);\
	viewSetDModeIcon $w 0;\
	\$ay(currentView) mc"

$m add command -image ay_DMShade_img -hidemargin 1 -command "\
        global ay; set ay(cVDMode) 1;\
	$w.f3D.togl setconf -shade \$ay(cVDMode);\
	viewSetDModeIcon $w 1;\
	\$ay(currentView) mc"
$m add command -image ay_DMShadeDraw_img -hidemargin 1 -command "\
        global ay; set ay(cVDMode) 2;\
	$w.f3D.togl setconf -shade \$ay(cVDMode);\
	viewSetDModeIcon $w 2;\
	\$ay(currentView) mc"

if { $AYWITHAQUA } {
    $m entryconfigure 0 -image {} -label "Draw"
    $m entryconfigure 1 -image {} -label "Shade"
    $m entryconfigure 2 -image {} -label "Shade&Draw"
}

# Grid Menu
if { ! $AYWITHAQUA } {
    menubutton $w.fMenu.g -image ay_Grid_img -menu $w.fMenu.g.m\
	    -padx 0 -pady 0 -borderwidth 0
    balloon_set $w.fMenu.g "change gridsize"
    set m [menu $w.fMenu.g.m -tearoff 0]
    set ay(gridm) fMenu.g.m
} else {
    set m [menu $mb.mgrid -tearoff 0]
    $mb add cascade -label Grid -menu $m
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
$m add command -image ay_GridX_img -hidemargin 1 -command "
    after idle \{$w.$ay(confm) invoke 11\}"

$m add command -image ay_Grid_img -hidemargin 1 -command "\
    $w.f3D.togl setconf -grid 0.0 -drawg 0 -ugrid 0;\
    $w.f3D.togl render;\
    viewSetGridIcon $w 0.0"


if { $AYWITHAQUA } {
    $m entryconfigure 0 -image {} -label "Grid 0.1"
    $m entryconfigure 1 -image {} -label "Grid 0.25"
    $m entryconfigure 2 -image {} -label "Grid 0.5"
    $m entryconfigure 3 -image {} -label "Grid 1.0"
    $m entryconfigure 4 -image {} -label "Set Grid"
    $m entryconfigure 5 -image {} -label "No Grid"
}

# Help menu (just for MacOSX/Aqua!)
if { $AYWITHAQUA } {
set m [menu $mb.help -tearoff 0]
$mb add cascade -label "Help" -menu $m
$m add command -label "Help" -command {
    after idle {
	global ayprefs
	browser_urlOpen $ayprefs(Docs)
    }
}

$m add command -label "Help on object" -command {
    after idle {
	global ayprefs
	set selected ""
	getSel selected
	if { $selected == "" } {
	    ayError 2 "Help on object" "Please select an object!"
	    return;
	}
	getType type
	set type [string tolower $type]
	if { [string first "file://" $ayprefs(Docs)] != -1 } {
	    set lslash [string last "/" $ayprefs(Docs)]
	    set url [string range
		     $ayprefs(Docs) 0 $lslash]/ayam-4.html\#${type}obj
	    browser_urlOpen $url
	} else {
	    browser_urlOpen $ayprefs(Docs)ayam-4.html\#${type}obj
	}
    }
}

$m add command -label "Show Shortcuts" -command "shortcut_show"
$m add command -label "About" -command "aboutAyam"
$m add checkbutton -label "Show Tooltips" -variable ayprefs(showtt)

}
# if

if { ! $AYWITHAQUA } {
    pack $w.fMenu.v $w.fMenu.t $w.fMenu.c -in $w.fMenu -side left

    pack $w.fMenu.g -in $w.fMenu -side right

    pack $w.fMenu.dm -in $w.fMenu -side right

    pack $w.fMenu.mm -in $w.fMenu -side right

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

return;
}
# vmenu_open
