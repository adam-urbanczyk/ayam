# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2001 by Randolf Schultz
# (rschultz@informatik.uni-rostock.de) and others.
#
# All rights reserved.
#
# See the file License for details.

# mmenu.tcl - the main menu

global ay AYWITHAQUA

if { ! $AYWITHAQUA } {
    frame .fu.fMenu -bd 2 -relief raised
    pack .fu.fMenu -side top -fill x
    # File
    menubutton .fu.fMenu.fil -text "File" -menu .fu.fMenu.fil.m -padx 3

    menu .fu.fMenu.fil.m -tearoff 0
    set m .fu.fMenu.fil.m
} else {
    #frame .fu.fMenu
    #pack .fu.fMenu -side top -fill x
    set mb [menu .fu.menubar -tearoff 0 -type menubar]
    . configure -menu $mb
    # File
    set m [menu $mb.mfil -tearoff 0]
    $mb add cascade -label "File" -menu $m
}

set ay(filemenu) $m

$m add command -label "New"\
	-command {
    global ay ayprefs tcl_platform
    if { ! [io_warnChanged] } { 
	update; newScene; uS;
	if { $ayprefs(NewLoadsEnv) == 1 } {
	    viewCloseAll; cS; plb_update
	    set filename [file nativename $ayprefs(EnvFile)]
	    if { $tcl_platform(platform) == "windows" } {
		regsub -all {\\} $filename {/} filename
	    }
	    catch [replaceScene $filename]
	    update

	}
	set ay(filename) ""
	wm title . "Ayam - Main"
	goTop
	selOb
	set ay(CurrentLevel) "root"
	set ay(SelectedLevel) "root"
	update
	uS
	rV
	set ay(sc) 0
	update
	foreach view $ay(views) { viewBind $view }
	update
	after idle viewMouseToCurrent
    }
}
$m add separator
$m add command -label "Replace" -command {
    if { ! [io_warnChanged] } {
	io_replaceScene
    }
}
$m add command -label "Insert" -command "io_insertScene"
$m add separator
$m add command -label "Save as" -command "io_saveScene ask 0"
$m add command -label "Save" -command "io_saveScene dontask 0"
$m add separator
$m add command -label "Import Mops" -command "io_importMops"
$m add separator
$m add command -label "Export RIB" -command "io_exportRIB"
$m add separator
$m add command -label "Load Custom" -command "io_loadCustom"
$m add separator
$m add command -label "Save Prefs" -command "prefs_save"
$m add separator
global ayprefs
set label "1. [lindex $ayprefs(mru) 0]"
$m add command -label $label -command { global ay
if { ! [io_warnChanged] } {io_mruLoad 0}}
set label "2. [lindex $ayprefs(mru) 1]"
$m add command -label $label -command { global ay
if { ! [io_warnChanged] } {io_mruLoad 1}}
set label "3. [lindex $ayprefs(mru) 2]"
$m add command -label $label -command { global ay
if { ! [io_warnChanged] } {io_mruLoad 2}}
set label "4. [lindex $ayprefs(mru) 3]"
$m add command -label $label -command { global ay
if { ! [io_warnChanged] } {io_mruLoad 3}}
$m add separator
$m add command -label "Exit!" -command {
    global ayprefs

    if { ! [io_warnChanged] } { 

	# remove all temporary files
	catch [ tmp_clean 1 ]

	if { $ayprefs(AutoSavePrefs) == 1 } {
	    catch [ prefs_save ]
	}
    
	puts "Good Bye!"
	update
	exit
    }
}


