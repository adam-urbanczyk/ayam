
# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2001 by Randolf Schultz
# (rschultz@informatik.uni-rostock.de) and others.
#
# All rights reserved.
#
# See the file License for details.

# toolbox.tcl - the toolbox

# toolbox_open:
proc toolbox_open { {w .tbw} } {
    global ay ayprefs ayviewshortcuts aymainshortcuts tcl_platform

    if { $w == ".tbw" } {
	catch {destroy $w}
	if { $ay(truecolor) == 1 } {
	    toplevel $w -class ayam -visual truecolor
	} else {
	    toplevel $w -class ayam
	}

	if { $ay(ws) == "Aqua" } {
	    ::tk::unsupported::MacWindowStyle style $w floating\
		{closeBox resizable}
	} else {
	    if { $ayprefs(ToolBoxTrans) == 1 } {
		wm transient $w .
	    }
	}

	wm title $w "Tools"
	wm iconname $w "Tools"
    }
    # if

    set f [frame $w.f -takefocus 0]
    set ay(toolbuttons) {}

    foreach i $ayprefs(toolBoxList) {

	if { $i == "solids" } {
	    lappend ay(toolbuttons) bb bs bcy bco bd bto bhy bpar

	    button $f.bb -image ay_Box_img -padx 0 -pady 0 -command {
		crtOb Box; uCR; if {!$::ay(ctrldown)} sL; rV;}
	    balloon_set $f.bb "create Box"


	    button $f.bs -image ay_Sphere_img -padx 0 -pady 0 -command {
		crtOb Sphere; uCR; if {!$::ay(ctrldown)} sL; rV;}
	    balloon_set $f.bs "create Sphere"

	    button $f.bcy -image ay_Cylinder_img  -padx 0 -pady 0 -command {
		crtOb Cylinder; uCR; if {!$::ay(ctrldown)} sL; rV;}
	    balloon_set $f.bcy "create Cylinder"

	    button $f.bco -image ay_Cone_img -padx 0 -pady 0 -command {
		crtOb Cone; uCR; if {!$::ay(ctrldown)} sL; rV;}
	    balloon_set $f.bco "create Cone"

	    #########

	    button $f.bd -image ay_Disk_img -padx 0 -pady 0 -command {
		crtOb Disk; uCR; if {!$::ay(ctrldown)} sL; rV;}
	    balloon_set $f.bd "create Disk"

	    button $f.bto -image ay_Torus_img -padx 0 -pady 0 -command {
		crtOb Torus; uCR; if {!$::ay(ctrldown)} sL; rV;}
	    balloon_set $f.bto "create Torus"

	    button $f.bhy -image ay_Hyperboloid_img -padx 0 -pady 0 -command {
		crtOb Hyperboloid; uCR; if {!$::ay(ctrldown)} sL; rV;}
	    balloon_set $f.bhy "create Hyperboloid"

	    button $f.bpar -image ay_Paraboloid_img -padx 0 -pady 0 -command {
		crtOb Paraboloid; uCR; if {!$::ay(ctrldown)} sL; rV;}
	    balloon_set $f.bpar "create Paraboloid"

	}

	##################

	if { $i == "points" } {
	    lappend ay(toolbuttons) bed bedw bedd sel

	    button $f.bed -image ay_Edit_img -padx 0 -pady 0 -command {
		global ay
		foreach i $ay(views) {
		    ${i}.f3D.togl mc

		    actionEditP ${i}.f3D.togl
		}
	    }
	    balloon_set $f.bed "edit points\n\[ $ayviewshortcuts(Edit) \]"

	    ######
	    button $f.bedw -image ay_EditW_img -padx 0 -pady 0 -command {
		global ay
		foreach i $ay(views) {
		    ${i}.f3D.togl mc

		    actionEditWP ${i}.f3D.togl

		}
	    }
	    balloon_set $f.bedw "edit weights\n\[ $ayviewshortcuts(WeightE) \]"

	    ######
	    button $f.bedd -image ay_EditD_img -padx 0 -pady 0 -command {
		global ay
		foreach i $ay(views) {
		    ${i}.f3D.togl mc

		    actionDEditP ${i}.f3D.togl

		}
	    }

	    balloon_set $f.bedd "directly edit coordinates\n\[ $ayviewshortcuts(EditD) \]"
	    ######
	    button $f.sel -image ay_Tag_img -padx 0 -pady 0 -command {
		global ay
		foreach i $ay(views) {
		    ${i}.f3D.togl mc

		    actionTagP ${i}.f3D.togl

		}
	    }
	    bind $f.sel <Shift-ButtonPress-1> {
		global  ay
		%W configure -relief sunken
		actionDelTagP $ay(currentView)
		rV
		after 100 "%W configure -relief raised"
		break;
	    }
	    balloon_set $f.sel "select (tag) points\n\[ $ayviewshortcuts(Select) \]\nShift: deselect points\n\[ $ayviewshortcuts(DeSelect) \]"


	}

	##################
	if { $i == "camera" } {
	    lappend ay(toolbuttons) bv brot bmov bzom

	    ######
	    button $f.bv -image ay_View_img -padx 0 -pady 0 -command {
		viewOpen 400 300; global ay; set ay(ul) root:0; uS 0 1; rV;
	    }
	    balloon_set $f.bv "new view"

	    ######
	    button $f.brot -image ay_RotV_img -padx 0 -pady 0 -command {
		global ay
		foreach i $ay(views) {
		    ${i}.f3D.togl mc

		    actionRotView ${i}.f3D.togl

		}
	    }
	    balloon_set $f.brot "rotate view \[ $ayviewshortcuts(RotV) \]"

	    ######
	    button $f.bmov -image ay_MoveV_img -padx 0 -pady 0 -command {
		global ay
		foreach i $ay(views) {
		    ${i}.f3D.togl mc

		    actionMoveView ${i}.f3D.togl

		}
	    }
	    balloon_set $f.bmov\
		    "move view \[ $ayviewshortcuts(MoveV) \]"

	    #####
	    button $f.bzom -image ay_ZoomV_img -padx 0 -pady 0 -command {
		global ay
		foreach i $ay(views) {
		    ${i}.f3D.togl mc

		    actionZoomView ${i}.f3D.togl
		}
	    }
	    balloon_set $f.bzom\
		    "zoom view \[ $ayviewshortcuts(ZoomV) \]"

	}
	##################
	if { $i == "trafo" } {
	    lappend ay(toolbuttons) bmovo broto brota bscal
	    ######
	    button $f.bmovo -image ay_Move_img -padx 0 -pady 0 -command {
		global ay
		foreach i $ay(views) {
		    ${i}.f3D.togl mc
		    actionMoveOb ${i}.f3D.togl
		}
	    }
	    balloon_set $f.bmovo "move \[ $ayviewshortcuts(MoveO) \]"

	    ######
	    button $f.broto -image ay_Rotate_img -padx 0 -pady 0 -command {
		global ay
		foreach i $ay(views) {
		    ${i}.f3D.togl mc
		    actionRotOb ${i}.f3D.togl
		}
	    }
	    balloon_set $f.broto "rotate \[ $ayviewshortcuts(RotO) \]"

	    ######
	    button $f.brota -image ay_RotateA_img -padx 0 -pady 0 -command {
		global ay
		foreach i $ay(views) {
		    ${i}.f3D.togl mc
		    actionRotObA ${i}.f3D.togl
		}
	    }
	    balloon_set $f.brota\
		    "rotate about \[ $ayviewshortcuts(RotA) \]"

	    #####
	    button $f.bscal -image ay_Scale3D_img -padx 0 -pady 0 -command {
		global ay
		foreach i $ay(views) {
		    ${i}.f3D.togl mc
		    actionSc3DOb ${i}.f3D.togl
		}
	    }
	    balloon_set $f.bscal\
		    "scale 3D \[ $ayviewshortcuts(Scal3) \]"

	}
	##################
	if { $i == "trafo2" } {
	    lappend ay(toolbuttons) bsc2d bsc1dx bsc1dy bsc1dz

	    ######
	    button $f.bsc2d -image ay_Scale2D_img -padx 0 -pady 0 -command {
		global ay
		foreach i $ay(views) {
		    ${i}.f3D.togl mc
		    actionSc2DOb ${i}.f3D.togl
		}
	    }
	    balloon_set $f.bsc2d "scale 2D \[ $ayviewshortcuts(Scal2) \]"

	    ######
	    button $f.bsc1dx -image ay_Scale1DX_img -padx 0 -pady 0 -command {
		global ay
		foreach i $ay(views) {
		    ${i}.f3D.togl mc
		    actionSc1DXOb ${i}.f3D.togl
		}
	    }
	    balloon_set $f.bsc1dx "scale x \[ $ayviewshortcuts(ScalX) \]"

	    ######
	    button $f.bsc1dy -image ay_Scale1DY_img -padx 0 -pady 0 -command {
		global ay
		foreach i $ay(views) {
		    ${i}.f3D.togl mc
		    actionSc1DYOb ${i}.f3D.togl
		}
	    }
	    balloon_set $f.bsc1dy\
		    "scale y \[ $ayviewshortcuts(ScalY) \]"

	    #####
	    button $f.bsc1dz -image ay_Scale1DZ_img -padx 0 -pady 0 -command {
		global ay
		foreach i $ay(views) {
		    ${i}.f3D.togl mc
		    actionSc1DZOb ${i}.f3D.togl
		}
	    }
	    balloon_set $f.bsc1dz\
		    "scale z \[ $ayviewshortcuts(ScalZ) \]"

	}

	##################

	if { $i == "misc" } {
	    lappend ay(toolbuttons) bhide bconv bnot bund
	    button $f.bhide -image ay_Show_img -padx 0 -pady 0 -command {
		global ay
		undo save HidSho
		hide -toggle
		set ay(ul) $ay(CurrentLevel); uS 1 1; rV
	    }
	    balloon_set $f.bhide "hide/show object"

	    #####
	    button $f.bconv -image ay_Convert_img -padx 0 -pady 0 -command {
		global ay
		set m $ay(toolsmenu)
		$m invoke 17
	    }
	    bind $f.bconv <Shift-ButtonPress-1> {
		global  ay
		%W configure -relief sunken
		set m $ay(toolsmenu)
		$m invoke 18
		after 100 "%W configure -relief raised"
		break;
	    }
	    balloon_set $f.bconv\
		    "convert object\nShift: convert object in place"

	    #####
	    button $f.bnot -image ay_Notify_img -padx 0 -pady 0 -command {
		forceNot; rV
	    }
	    bind $f.bnot <Shift-ButtonPress-1> {
		%W configure -relief sunken
		forceNot all
		rV
		after 100 "%W configure -relief raised"
		break;
	    }
	    balloon_set $f.bnot\
		    "force notification\nShift: complete notification"
	    #####
	    button $f.bund -image ay_Undo_img -padx 0 -pady 0 -command {
		global ay
		set m $ay(editmenu)
		$m invoke 9
	    }
	    balloon_set $f.bund "undo \[ $aymainshortcuts(Undo) \]"

	}
	##################

	if { $i == "misco" } {
	    lappend ay(toolbuttons) blevel blight binst bmat
	    button $f.blevel -image ay_Level_img -padx 0 -pady 0 -command {
		crtOb Level 1; uCR; if {!$::ay(ctrldown)} sL; rV;
	    }
	    balloon_set $f.blevel "create Level\nShift: and move objects into"
	    bind $f.blevel <Shift-ButtonPress-1> {
		%W configure -relief sunken
		level_crt Level
		after 100 "%W configure -relief raised"
		break;
	    }
	    bind $f.blevel <Control-Shift-ButtonPress-1> {
		%W configure -relief sunken
		level_crt Level "" 1
		after 100 "%W configure -relief raised"
		break;
	    }
	    #####
	    button $f.blight -image ay_Light_img -padx 0 -pady 0 -command {
		crtOb Light; uCR; if {!$::ay(ctrldown)} sL; rV;
	    }
	    balloon_set $f.blight "create Light"

	    #####
	    button $f.binst -image ay_Instance_img -padx 0 -pady 0 -command {
		crtOb Instance; uCR; if {!$::ay(ctrldown)} sL; rV;
	    }
	    balloon_set $f.binst "create Instance"

	    #####
	    button $f.bmat -image ay_Material_img -padx 0 -pady 0 -command {
		material_createp;
	    }
	    balloon_set $f.bmat "create Material"

	}
	##################

	if { $i == "nctools1" } {
	    lappend ay(toolbuttons) bins bdel brev bref
	    button $f.bins -image ay_Insert_img -padx 0 -pady 0 -command {
		global ay
		foreach i $ay(views) {
		    ${i}.f3D.togl mc
		    actionInsertP ${i}.f3D.togl
		}
	    }
	    balloon_set $f.bins "insert point \n\[ i \]"

	    #####
	    button $f.bdel -image ay_Delete_img -padx 0 -pady 0 -command {
		global ay
		foreach i $ay(views) {
		    ${i}.f3D.togl mc
		    actionDeleteP ${i}.f3D.togl
		}
	    }
	    balloon_set $f.bdel "delete point \n\[ d \]"

	    #####
	    button $f.brev -image ay_Revert_img -padx 0 -pady 0 -command {
		undo save Revert; revert; plb_update; rV
	    }
	    balloon_set $f.brev "revert curve"

	    #####
	    button $f.bref -image ay_Refine_img -padx 0 -pady 0 -command {
		undo save Refine; refineNC; plb_update; rV
	    }
	    balloon_set $f.bref "refine curve\nShift: coarsen curve"
	    bind $f.bref <Shift-ButtonPress-1> {
		%W configure -relief sunken
		undo save Coarsen; coarsenNC; plb_update; rV
		after 100 "%W configure -relief raised"
		break;
	    }

	}
	##################

	if { $i == "nctools2" } {
	    lappend ay(toolbuttons) bfindu bspl bconc bclamp

	    button $f.bfindu -image ay_FindU_img -padx 0 -pady 0 -command {
		global ay
		foreach i $ay(views) {
		    ${i}.f3D.togl mc
		    actionFindU ${i}.f3D.togl
		}
	    }
	    balloon_set $f.bfindu "find u \[ $ayviewshortcuts(FindU) \]"

	    #####
	    button $f.bspl -image ay_Split_img -padx 0 -pady 0 -command {
		global ay
		foreach i $ay(views) {
		    ${i}.f3D.togl mc
		    actionSplitNC ${i}.f3D.togl
		}
	    }
	    balloon_set $f.bspl "split curve \[ $ayviewshortcuts(SplitNC) \]"

	    #####
	    button $f.bconc -image ay_Concat_img -padx 0 -pady 0 -command {
		level_crt ConcatNC
	    }
	    balloon_set $f.bconc "create ConcatNC\n(concat curves)"
	    bind $f.bconc <Control-ButtonPress-1> {
		%W configure -relief sunken
		level_crt ConcatNC "" 1
		after 100 "%W configure -relief raised"
		break;
	    }
	    #####
	    button $f.bclamp -image ay_Clamp_img -padx 0 -pady 0 -command {
		undo save Clamp; clampNC; plb_update; rV
	    }
	    balloon_set $f.bclamp "clamp curve"

	}
	##################

	if { $i == "nurbs" } {
	    lappend ay(toolbuttons) bnc bic bnci bnp

	    button $f.bnc -image ay_NCurve_img -padx 0 -pady 0 -command {
		crtOb NCurve -length $ay(nclen); uCR;
		if {!$::ay(ctrldown)} sL; forceNot; rV;
	    }
	    balloon_set $f.bnc "create NCurve"

	    #####
	    button $f.bic -image ay_ICurve_img -padx 0 -pady 0 -command {
		crtOb ICurve -length $ay(iclen); uCR;
		if {!$::ay(ctrldown)} sL; forceNot; rV;
	    }
	    balloon_set $f.bic "create ICurve"

	    #####
	    button $f.bnci -image ay_NCircle_img -padx 0 -pady 0 -command {
		crtClosedBS $ay(cbspnum) $ay(cbsporder); uCR; sL; forceNot; rV;
	    }
	    bind $f.bnci <Shift-ButtonPress-1> {
		global  ay
		%W configure -relief sunken
		crtOb NCircle; uCR; if {!$::ay(ctrldown)} sL; forceNot; rV;
		after 100 "%W configure -relief raised"
		break;
	    }
	    balloon_set $f.bnci \
		"create circular B-Spline\nShift: create NCircle object"

	    #####
	    button $f.bnp -image ay_NPatch_img -padx 0 -pady 0 -command {
		crtOb NPatch -width $ay(npwidth) -height $ay(npheight);
		uCR; if {!$::ay(ctrldown)} sL; forceNot; rV;
	    }
	    balloon_set $f.bnp "create NPatch"

	}
	##################

	if { $i == "toolobjs" } {
	    lappend ay(toolbuttons) brevo bex bswp bcap

	    button $f.brevo -image ay_Revolve_img -padx 0 -pady 0 -command {
		level_crt Revolve;
	    }
	    balloon_set $f.brevo "create Revolve"
	    bind $f.brevo <Control-ButtonPress-1> {
		%W configure -relief sunken
		level_crt Revolve "" 1
		after 100 "%W configure -relief raised"
		break;
	    }
	    #####
	    button $f.bex -image ay_Extrude_img -padx 0 -pady 0 -command {
		level_crt Extrude;
	    }
	    balloon_set $f.bex "create Extrude"
	    bind $f.bex <Control-ButtonPress-1> {
		%W configure -relief sunken
		level_crt Extrude "" 1
		after 100 "%W configure -relief raised"
		break;
	    }
	    #####
	    button $f.bswp -image ay_Sweep_img -padx 0 -pady 0 -command {
		level_crt Sweep;
	    }
	    balloon_set $f.bswp "create Sweep"
	    bind $f.bswp <Control-ButtonPress-1> {
		%W configure -relief sunken
		level_crt Sweep "" 1
		after 100 "%W configure -relief raised"
		break;
	    }
	    #####
	    button $f.bcap -image ay_Cap_img -padx 0 -pady 0 -command {
		level_crt Cap;
	    }
	    balloon_set $f.bcap "create Cap"
	    bind $f.bcap <Control-ButtonPress-1> {
		%W configure -relief sunken
		level_crt Cap "" 1
		after 100 "%W configure -relief raised"
		break;
	    }

	}
	##################

	if { $i == "toolobjs2" } {
	    lappend ay(toolbuttons) bbirail1 bbirail2 bgord bskin

	    #####
	    button $f.bbirail1 -image ay_Birail1_img -padx 0 -pady 0 -command {
		level_crt Birail1;
	    }
	    balloon_set $f.bbirail1 "create Birail1"
	    bind $f.bbirail1 <Control-ButtonPress-1> {
		%W configure -relief sunken
		level_crt Birail1 "" 1
		after 100 "%W configure -relief raised"
		break;
	    }
	    #####
	    button $f.bbirail2 -image ay_Birail2_img -padx 0 -pady 0 -command {
		level_crt Birail2;
	    }
	    balloon_set $f.bbirail2 "create Birail2"
	    bind $f.bbirail2 <Control-ButtonPress-1> {
		%W configure -relief sunken
		level_crt Birail2 "" 1
		after 100 "%W configure -relief raised"
		break;
	    }
	    button $f.bgord -image ay_Gordon_img -padx 0 -pady 0 -command {
		level_crt Gordon;
	    }
	    balloon_set $f.bgord "create Gordon"
	    bind $f.bgord <Control-ButtonPress-1> {
		%W configure -relief sunken
		level_crt Gordon "" 1
		after 100 "%W configure -relief raised"
		break;
	    }
	    #####
	    button $f.bskin -image ay_Skin_img -padx 0 -pady 0 -command {
		level_crt Skin;
	    }
	    balloon_set $f.bskin "create Skin"
	    bind $f.bskin <Control-ButtonPress-1> {
		%W configure -relief sunken
		level_crt Skin "" 1
		after 100 "%W configure -relief raised"
		break;
	    }
	}
	##################

	if { $i == "nptools1" } {
	    lappend ay(toolbuttons) bextrnc bswpuv brevu brevv

	    button $f.bextrnc -image ay_NPExtrNC_img -padx 0 -pady 0 -command {
		level_crt ExtrNC;
	    }
	    balloon_set $f.bextrnc "extract NURBCurve"
	    bind $f.bextrnc <Control-ButtonPress-1> {
		%W configure -relief sunken
		level_crt ExtrNC "" 1
		after 100 "%W configure -relief raised"
		break;
	    }

	    #####
	    button $f.bswpuv -image ay_NPSwapUV_img -padx 0 -pady 0 -command {
		undo save SwapUV; swapUV; plb_update; rV
	    }
	    balloon_set $f.bswpuv "swap UV"

	    #####
	    button $f.brevu -image ay_NPRevU_img -padx 0 -pady 0 -command {
		undo save RevertU; revertU; plb_update; rV
	    }
	    balloon_set $f.brevu "revert U"

	    #####
	    button $f.brevv -image ay_NPRevV_img -padx 0 -pady 0 -command {
		undo save RevertV; revertV; plb_update; rV
	    }
	    balloon_set $f.brevv "revert V"

	}
    }
    # foreach

    update
    
    if { $w == ".tbw" } {
	if { $ayprefs(toolBoxGeom) != "" } {
	    winMoveOrResize .tbw $ayprefs(toolBoxGeom)
	} else {
	    set size [winfo reqwidth $w.f.[lindex $ay(toolbuttons) 0]]
	    set width [expr 4*$size]
	    set height [expr [llength $ay(toolbuttons)]/4*$size]
	    winMoveOrResize .tbw ${width}x${height}
	}
	update
    }
    # if

    # establish main shortcuts also for toolbox
    shortcut_main $w

    if { $w == ".tbw" } {
	global ayviewshortcuts
	bind $w <[repcont $ayviewshortcuts(Close)]> "destroy .tbw"

	wm protocol $w WM_DELETE_WINDOW {
	    global ayprefs
	    set ayprefs(showtb) 0
	    destroy .tbw
	}
    }

    toolbox_layout $w

 return;
}
# toolbox_open


