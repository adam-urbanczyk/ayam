# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2008 by Randolf Schultz
# (randolf.schultz@gmail.com) and others.
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
	    toplevel $w -class Ayam -visual truecolor
	} else {
	    toplevel $w -class Ayam
	}

	if { $ay(ws) == "Aqua" } {
	    winMakeFloat $w
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
    set ay(tbw) $f

    foreach i $ayprefs(toolBoxList) {

	if { $i == "solids" } {
	    lappend ay(toolbuttons) bb bs bcy bco bd bto bhy bpar

	    button $f.bb -image ay_Box_img -padx 0 -pady 0 -command {
		crtOb Box; uCR; sL; rV;}
	    bind $f.bb <Control-ButtonPress-1> {
		%W configure -relief sunken
		crtOb Box; uCR; rV;
		after 100 "%W configure -relief raised"
		break;
	    }
	    balloon_set $f.bb "create Box"

	    button $f.bs -image ay_Sphere_img -padx 0 -pady 0 -command {
		crtOb Sphere; uCR; sL; rV;}
	    bind $f.bs <Control-ButtonPress-1> {
		%W configure -relief sunken
		crtOb Sphere; uCR; rV;
		after 100 "%W configure -relief raised"
		break;
	    }
	    balloon_set $f.bs "create Sphere"

	    button $f.bcy -image ay_Cylinder_img  -padx 0 -pady 0 -command {
		crtOb Cylinder; uCR; sL; rV;}
	    bind $f.bcy <Control-ButtonPress-1> {
		%W configure -relief sunken
		crtOb Cylinder; uCR; rV;
		after 100 "%W configure -relief raised"
		break;
	    }
	    balloon_set $f.bcy "create Cylinder"

	    button $f.bco -image ay_Cone_img -padx 0 -pady 0 -command {
		crtOb Cone; uCR; sL; rV;}
	    bind $f.bco <Control-ButtonPress-1> {
		%W configure -relief sunken
		crtOb Cone; uCR; rV;
		after 100 "%W configure -relief raised"
		break;
	    }
	    balloon_set $f.bco "create Cone"

	    #########

	    button $f.bd -image ay_Disk_img -padx 0 -pady 0 -command {
		crtOb Disk; uCR; sL; rV;}
	    bind $f.bd <Control-ButtonPress-1> {
		%W configure -relief sunken
		crtOb Disk; uCR; rV;
		after 100 "%W configure -relief raised"
		break;
	    }
	    balloon_set $f.bd "create Disk"

	    button $f.bto -image ay_Torus_img -padx 0 -pady 0 -command {
		crtOb Torus; uCR; sL; rV;}
	    bind $f.bto <Control-ButtonPress-1> {
		%W configure -relief sunken
		crtOb Torus; uCR; rV;
		after 100 "%W configure -relief raised"
		break;
	    }
	    balloon_set $f.bto "create Torus"

	    button $f.bhy -image ay_Hyperboloid_img -padx 0 -pady 0 -command {
		crtOb Hyperboloid; uCR; sL; rV;}
	    bind $f.bhy <Control-ButtonPress-1> {
		%W configure -relief sunken
		crtOb Hyperboloid; uCR; rV;
		after 100 "%W configure -relief raised"
		break;
	    }
	    balloon_set $f.bhy "create Hyperboloid"

	    button $f.bpar -image ay_Paraboloid_img -padx 0 -pady 0 -command {
		crtOb Paraboloid; uCR; sL; rV;}
	    bind $f.bpar <Control-ButtonPress-1> {
		%W configure -relief sunken
		crtOb Paraboloid; uCR; rV;
		after 100 "%W configure -relief raised"
		break;
	    }
	    balloon_set $f.bpar "create Paraboloid"

	}

	##################

	if { $i == "points" } {
	    lappend ay(toolbuttons) bed bedw bedd sel

	    button $f.bed -image ay_Edit_img -padx 0 -pady 0 -command {
		toolbox_startaction actionEditP
	    }
	    balloon_set $f.bed\
		"edit points <[remkpkr $ayviewshortcuts(Edit)]>"

	    ######
	    button $f.bedw -image ay_EditW_img -padx 0 -pady 0 -command {
		toolbox_startaction actionEditWP
	    }
	    balloon_set $f.bedw\
		"edit weights <[remkpkr $ayviewshortcuts(WeightE)]>"

	    ######
	    button $f.bedd -image ay_EditN_img -padx 0 -pady 0 -command {
		toolbox_startaction actionEditNumP
	    }

	    balloon_set $f.bedd\
	  "edit points numeric <[remkpkr $ayviewshortcuts(EditN)]>"
	    ######
	    button $f.sel -image ay_Tag_img -padx 0 -pady 0 -command {
		toolbox_startaction actionTagP
	    }
	    bind $f.sel <Shift-ButtonPress-1> {
		global  ay
		%W configure -relief sunken
		selPnts;
		rV
		after 100 "%W configure -relief raised"
		break;
	    }
	    balloon_set $f.sel "select (tag) points <[remkpkr $ayviewshortcuts(Select)]>\nShift: deselect points <[remkpkr $ayviewshortcuts(DeSelect)]>"


	}

	##################
	if { $i == "camera" } {
	    lappend ay(toolbuttons) bv brot bmov bzom

	    ######
	    button $f.bv -image ay_View_img -padx 0 -pady 0 -command {
		viewOpen 400 300; global ay; set ay(ul) root:0; uS 0 1; rV;
		after idle viewMouseToCurrent
	    }
	    balloon_set $f.bv "new view"

	    ######
	    button $f.brot -image ay_RotV_img -padx 0 -pady 0 -command {
		toolbox_startaction actionRotView
	    }
	    balloon_set $f.brot\
		"rotate view <[remkpkr $ayviewshortcuts(RotV)]>"

	    ######
	    button $f.bmov -image ay_MoveV_img -padx 0 -pady 0 -command {
		toolbox_startaction actionMoveView
	    }
	    balloon_set $f.bmov\
		"move view <[remkpkr $ayviewshortcuts(MoveV)]>"

	    #####
	    button $f.bzom -image ay_ZoomV_img -padx 0 -pady 0 -command {
		toolbox_startaction actionZoomView
	    }
	    balloon_set $f.bzom\
		"zoom view <[remkpkr $ayviewshortcuts(ZoomV)]>"

	}
	##################
	if { $i == "trafo" } {
	    lappend ay(toolbuttons) bmovo broto bscal bsc2d
	    ######
	    button $f.bmovo -image ay_Move_img -padx 0 -pady 0 -command {
		toolbox_startaction actionMoveOb
	    }
	    balloon_set $f.bmovo\
		"move <[remkpkr $ayviewshortcuts(MoveO)]>"

	    ######
	    button $f.broto -image ay_Rotate_img -padx 0 -pady 0 -command {
		toolbox_startaction actionRotOb
	    }
	    balloon_set $f.broto\
		"rotate <[remkpkr $ayviewshortcuts(RotO)]>"

	    #####
	    button $f.bscal -image ay_Scale3D_img -padx 0 -pady 0 -command {
		toolbox_startaction actionSc3DOb
	    }
	    balloon_set $f.bscal\
		"scale 3D <[remkpkr $ayviewshortcuts(Scal3)]>"

	    ######
	    button $f.bsc2d -image ay_Scale2D_img -padx 0 -pady 0 -command {
		toolbox_startaction actionSc2DOb
	    }
	    balloon_set $f.bsc2d\
		"scale 2D <[remkpkr $ayviewshortcuts(Scal2)]>"

	}
	##################
	if { $i == "trafo2" } {
	    lappend ay(toolbuttons) bsc1dx bsc1dy bsc1dz bst2d

	    ######
	    button $f.bsc1dx -image ay_Scale1DX_img -padx 0 -pady 0 -command {
		toolbox_startaction actionSc1DXOb
	    }
	    balloon_set $f.bsc1dx\
		"scale x <[remkpkr $ayviewshortcuts(Scal2)][remkpkr $ayviewshortcuts(RestrictX)]>"

	    ######
	    button $f.bsc1dy -image ay_Scale1DY_img -padx 0 -pady 0 -command {
		toolbox_startaction actionSc1DYOb
	    }
	    balloon_set $f.bsc1dy\
		"scale y <[remkpkr $ayviewshortcuts(Scal2)][remkpkr $ayviewshortcuts(RestrictY)]>"

	    #####
	    button $f.bsc1dz -image ay_Scale1DZ_img -padx 0 -pady 0 -command {
		toolbox_startaction actionSc1DZOb
	    }
	    balloon_set $f.bsc1dz\
		"scale z <[remkpkr $ayviewshortcuts(Scal2)][remkpkr $ayviewshortcuts(RestrictZ)]>"

	    ######
	    button $f.bst2d -image ay_Stretch2D_img -padx 0 -pady 0 -command {
		toolbox_startaction actionStr2DOb
	    }
	    balloon_set $f.bst2d\
		"stretch <[remkpkr $ayviewshortcuts(Stretch)]>"

	}
	##################

	if { $i == "trafo3" } {
	    lappend ay(toolbuttons) brota bsc1dxa bsc1dya bsc1dza

	    ######
	    button $f.brota -image ay_RotateA_img -padx 0 -pady 0 -command {
		toolbox_startaction actionRotOb 1
	    }
	    balloon_set $f.brota\
		"rotate about <[remkpkr $ayviewshortcuts(RotO)][remkpkr $ayviewshortcuts(About)]>"

	    ######
	    button $f.bsc1dxa -image ay_Scale1DXA_img -padx 0 -pady 0\
             -command {
		toolbox_startaction actionScale1DX 1
	    }
	    balloon_set $f.bsc1dxa\
		"scale 1D X about <[remkpkr $ayviewshortcuts(Scal2)][remkpkr $ayviewshortcuts(RestrictX)][remkpkr $ayviewshortcuts(About)]>"
	    ######
	    button $f.bsc1dya -image ay_Scale1DYA_img -padx 0 -pady 0\
             -command {
		toolbox_startaction actionScale1DY 1
	    }
	    balloon_set $f.bsc1dya\
		"scale 1D Y about <[remkpkr $ayviewshortcuts(Scal2)][remkpkr $ayviewshortcuts(RestrictY)][remkpkr $ayviewshortcuts(About)]>"
	    ######
	    button $f.bsc1dza -image ay_Scale1DZA_img -padx 0 -pady 0\
             -command {
		toolbox_startaction actionScale1DZ 1
	    }
	    balloon_set $f.bsc1dza\
		"scale 1D Z about <[remkpkr $ayviewshortcuts(Scal2)][remkpkr $ayviewshortcuts(RestrictZ)][remkpkr $ayviewshortcuts(About)]>"
	}
	##################

	if { $i == "misc" } {
	    lappend ay(toolbuttons) bhide bconv bnot bund
	    button $f.bhide -image ay_Show_img -padx 0 -pady 0 -command {
		global ay
		undo save HidSho
		hideOb -toggle
		set ay(ul) $ay(CurrentLevel); uS 1 1; rV
	    }
	    balloon_set $f.bhide "hide/show object"

	    #####
	    button $f.bconv -image ay_Convert_img -padx 0 -pady 0 -command {
		global ay
		set m $ay(toolsmenu)
		$m invoke 16
	    }
	    bind $f.bconv <Shift-ButtonPress-1> {
		global  ay
		%W configure -relief sunken
		set m $ay(toolsmenu)
		$m invoke 17
		after 100 "%W configure -relief raised"
		break;
	    }
	    balloon_set $f.bconv\
		    "convert object\nShift: convert object in place"

	    #####
	    button $f.bnot -image ay_Notify_img -padx 0 -pady 0 -command {
		notifyOb; rV
	    }
	    bind $f.bnot <Shift-ButtonPress-1> {
		%W configure -relief sunken
		notifyOb -all
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
		$m invoke 12
	    }
	    balloon_set $f.bund "undo <[remkpkr $aymainshortcuts(Undo)]>"

	}
	##################

	if { $i == "misco" } {
	    lappend ay(toolbuttons) blevel blight binst bmat
	    button $f.blevel -image ay_Level_img -padx 0 -pady 0 -command {
		crtOb Level 1; uCR; sL; rV;
	    }
	    bind $f.blevel <Control-ButtonPress-1> {
		%W configure -relief sunken
		crtOb Level 1; uCR; rV;
		after 100 "%W configure -relief raised"
		break;
	    }
	    bind $f.blevel <Shift-ButtonPress-1> {
		%W configure -relief sunken
		level_crt Level 1
		after 100 "%W configure -relief raised"
		break;
	    }
	    bind $f.blevel <Control-Shift-ButtonPress-1> {
		%W configure -relief sunken
		level_crt Level 1 1
		after 100 "%W configure -relief raised"
		break;
	    }
	    balloon_set $f.blevel "create Level\nShift: and move objects into"
	    #####
	    button $f.blight -image ay_Light_img -padx 0 -pady 0 -command {
		crtOb Light; uCR; sL; rV;
	    }
	    bind $f.blight <Control-ButtonPress-1> {
		%W configure -relief sunken
		crtOb Light; uCR; rV;
		after 100 "%W configure -relief raised"
		break;
	    }
	    balloon_set $f.blight "create Light"

	    #####
	    button $f.binst -image ay_Instance_img -padx 0 -pady 0 -command {
		crtInstances; uCR; sL; rV;
	    }
	    bind $f.binst <Control-ButtonPress-1> {
		%W configure -relief sunken
		crtInstances; uCR; rV;
		after 100 "%W configure -relief raised"
		break;
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
		toolbox_startaction actionInsertP
	    }
	    balloon_set $f.bins "insert point \n<i>"

	    #####
	    button $f.bdel -image ay_Delete_img -padx 0 -pady 0 -command {
		toolbox_startaction actionDeleteP
	    }
	    balloon_set $f.bdel "delete point \n<d>"

	    #####
	    button $f.brev -image ay_Revert_img -padx 0 -pady 0 -command {
		undo save Revert; revertC; plb_update; rV
	    }
	    balloon_set $f.brev "revert curve"

	    #####
	    button $f.bref -image ay_Refine_img -padx 0 -pady 0 -command {
		undo save Refine; refineC; plb_update; rV
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
		toolbox_startaction actionFindU
	    }
	    balloon_set $f.bfindu\
		"find u <[remkpkr $ayviewshortcuts(FindU)]>"

	    #####
	    button $f.bspl -image ay_Split_img -padx 0 -pady 0 -command {
		toolbox_startaction actionSplitNC
	    }
	    balloon_set $f.bspl\
		"split curve <[remkpkr $ayviewshortcuts(SplitNC)]>"

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
		undo save ClampNC; clampNC; plb_update; rV
	    }
	    bind $f.bclamp <Shift-ButtonPress-1> {
		global  ay
		%W configure -relief sunken
		undo save UnclampNC; unclampNC; plb_update; rV
		after 100 "%W configure -relief raised"
		break;
	    }
	    balloon_set $f.bclamp "clamp curve\nShift: unclamp curve"

	}
	##################

	if { $i == "nurbs" } {
	    lappend ay(toolbuttons) bnc bic bnci bnp

	    button $f.bnc -image ay_NCurve_img -padx 0 -pady 0 -command {
		if { $ay(ncadda) == "" } {
		    crtOb NCurve -length $ay(nclen);
		} else {
		    eval crtOb NCurve -length $ay(nclen) $ay(ncadda);
		}
		uCR; sL; notifyOb; rV;
	    }
	    bind $f.bnc <Control-ButtonPress-1> {
		%W configure -relief sunken
		if { $ay(ncadda) == "" } {
		    crtOb NCurve -length $ay(nclen);
		} else {
		    eval crtOb NCurve -length $ay(nclen) $ay(ncadda);
		}
		uCR; rV;
		after 100 "%W configure -relief raised"
		break;
	    }
	    balloon_set $f.bnc "create NCurve"

	    #####
	    button $f.bic -image ay_ICurve_img -padx 0 -pady 0 -command {
		if { $ay(icadda) == "" } {
		    crtOb ICurve -length $ay(iclen);
		} else {
		    eval crtOb ICurve -length $ay(iclen) $ay(icadda);
		}
		uCR; sL; rV;
	    }
	    bind $f.bic <Control-ButtonPress-1> {
		%W configure -relief sunken
		if { $ay(icadda) == "" } {
		    crtOb ICurve -length $ay(iclen);
		} else {
		    eval crtOb ICurve -length $ay(iclen) $ay(icadda);
		}
		uCR; rV;
		after 100 "%W configure -relief raised"
		break;
	    }
	    balloon_set $f.bic "create ICurve"

	    #####
	    button $f.bnci -image ay_NCircle_img -padx 0 -pady 0 -command {
		crtClosedBS -s $ay(cbspsec) -o $ay(cbsporder)\
		    -a $ay(cbsptmax) -r $ay(cbsprad); uCR; sL; rV;
	    }
	    bind $f.bnci <Shift-ButtonPress-1> {
		global  ay
		%W configure -relief sunken
		crtOb NCircle; uCR; sL; notifyOb; rV;
		after 100 "%W configure -relief raised"
		break;
	    }
	    bind $f.bnci <Control-ButtonPress-1> {
		%W configure -relief sunken
		crtClosedBS -s $ay(cbspsec) -o $ay(cbsporder)\
		    -a $ay(cbsptmax) -r $ay(cbsprad); uCR; rV;
		after 100 "%W configure -relief raised"
		break;
	    }
	    bind $f.bnci <Shift-Control-ButtonPress-1> {
		%W configure -relief sunken
		crtOb NCircle; uCR
		pushSel
		  hSL
		  notifyOb
		popSel
		rV
		after 100 "%W configure -relief raised"
		break;
	    }
	    balloon_set $f.bnci \
		"create circular B-Spline\nShift: create NCircle object"

	    #####
	    button $f.bnp -image ay_NPatch_img -padx 0 -pady 0 -command {
		if { $ay(npadda) == "" } {
		    crtOb NPatch -width $ay(npwidth) -height $ay(npheight);
		} else {
		    eval crtOb NPatch -width $ay(npwidth)\
			-height $ay(npheight) $ay(npadda);
		}
		uCR; sL; rV;
	    }
	    bind $f.bnp <Control-ButtonPress-1> {
		%W configure -relief sunken
		if { $ay(npadda) == "" } {
		    crtOb NPatch -width $ay(npwidth) -height $ay(npheight);
		} else {
		    eval crtOb NPatch -width $ay(npwidth)\
			-height $ay(npheight) $ay(npadda);
		}
		uCR; rV;
		after 100 "%W configure -relief raised"
		break;
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
		level_crt Sweep
		sweep_rotcross
	    }
	    balloon_set $f.bswp "create Sweep"
	    bind $f.bswp <Control-ButtonPress-1> {
		%W configure -relief sunken
		level_crt Sweep "" 1
		sweep_rotcross
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
		level_crt ExtrNC "" -1;
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
		undo save SwapUVS; swapuvS; plb_update; rV
	    }
	    balloon_set $f.bswpuv "swap UV"

	    #####
	    button $f.brevu -image ay_NPRevU_img -padx 0 -pady 0 -command {
		undo save RevertUS; revertuS; plb_update; rV
	    }
	    balloon_set $f.brevu "revert U"

	    #####
	    button $f.brevv -image ay_NPRevV_img -padx 0 -pady 0 -command {
		undo save RevertVS; revertvS; plb_update; rV
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

    if { $w == ".tbw" } {
	global ayviewshortcuts

	# establish main shortcuts also for toolbox
	shortcut_main $w

	bind $w <[repctrl $ayviewshortcuts(Close)]> "destroy .tbw"

	wm protocol $w WM_DELETE_WINDOW {
	    global ayprefs
	    set ayprefs(showtb) 0
	    destroy .tbw
	}
    }
    # if

    toolbox_layout $w

    if { $ayprefs(FixImageButtons) } {
	foreach bu $ay(toolbuttons) {
	    set oldcommand [$w.f.$bu cget -command]
	    append oldcommand ";$w.f.$bu configure -relief raised"
	    $w.f.$bu conf -command $oldcommand
	}
    }

 return;
}
# toolbox_open


# toolbox_layout:
#  arrange the buttons in rows/columns according
#  to window size
proc toolbox_layout { {w ".tbw"} } {
    global ay ayprefs

    set internal .fv.fTools

    if { $ay(tblock) == 1 } {
	return;
    } else {
	set ay(tblock) 1
    }

    if { ! [winfo exists $w] } {
	set w $internal
	if { ! [winfo exists $w] } {
	    set ay(tblock) 0; return;
	}
    }

    # to avoid being called too often, we temporarily remove the
    # configure binding
    bind $w <Configure> ""

    # we also temporarily allow the enlargement of the toolbox frame
    if { $w == $internal } {
	pack configure $w -expand yes
    }

    set size [winfo reqwidth $w.f.[lindex $ay(toolbuttons) 0]]
    set rows [expr round([winfo height $w] / $size)]
    set columns [expr round([winfo width $w] / $size)]
    if { $columns < 1 } { set columns 1 }
    set numb [llength $ay(toolbuttons)]

    if { [expr $rows*$columns] < $numb } {
	ayError 1 toolbox_layout "Can not display all buttons! Resizing..."
	set height [expr ceil(double($numb)/$columns)*$size]
	if { $w == $internal } {
	    # toolbox is integrated in main window
	    $w configure -height $height
	} else {
	    # toolbox is in an extra toplevel window
	    winMoveOrResize .tbw [winfo reqwidth $w]x${height}
	}
	update
	set rows [expr round([winfo height $w] / $size)]
    }

    # release old layout
    foreach button $ay(toolbuttons) {
	grid forget $w.f.$button
    }
    pack forget $w.f

    # create new grid layout
    set bi 0
    for { set i 0 } { $i < $rows } { incr i } {
	for { set j 0 } { $j < $columns } { incr j } {
	    if { $bi < [llength $ay(toolbuttons)] } {
		grid $w.f.[lindex $ay(toolbuttons) $bi] -row $i -column $j
	    }
	    incr bi
	}
    }
    grid propagate $w.f yes
    pack $w.f -fill both -expand no

    # shrink-wrap the toplevel around buttons
    # (if the toolbox is in a separate toplevel window)
    if { $w != $internal } {
	if { $ayprefs(ToolBoxShrink) } {
	    winMoveOrResize $w [winfo reqwidth $w.f]x[winfo reqheight $w.f]
	    update
	}
    }

    set ay(tbwidth) [winfo width $w]
    set ay(tbheight) [winfo height $w]

    # disallow enlargement of the toolbox frame
    if { $w == $internal } {
	pack configure $w -expand no
    }

    bind $w <Configure> {
	if { $ay(tbwidth) != %w || $ay(tbheight) != %h } {
	    if { $ayprefs(SingleWindow) } {
		toolbox_layout .fv.fTools
	    } else {
		toolbox_layout .tbw
	    }
	}
    }

    # before we can allow to call this procedure again
    # we must make sure all changes are realized
    update

    set ay(tblock) 0

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


# toolbox_add:
#  add element to toolbox window
proc toolbox_add { fn colspan } {
    global ayprefs

    set f ""

    # find tool window
    set tf ""
    if { [winfo exists .tbw] && ![winfo exists .tbw.f.$fn] } {
	set tf .tbw.f
    }

    if { [winfo exists .fv.fTools] && ![winfo exists .fv.fTools.f.$fn] } {
	set tf .fv.fTools.f
    }

    if { $tf != "" } {

	set f [frame $tf.$fn]

	if { [string first .tbw $tf] == 0 } {
	    # calculate the row number below the last row:
	    set row [expr [lindex [grid size $tf] 1] + 1]
	    # now display the frame at calculated row,
	    # spanning the whole window:
	    grid $f -row $row -column 0 -columnspan [lindex [grid size $tf] 0]\
		-sticky we
	} else {
	    # calculate the row number of the last row
	    set row [expr [lindex [grid size $tf] 1] - 1]

	    # calculate the number of columns in the last row
	    set col 0
	    foreach slave [grid slaves .fv.fTools.f -row $row] {
		set slaveconf [grid info $slave]
		set hascolspan 0
		set i 0
		while { $i < [llength $slaveconf] } {
		    if { [string first "-columnspan" [lindex $slaveconf $i]] \
			     == 0 } {
			incr col [lindex $slaveconf [expr $i + 1]]
			set hascolspan 1
		    }
		    # jump to next option/value pair
		    incr i 2
		}
		# while
		if { ! $hascolspan } {
		    incr col
		}
	    }
	    # foreach

	    # now display the frame f at the calculated row/column,
	    # spanning colspan cells
	    grid $f -row $row -column $col -columnspan $colspan -sticky we
	}
	# if
    }
    # if

 return $f;
}
# toolbox_add


proc toolbox_startaction { action { about 0 } } {
    global ay ayprefs

    if { ($ayprefs(AutoFocus) != 1) && ($ayprefs(SingleWindow) == 1) } {
	set intfocus ""
	foreach i $ay(views) {
	    if { $i == [focus] } {
		set intfocus $i
	    }
	}
	if { $intfocus != "" } {
	    ${intfocus}.f3D.togl mc
	    $action ${intfocus}.f3D.togl
	    if { $about } {
		actionSetMark ${intfocus} ${action}A
	    }

	} else {
	    foreach i $ay(views) {
		${i}.f3D.togl mc
		$action ${i}.f3D.togl
		if { $about } {
		    actionSetMark ${i} ${action}A
		}
	    }
	}
    } else {
	foreach i $ay(views) {
	    ${i}.f3D.togl mc
	    $action ${i}.f3D.togl
	    if { $about } {
		actionSetMark ${i} ${action}A
	    }
	}
    }
    # if

 return;
}
# toolbox_startaction