if { ! $AYWITHAQUA } {
    pack .fu.fMenu.fil -in .fu.fMenu -side left

    # Edit
    menubutton .fu.fMenu.ed -text "Edit" -menu .fu.fMenu.ed.m -padx 3
    menu .fu.fMenu.ed.m  -tearoff 0
    set m .fu.fMenu.ed.m
} else {
    # Edit
    set m [menu $mb.medi -tearoff 0]
    $mb add cascade -label "Edit" -menu $m

}
set ay(editmenu) $m
$m add command -label "Copy" -command {copOb}
$m add command -label "Cut" -command {cutOb; cS;
global ay; set ay(ul) $ay(CurrentLevel); uS; rV; set ay(sc) 1}
$m add command -label "Paste" -command {pasOb; cS;
global ay; set ay(ul) $ay(CurrentLevel); uS; rV; set ay(sc) 1}
$m add command -label "Delete" -command {delOb; cS;
global ay; set ay(ul) $ay(CurrentLevel); uS; rV; set ay(sc) 1}
$m add separator
$m add command -label "Copy Property" -command {pclip_copy 0}
$m add command -label "Copy Marked Prop" -command {pclip_copy 1}
$m add command -label "Paste Property" -command {global ay;
pclip_paste; set ay(sc) 1}
$m add separator
$m add command -label "Undo" -command {undo; uCL cl "0 1"; plb_update; rV}
$m add command -label "Redo" -command {undo redo; uCL cl "0 1"; plb_update; rV}
$m add separator
$m add command -label "Material" -command {material_edit;}
$m add command -label "Master" -command {instance_edit;}
$m add separator
$m add command -label "Preferences" -command {prefs_open; rV}

if { ! $AYWITHAQUA } {
    pack .fu.fMenu.ed -in .fu.fMenu -side left

    # Create
    menubutton .fu.fMenu.cr -text "Create" -menu .fu.fMenu.cr.m -padx 3
    menu .fu.fMenu.cr.m -tearoff 0
    set m .fu.fMenu.cr.m
} else {
    # Create
    set m [menu $mb.mcrt -tearoff 0]
    $mb add cascade -label "Create" -menu $m
}

set ay(createmenu) $m

$m add command -label "NURBCurve" -command {
    runTool ay(nclen) "Length:" "crtOb NCurve -length %0; uCR; sL; rV;"   
}
$m add command -label "ICurve" -command {
    runTool ay(iclen) "Length:" "crtOb ICurve -length %0; uCR; sL; rV;"
}
$m add command -label "NURBPatch" -command {
    runTool {ay(npwidth) ay(npheight)} {"Width:" "Height:"}\
	    "crtOb NPatch -width %0 -height %1; uCR; sL; rV;"
}
$m add command -label "BPatch" -command {
    crtOb BPatch; uCR; sL; rV;
}
$m add command -label "PatchMesh" -command {
    runTool {ay(pmwidth) ay(pmheight)} {"Width:" "Height:"}\
	    "crtOb PatchMesh -width %0 -height %1; uCR; sL; rV;"
}
$m add separator
$m add cascade -menu $m.sl -label "Solid"
menu $m.sl -tearoff 0
$m.sl add command -label "Box"\
-command "crtOb Box; uCR; sL; rV;"
$m.sl add command -label "Sphere"\
-command "crtOb Sphere; uCR; sL; rV;"
$m.sl add command -label "Disk"\
-command "crtOb Disk; uCR; sL; rV;"
$m.sl add command -label "Cone"\
-command "crtOb Cone; uCR; sL; rV;"
$m.sl add command -label "Cylinder"\
-command "crtOb Cylinder; uCR; sL; rV;"
$m.sl add command -label "Torus"\
-command "crtOb Torus; uCR; sL; rV;"
$m.sl add command -label "Hyperboloid"\
-command "crtOb Hyperboloid; uCR; sL; rV;"
$m.sl add command -label "Paraboloid"\
-command "crtOb Paraboloid;  uCR; sL; rV;"