# toolbox_layout:
#  arrange the buttons in rows/columns according
#  to window size
proc toolbox_layout { {w ".tbw"} } {
    global ay ayprefs

    if { $ay(tblayoutsema) == 1 } {
	return;
    } else {
	update
	set ay(tblayoutsema) 1
	update
    }

    if { ! [winfo exists $w] } { return; }

    bind $w <Configure> ""

    set size [winfo reqwidth $w.f.[lindex $ay(toolbuttons) 0]]
    set rows [expr round([winfo height $w] / $size)]
    set columns [expr round([winfo width $w] / $size)]
    if { $columns < 1 } { set columns 1 }
    set numb [llength $ay(toolbuttons)]

    if { [expr $rows*$columns] < $numb } {
	if { $w == ".tbw" } {
	    ayError 2 toolbox_layout "Can not display all buttons! Resizing..."
	    set height [expr ceil(double($numb)/$columns)*$size]
	    winMoveOrResize .tbw [winfo reqwidth $w]x${height}
	    update
	    set rows [expr round([winfo height $w] / $size)]
	} else {
	    set height [expr ceil(double($numb)/$columns)*$size]
	    $w conf -height $height
	    update
	    set rows [expr round([winfo height $w] / $size)]
	}
    }

    foreach button $ay(toolbuttons) {
	grid forget $w.f.$button
    }
    pack forget $w.f
    update
    set bi 0
    for { set i 0 } { $i < $rows } { incr i} {
	for { set j 0 } { $j < $columns } { incr j} {
	    if { $bi < [llength $ay(toolbuttons)] } {
		grid $w.f.[lindex $ay(toolbuttons) $bi] -row $i -column $j
	    }
	    incr bi
	}
    }
    grid propagate $w.f yes
    pack $w.f -fill both -expand no
    update

    # shrink-wrap the toplevel around buttons
    if { $w == ".tbw" } {
	if { $ayprefs(ToolBoxShrink) } {
	    winMoveOrResize $w [winfo reqwidth $w.f]x[winfo reqheight $w.f]
	    update
	}
    }

    set ay(tbw) [winfo width $w]
    set ay(tbh) [winfo height $w]

    bind $w <Configure> { if { $ay(tbw) != %w ||\
	        $ay(tbh) != %h } { toolbox_layout %W} }

    update
    set ay(tblayoutsema) 0
    update

 return;
}
# toolbox_layout


# toolbox_toggle:
#  open or close toolbox window
proc toolbox_toggle { } {
    global ayprefs

    if { [winfo exists .tbw] } {
	catch {destroy .tbw}
	set ayprefs(showtb) 0
    } else {
	toolbox_open
	set ayprefs(showtb) 1
    }

 return;
}
# toolbox_toggle
