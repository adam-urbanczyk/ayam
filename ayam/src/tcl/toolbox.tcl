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
proc toolbox_open { } {
    global ayprefs ayviewshortcuts aymainshortcuts

    set w .tbw
    catch {destroy $w}
    
    toplevel $w -visual truecolor
    wm title $w "Tools"
    wm iconname $w "Tools"

    if { $tcl_platform(platform) == "windows" } {
	wm transient $w .
    }

    foreach i $ayprefs(toolBoxList) {

	if { $i == "solids" } {
	    set f [frame $w.fsol1 ]
	    
	    button $f.bb -image ay_Box_img -padx 0 -pady 0 -command {
		crtOb Box; uCR; sL; rV;}
	    balloon_set $f.bb "create Box"
	    pack $f.bb -in $f -side left
	    button $f.bs -image ay_Sphere_img -padx 0 -pady 0 -command {
		crtOb Sphere; uCR; sL; rV;}
	    balloon_set $f.bs "create Sphere"
	    pack $f.bs -in $f -side left
	    button $f.bcy -image ay_Cylinder_img  -padx 0 -pady 0 -command {
		crtOb Cylinder; uCR; sL; rV;}
	    balloon_set $f.bcy "create Cylinder"
	    pack $f.bcy -in $f -side left
	    button $f.bco -image ay_Cone_img -padx 0 -pady 0 -command {
		crtOb Cone; uCR; sL; rV;}
	    balloon_set $f.bco "create Cone"
	    pack $f.bco -in $f -side left

	    pack $f -in $w -side top -anchor w
	    #########
	    set f [frame $w.fsol2 ]
	    button $f.bd -image ay_Disk_img -padx 0 -pady 0 -command {
		crtOb Disk; uCR; sL; rV;}
	    balloon_set $f.bd "create Disk"
	    pack $f.bd -in $f -side left
	    button $f.bto -image ay_Torus_img -padx 0 -pady 0 -command {
		crtOb Torus; uCR; sL; rV;}
	    balloon_set $f.bto "create Torus"
	    pack $f.bto -in $f -side left
	    button $f.bhy -image ay_Hyperboloid_img -padx 0 -pady 0 -command {
		crtOb Hyperboloid; uCR; sL; rV;}
	    balloon_set $f.bhy "create Hyperboloid"
	    pack $f.bhy -in $f -side left

	    button $f.bpar -image ay_Paraboloid_img -padx 0 -pady 0 -command {
		crtOb Paraboloid; uCR; sL; rV;}
	    balloon_set $f.bpar "create Paraboloid"
	    pack $f.bpar -in $f -side left

	    pack $f -in $w -side top -anchor w
	    #########
	    set f [frame $w.fbarsol -relief raised -bd 2 -height 2]
	    pack $f -in $w -side top -fill x -expand yes
	    #########

	}

	##################

	if { $i == "points" } {

	    set f [frame $w.fp]
	    
	    button $f.bed -image ay_Edit_img -padx 0 -pady 0 -command {
		global ay
		foreach i $ay(views) {
		    ${i}.f3D.togl mc

		    actionEditP ${i}.f3D.togl
		}
	    }
	    balloon_set $f.bed "edit points\n\[ $ayviewshortcuts(Edit) \]"
	    pack $f.bed -in $f -side left
	    ######
	    button $f.bedw -image ay_EditW_img -padx 0 -pady 0 -command {
		global ay
		foreach i $ay(views) {
		    ${i}.f3D.togl mc

		    actionEditWP ${i}.f3D.togl

		}
	    }
	    balloon_set $f.bedw "edit weights\n\[ $ayviewshortcuts(WeightE) \]"
	    pack $f.bedw -in $f -side left
	    ######
	    button $f.bedd -image ay_EditD_img -padx 0 -pady 0 -command {
		global ay
		foreach i $ay(views) {
		    ${i}.f3D.togl mc

		    actionDEditP ${i}.f3D.togl

		}
	    }
	    pack $f.bedd -in $f -side left
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
		actionDelTagP $ay(currentView)
		rV
		break;
	    }
	    balloon_set $f.sel "select (tag) points\n\[ $ayviewshortcuts(Select) \]\nShift: deselect points\n\[ $ayviewshortcuts(DeSelect) \]"
	    pack $f.sel -in $f -side left

	    pack $f -in $w -side top -anchor w

	    set f [frame $w.fbarpoint -relief raised -bd 2 -height 2]
	    pack $f -in $w -side top -fill x -expand yes

	}

	##################
	if { $i == "camera" } {
	    set f [frame $w.fcam]

	    ######
	    button $f.bv -image ay_View_img -padx 0 -pady 0 -command {
		viewOpen 400 300; global ay; set ay(ul) root:0; uS 0 1; rV;
	    }
	    balloon_set $f.bv "new view"
	    pack $f.bv -in $f -side left
	    ######
	    button $f.brot -image ay_RotV_img -padx 0 -pady 0 -command {
		global ay
		foreach i $ay(views) {
		    ${i}.f3D.togl mc

		    actionRotView ${i}.f3D.togl

		}
	    }
	    balloon_set $f.brot "rotate view \[ $ayviewshortcuts(RotV) \]"
	    pack $f.brot -in $f -side left
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
	    pack $f.bmov -in $f -side left
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
	    pack $f.bzom -in $f -side left
	    #####
	    pack $f -in $w -side top -anchor w

	    set f [frame $w.fbarcamera -relief raised -bd 2 -height 2]
	    pack $f -in $w -side top -fill x -expand yes
	}
	##################
	if { $i == "trafo" } {
	    set f [frame $w.ftrafo]

	    ######
	    button $f.bmov -image ay_Move_img -padx 0 -pady 0 -command {
		global ay
		foreach i $ay(views) {
		    ${i}.f3D.togl mc
		    actionMoveOb ${i}.f3D.togl
		}
	    }
	    balloon_set $f.bmov "move \[ $ayviewshortcuts(MoveO) \]"
	    pack $f.bmov -in $f -side left
	    ######
	    button $f.brot -image ay_Rotate_img -padx 0 -pady 0 -command {
		global ay
		foreach i $ay(views) {
		    ${i}.f3D.togl mc
		    actionRotOb ${i}.f3D.togl
		}
	    }
	    balloon_set $f.brot "rotate \[ $ayviewshortcuts(RotO) \]"
	    pack $f.brot -in $f -side left
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
	    pack $f.brota -in $f -side left
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
	    pack $f.bscal -in $f -side left
	    #####
	    pack $f -in $w -side top -anchor w

	    set f [frame $w.fbartrafo -relief raised -bd 2 -height 2]
	    pack $f -in $w -side top -fill x -expand yes
	}
	##################
	if { $i == "trafo2" } {
	    set f [frame $w.ftrafo2]

	    ######
	    button $f.bsc2d -image ay_Scale2D_img -padx 0 -pady 0 -command {
		global ay
		foreach i $ay(views) {
		    ${i}.f3D.togl mc
		    actionSc2DOb ${i}.f3D.togl
		}
	    }
	    balloon_set $f.bsc2d "scale 2D \[ $ayviewshortcuts(Scal2) \]"
	    pack $f.bsc2d -in $f -side left
	    ######
	    button $f.bsc1dx -image ay_Scale1DX_img -padx 0 -pady 0 -command {
		global ay
		foreach i $ay(views) {
		    ${i}.f3D.togl mc
		    actionSc1DXOb ${i}.f3D.togl
		}
	    }
	    balloon_set $f.bsc1dx "scale x \[ $ayviewshortcuts(ScalX) \]"
	    pack $f.bsc1dx -in $f -side left
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
	    pack $f.bsc1dy -in $f -side left
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
	    pack $f.bsc1dz -in $f -side left
	    #####
	    pack $f -in $w -side top -anchor w

	    set f [frame $w.fbartrafo2 -relief raised -bd 2 -height 2]
	    pack $f -in $w -side top -fill x -expand yes
	}

	##################

	if { $i == "misc" } {
	    set f [frame $w.fmisc ]

	    button $f.bhide -image ay_Show_img -padx 0 -pady 0 -command {
		hide -toggle; rV
	    }
	    balloon_set $f.bhide "hide/show object"
	    pack $f.bhide -in $f -side left
	    #####
	    button $f.bconv -image ay_Convert_img -padx 0 -pady 0 -command {
		convOb; uS; rV
	    }
	    balloon_set $f.bconv "convert object"
	    pack $f.bconv -in $f -side left
	    #####
	    button $f.bnot -image ay_Notify_img -padx 0 -pady 0 -command {
		forceNot; rV
	    }
	    balloon_set $f.bnot "force notification"
	    pack $f.bnot -in $f -side left
	    #####
	    button $f.bund -image ay_Undo_img -padx 0 -pady 0 -command {
		.fu.fMenu.ed.m invoke 9
	    }
	    balloon_set $f.bund "undo \[ $aymainshortcuts(Undo) \]"
	    pack $f.bund -in $f -side left

	    #####
	    pack $f -in $w -side top -anchor w
	    #########
	    set f [frame $w.fbarmisc -relief raised -bd 2 -height 2]
	    pack $f -in $w -side top -fill x -expand yes
	    #########

	}
	##################

	if { $i == "misco" } {
	    set f [frame $w.fmisco ]

	    button $f.blevel -image ay_Level_img -padx 0 -pady 0 -command {
		crtOb Level 1; uCR; sL; rV;
	    }
	    balloon_set $f.blevel "create Level"
	    pack $f.blevel -in $f -side left
	    #####
	    button $f.blight -image ay_Light_img -padx 0 -pady 0 -command {
		crtOb Light; uCR; sL; rV;
	    }
	    balloon_set $f.blight "create Light"
	    pack $f.blight -in $f -side left
	    #####
	    button $f.binst -image ay_Instance_img -padx 0 -pady 0 -command {
		crtOb Instance; uCR; sL; rV;
	    }
	    balloon_set $f.binst "create Instance"
	    pack $f.binst -in $f -side left
	    #####
	    button $f.bmat -image ay_Material_img -padx 0 -pady 0 -command {
		material_createp;
	    }
	    balloon_set $f.bmat "create Material"
	    pack $f.bmat -in $f -side left
	    #####
	    pack $f -in $w -side top -anchor w
	    #########
	    set f [frame $w.fbarmisco -relief raised -bd 2 -height 2]
	    pack $f -in $w -side top -fill x -expand yes
	    #########

	}
	##################

	if { $i == "nctools1" } {
	    set f [frame $w.fnct1 ]

	    button $f.bins -image ay_Insert_img -padx 0 -pady 0 -command {
		global ay
		foreach i $ay(views) {
		    ${i}.f3D.togl mc
		    actionInsertP ${i}.f3D.togl
		}
	    }
	    balloon_set $f.bins "insert point \n\[ i \]"
	    pack $f.bins -in $f -side left
	    #####
	    button $f.bdel -image ay_Delete_img -padx 0 -pady 0 -command {
		global ay
		foreach i $ay(views) {
		    ${i}.f3D.togl mc
		    actionDeleteP ${i}.f3D.togl
		}
	    }
	    balloon_set $f.bdel "delete point \n\[ d \]"
	    pack $f.bdel -in $f -side left
	    #####
	    button $f.brev -image ay_Revert_img -padx 0 -pady 0 -command {
		revert; plb_update;rV
	    }
	    balloon_set $f.brev "revert curve"
	    pack $f.brev -in $f -side left
	    #####
	    button $f.bref -image ay_Refine_img -padx 0 -pady 0 -command {
		refineNC; plb_update;rV
	    }
	    balloon_set $f.bref "refine curve"
	    pack $f.bref -in $f -side left

	    #####
	    pack $f -in $w -side top -anchor w
	    #########
	    set f [frame $w.fbarnct1 -relief raised -bd 2 -height 2]
	    pack $f -in $w -side top -fill x -expand yes
	    #########

	}
	##################

	if { $i == "nctools2" } {
	    set f [frame $w.fnct2 ]

	    button $f.bfindu -image ay_FindU_img -padx 0 -pady 0 -command {
		global ay
		foreach i $ay(views) {
		    ${i}.f3D.togl mc
		    actionFindU ${i}.f3D.togl
		}
	    }
	    balloon_set $f.bfindu "find u \[ $ayviewshortcuts(FindU) \]"
	    pack $f.bfindu -in $f -side left
	    #####
	    button $f.bspl -image ay_Split_img -padx 0 -pady 0 -command {
		global ay
		foreach i $ay(views) {
		    ${i}.f3D.togl mc
		    actionSplitNC ${i}.f3D.togl
		}
	    }
	    balloon_set $f.bspl "split curve \[ $ayviewshortcuts(SplitNC) \]"
	    pack $f.bspl -in $f -side left
	    #####
	    button $f.bconc -image ay_Concat_img -padx 0 -pady 0 -command {
		concatNC; uCR;sL;rV
	    }
	    balloon_set $f.bconc "concat curves"
	    pack $f.bconc -in $f -side left
	    #####
	    button $f.bclamp -image ay_Clamp_img -padx 0 -pady 0 -command {
		clampNC; plb_update;rV
	    }
	    balloon_set $f.bclamp "clamp curve"
	    pack $f.bclamp -in $f -side left

	    #####
	    pack $f -in $w -side top -anchor w
	    #########
	    set f [frame $w.fbarnct2 -relief raised -bd 2 -height 2]
	    pack $f -in $w -side top -fill x -expand yes
	    #########

	}
	##################

	if { $i == "nurbs" } {
	    set f [frame $w.fnurb ]

	    button $f.bnc -image ay_NCurve_img -padx 0 -pady 0 -command {
		crtOb NCurve -length $ay(nclen); uCR; sL; rV;
	    }
	    balloon_set $f.bnc "create NCurve"
	    pack $f.bnc -in $f -side left
	    #####
	    button $f.bic -image ay_ICurve_img -padx 0 -pady 0 -command {
		crtOb ICurve -length $ay(iclen); uCR; sL; rV;
	    }
	    balloon_set $f.bic "create ICurve"
	    pack $f.bic -in $f -side left
	    #####
	    button $f.bnci -image ay_NCircle_img -padx 0 -pady 0 -command {
		crtClosedBS $ay(cbspnum); uCR; sL; rV;
	    }
	    balloon_set $f.bnci "create circle"
	    pack $f.bnci -in $f -side left
	    #####
	    button $f.bnp -image ay_NPatch_img -padx 0 -pady 0 -command {
		crtOb NPatch -width $ay(npwidth) -height $ay(npheight);
		uCR; sL; rV;
	    }
	    balloon_set $f.bnp "create NPatch"
	    pack $f.bnp -in $f -side left

	    #####
	    pack $f -in $w -side top -anchor w
	    #########
	    set f [frame $w.fbarnurb -relief raised -bd 2 -height 2]
	    pack $f -in $w -side top -fill x -expand yes
	    #########

	}
	##################

	if { $i == "toolobjs" } {
	    set f [frame $w.ftoolobj -borderwidth 0]

	    button $f.brev -image ay_Revolve_img -padx 0 -pady 0 -command {
		revolve_crt; 
	    }
	    balloon_set $f.brev "create Revolve"
	    pack $f.brev -in $f -side left
	    #####
	    button $f.bex -image ay_Extrude_img -padx 0 -pady 0 -command {
		extrude_crt;
	    }
	    balloon_set $f.bex "create Extrude"
	    pack $f.bex -in $f -side left
	    #####
	    button $f.bswp -image ay_Sweep_img -padx 0 -pady 0 -command {
		sweep_crt;
	    }
	    balloon_set $f.bswp "create Sweep"
	    pack $f.bswp -in $f -side left
	    #####
	    button $f.bskin -image ay_Skin_img -padx 0 -pady 0 -command {
		skin_crt;
	    }
	    balloon_set $f.bskin "create Skin"
	    pack $f.bskin -in $f -side left

	    #####
	    pack $f -in $w -side top -anchor w -fill x -expand yes
	    #########
	    set f [frame $w.fbartoolobj -relief raised -bd 2 -height 2]
	    pack $f -in $w -side top -fill x -expand yes
	    #########

	}
    }

    update
    if { $ayprefs(toolBoxGeom) != "" } {
	winMoveOrResize .tbw $ayprefs(toolBoxGeom)
    }
    

    # establish main shortcuts also for toolbox
    shortcut_main $w

    return;
}
# toolbox_open

# toolbox_toggle:
#  open or close toolbox window
proc toolbox_toggle { } {

    if { [winfo exists .tbw] } {
	catch {destroy .tbw}
    } else {
	toolbox_open
    }

}
# toolbox_toggle