$m add cascade -menu $m.csg -label "Level"
menu $m.csg -tearoff 0
$m.csg add command -label "Level"\
-command "crtOb Level 1; uCR; sL; rV;"
$m.csg add separator
$m.csg add command -label "Union"\
-command "crtOb Level 2; uCR; sL; rV;"
$m.csg add command -label "Intersection"\
-command "crtOb Level 4; uCR; sL; rV;"
$m.csg add command -label "Difference"\
-command "crtOb Level 3; uCR; sL; rV;"
$m.csg add separator
$m.csg add command -label "Primitive"\
-command "crtOb Level 5; uCR; sL; rV;"

#$m.csg add command -label "TrimLoop"\
#-command "crtOb level tloop; updateMopsSelect; selectLast; redrawAllViews"
$m add command -label "Light" -command "crtOb Light; uCR; sL; rV;"
$m add separator
$m add cascade -menu $m.cus -label "Custom Object"
menu $m.cus -tearoff 0
#$m.cus add command -label "Revolve"\
# -command "crtOb custom Revolve; updateMopsSelect; redrawAllViews"
#$m.cus add command -label "Extrude"\
# -command "crtOb custom Extrude; updateMopsSelect; redrawAllViews"
#$m.cus add command -label "Sweep"\
# -command "crtOb custom Sweep; updateMopsSelect; redrawAllViews"
$m add separator
$m add command -label "Instance" \
	-command "crtOb Instance; uCR; sL; rV;"
$m add command -label "Clone" \
	-command "crtOb Clone; uCR; sL;rV;"
$m add command -label "View" \
	-command "viewOpen 400 300; global ay; set ay(ul) root:0; uS 0 1; rV;"
$m add command -label "Material" \
	-command "material_createp;"
$m add command -label "Camera" \
	-command "crtOb Camera; uCR; sL;rV;"
$m add command -label "RiInc" \
	-command "crtOb RiInc; uCR; sL;rV;"

if { ! $AYWITHAQUA } {
    pack .fu.fMenu.cr -in .fu.fMenu -side left

    # Tools
    menubutton .fu.fMenu.tool -text "Tools" -menu .fu.fMenu.tool.m -padx 3
    set m [menu .fu.fMenu.tool.m -tearoff 0]
} else {
    # Tools
    set m [menu $mb.mtools -tearoff 0]
    $mb add cascade -label "Tools" -menu $m
}

set ay(toolsmenu) $m

$m add cascade -menu $m.nc -label "Create"
menu $m.nc -tearoff 0
$m.nc add command -label "ClosedBSpline" -command {
   runTool ay(cbspnum) "Number of Points:" "crtClosedBS %0; uCR; sL; rV;"
}
$m.nc add command -label "NURBCircle" -command {
    crtNCircle; uCR; sL; rV }
$m.nc add command -label "NURBCircleArc" -command {
    runTool ay(ncircarc) "Arc:" "crtNCircle %0; uCR; sL; rV;"
}
$m.nc add command -label "TrimRect" -command {
    crtNRect; uCR; sL; rV}
$m.nc add separator
$m.nc add command -label "NURBSphere" -command {
    crtNSphere; uCR; sL; rV}
$m.nc add command -label "NURBSphere2" -command {
    crtNSphere2; uCR; sL; rV}

$m add separator
$m add cascade -menu $m.nct -label "NURBCurve"
menu $m.nct -tearoff 0
$m.nct add command -label "Revolve" -command "revolve_crt;"
$m.nct add command -label "Extrude" -command "extrude_crt;"
$m.nct add command -label "Sweep" -command "sweep_crt;"
$m.nct add command -label "Skin" -command "skin_crt;"
$m.nct add command -label "Cap" -command "cap_crt;"
$m.nct add separator
$m.nct add command -label "Revert" -command { undo save Revert; revertNC;
                                              plb_update; rV }
$m.nct add command -label "Concat" -command { concatNC; uCR; rV}
$m.nct add command -label "Split" -command { 
runTool ay(splitu) {"Split at:"} "undo save; splitNC %0; uCR; sL; rV" }
$m.nct add command -label "Refine" -command { undo save Refine; refineNC;
                                              plb_update; rV }
#$m.nct add command -label "Refine with" -command { runTool ay(refinekn) {"New Knots:"} "refineNC \{%0\}; plb_update; rV" }
$m.nct add command -label "Clamp" -command { undo save Clamp; clampNC;
                                             plb_update; rV }
$m.nct add command -label "Elevate" -command {
    runTool ay(elevd) {"Elevate by:"}\
	    "undo save Elevate; elevateNC %0; plb_update; rV" }
$m.nct add command -label "Insert Knot" -command {
    runTool [list ay(insknu) ay(insknr)]\
	    [list "Insert knot at:" "Insert times:"]\
	    "undo save InsKn; insknNC %0 %1; plb_update; rV" }
$m.nct add command -label "Plot Curvature" -command {
    runTool [list ay(curvatp) ay(curvatw) ay(curvats)]\
	    [list "Data points:" "Width:" "Scale Height:"]\
	    "curvPlot %0 %1 %2; uCR; rV" }

$m.nct add separator
$m.nct add command -label "Collapse Points" -command { collNC; rV; }
$m.nct add command -label "Explode Points" -command { explNC; rV; }

$m add separator
$m add cascade -menu $m.npt -label "NURBPatch"
menu $m.npt -tearoff 0
$m.npt add command -label "Split to Curves (u)" -command {
    splitNP u; uS; sL; rV}
$m.npt add command -label "Split to Curves (v)" -command {
    splitNP v; uS; sL; rV}
$m.npt add command -label "Build from Curves" -command {
    buildNP; uCR; sL; rV}
$m add separator

$m add cascade -menu $m.pm -label "PolyMesh"
menu $m.pm -tearoff 0
$m.pm add command -label "Merge" -command {
    pomesh_merge
}
$m.pm add command -label "Optimize" -command { pomesh_optimize }
$m add separator
$m add command -label "Hide" -command {
    global ay
    undo save Hide
    hide
    set ay(ul) $ay(CurrentLevel); uS 1 1; rV
}
$m add command -label "Hide All" -command "hide -all; uS 1 1; rV"
$m add command -label "Show" -command {
    global ay
    undo save Show
    show
    set ay(ul) $ay(CurrentLevel); uS 1 1; rV
}
$m add command -label "Show All" -command "show -all; uS 1 1; rV"
$m add separator
$m add command -label "Convert" -command {
    global ay
    convOb; update; cS; set ay(ul) $ay(CurrentLevel); uS; rV
}
$m add command -label "Convert (In Place)" -command {
    global ay;
    #undo save Convert;
    convOb -inplace; update; set ay(ul) $ay(CurrentLevel); uS 1 1; rV
}
$m add separator
$m add command -label "Force Notification" -command "forceNot; rV"
$m add separator
$m add command -label "Highlight Material" -command "material_highlight"

if { ! $AYWITHAQUA } {
    pack .fu.fMenu.tool -in .fu.fMenu -side left

    # Custom
    menubutton .fu.fMenu.cust -text "Custom" -menu .fu.fMenu.cust.m -padx 3
    menu .fu.fMenu.cust.m -tearoff 0
    pack .fu.fMenu.cust -in .fu.fMenu -side left
    set ay(cm) .fu.fMenu.cust.m
    # Special
    menubutton .fu.fMenu.spec -text "Special" -menu .fu.fMenu.spec.m -padx 3
    set m [menu .fu.fMenu.spec.m -tearoff 0]
    pack .fu.fMenu.spec -in .fu.fMenu -side left

} else {
    # Custom
    set m [menu $mb.mcust -tearoff 0]
    $mb add cascade -label "Custom" -menu $m
    set ay(cm) $m
    # Special
    set m [menu $mb.mspecial -tearoff 0]
    $mb add cascade -label "Special" -menu $m
}
set ay(specialmenu) $m

$m add command -label "Save Selected as" -command "io_saveScene ask 1"
$m add command -label "Save Environment" -command "io_saveEnv"

$m add cascade -menu $m.clp -label "Clipboard"
set sm [menu $m.clp -tearoff 0]
$sm add command -label "Paste (Move)" -command {cmovOb; cS;
global ay; set ay(ul) $ay(CurrentLevel); uS; rV; set ay(sc) 1}
$sm add command -label "Paste Property to Selected" -command "pclip_pastetosel;forceNot;rV"

$m add cascade -menu $m.pnt -label "Points"
set sm [menu $m.pnt -tearoff 0]
$sm add command -label "Select All Points" -command "selPoints;rV"
$sm add command -label "Invert Selection" -command "invSelPoints;rV"
$sm add command -label "Apply Trafo To All Points"\
	-command "undo save ApplyTr; applyTrafo all;\
	plb_update;forceNot;rV"
$sm add command -label "Apply Trafo To Selected Points"\
	-command "undo save ApplyTr; applyTrafo sel;\
	plb_update;forceNot;rV"

$m add cascade -menu $m.ins -label "Instances"
set sm [menu $m.ins -tearoff 0]
$sm add command -label "Resolve all Instances" -command "ai_resolve"
$sm add command -label "Automatic Instancing" -command "ai_open"


$m add cascade -menu $m.tag -label "Tags"
set sm [menu $m.tag -tearoff 0]
$sm add command -label "Add RiOption" -command "riopt_addp"
$sm add command -label "Add RiAttribute" -command "riattr_addp"
$sm add command -label "Edit TexCoords" -command "tc_edit"
$m add separator
#$m add command -label "Create ShadowMaps" -command "riopt_addp"
$m add command -label "Select Renderer" -command "render_select"

$m add separator
$m add cascade -menu $m.rib -label "RIB-Export"
set sm [menu $m.rib -tearoff 0]
$sm add command -label "From Camera" -command "io_exportRIBfC"
$sm add command -label "Selected Objects" -command "io_exportRIBSO"
$sm add command -label "Create ShadowMaps" -command "io_RenderSM"
$m add separator
$m add command -label "Toggle Toolbox" -command "toolbox_toggle"
$m add command -label "Toggle TreeView" -command "tree_toggle"
$m add separator
$m add command -label "Zap Ayam" -command "zap"

if { ! $AYWITHAQUA } {
    # Help
    menubutton .fu.fMenu.hlp -text "Help" -menu .fu.fMenu.hlp.m -padx 3
    set m [menu .fu.fMenu.hlp.m -tearoff 0]
    pack .fu.fMenu.hlp -in .fu.fMenu -side right

} else {
    # Help
    set m [menu $mb.help -tearoff 0]
    $mb add cascade -label "Help" -menu $m
}
set ay(helpmenu) $m

$m add command -label "Help" -command {
    global ayprefs
    browser_urlOpen $ayprefs(Docs) }

$m add command -label "Help on object" -command {
    global ayprefs
    set selected ""
    getSel selected
    if { $selected == "" } { 
	ayError 2 "Help on object" "Please select an object!"
	return;
    }
    getType type
    set type [string tolower $type]
    browser_urlOpen $ayprefs(Docs)ayam-4.html\#${type}obj
}

$m add command -label "Show Shortcuts" -command "shortcut_show"
$m add command -label "About" -command "aboutAyam"
$m add checkbutton -label "Show Tooltips" -variable ayprefs(showtt)

# XXXX Win32 Menus are a bit to tall
global tcl_platform
if { $tcl_platform(platform) == "windows" } {
    set children [winfo children .fu.fMenu]
    foreach child $children {
	$child configure -pady 1
    }
}


# mmenu_addcustom:
#  XXXX add code to avoid creating double entries when
#  custom objects are overloaded?
proc mmenu_addcustom { name command } {
    global ay
    set m $ay(createmenu).cus
#    set m .fu.fMenu.cr.m.cus
    $m add command -label $name -command $command

 return
}
# mmenu_addcustom

return
