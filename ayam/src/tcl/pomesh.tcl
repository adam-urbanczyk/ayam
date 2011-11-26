# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2001 by Randolf Schultz
# (randolf.schultz@gmail.com) and others.
#
# All rights reserved.
#
# See the file License for details.

# pomesh.tcl - PolyMesh objects Tcl code

set PolyMesh_props { Transformations Attributes Material Tags PolyMeshAttr }

array set PolyMeshAttr {
arr   PolyMeshAttrData
sproc ""
gproc ""
w     fPolyMeshAttr

}

array set PolyMeshAttrData {
Type 0
}
# create PolyMeshAttr-UI
set w [frame $ay(pca).$PolyMeshAttr(w)]

addInfo $w PolyMeshAttrData NPolys
addInfo $w PolyMeshAttrData NControls
addInfo $w PolyMeshAttrData HasNormals

uplevel #0 { array set pomeshmerge_options {
    RemoveMerged 0
    OptimizeNew 0
}   }

# pomesh_merge:
#
#
proc pomesh_merge { } {
    global ay ay_error pomeshmerge_options

    winAutoFocusOff

    set pomeshmerge_options(oldfocus) [focus]

    set w .pomeshmerge
    winDialog $w "Merge PolyMeshes"

    set f [frame $w.f1]
    pack $f -in $w -side top -fill x

    set ay(bca) $w.f2.bca
    set ay(bok) $w.f2.bok

    addCheck $f pomeshmerge_options RemoveMerged
    addCheck $f pomeshmerge_options OptimizeNew
    addCheck $f pomeshmerge_options MergePVTags

    set f [frame $w.f2]
    button $f.bok -text "Ok" -width 5 -command {
	global ay_error pomeshmerge_options

	set ay_error ""

	mergePo -p $pomeshmerge_options(MergePVTags)

	if { $ay_error > 1 } {
	    ayError 2 "Merge" "There were errors while merging!"
	    if { $pomeshmerge_options(RemoveMerged) == 1 } {
		ayError 2 "Merge" "Original PolyMesh objects not deleted!"
	    }
	    uS; rV
	    grab release .pomeshmerge
	    restoreFocus $pomeshmerge_options(oldfocus)
	    destroy .pomeshmerge
	} else {

	   if { $pomeshmerge_options(RemoveMerged) == 1 } {
	       catch {delOb}; uS; sL; rV
	    } else {
		uS; sL; rV
	    }

	   set ay(sc) 1

	   grab release .pomeshmerge
	   restoreFocus $pomeshmerge_options(oldfocus)
	   destroy .pomeshmerge

	   if { $pomeshmerge_options(OptimizeNew) == 1 } {
	       pomesh_optimize
	   }
       }

    }

    button $f.bca -text "Cancel" -width 5 -command "\
	    grab release .pomeshmerge;\
	    restoreFocus $pomeshmerge_options(oldfocus);\
	    destroy .pomeshmerge"

    pack $f.bok $f.bca -in $f -side left -fill x -expand yes
    pack $f -in $w -side bottom -fill x

    # Esc-key && close via window decoration == Cancel button
    bind $w <Escape> "$f.bca invoke"
    wm protocol $w WM_DELETE_WINDOW "$f.bca invoke"

    winCenter $w
    grab $w
    focus $w.f2.bok
    tkwait window $w

    winAutoFocusOn

    after idle viewMouseToCurrent

 return;
}
# pomesh_merge


uplevel #0 { array set pomeshopt_options {
    OptimizeCoords 1
    IgnoreNormals 1
    OptimizeFaces 0
}   }


# pomesh_optimize:
#
#
proc pomesh_optimize { } {
    global ay ay_error pomeshopt_options

    winAutoFocusOff

    set pomeshopt_options(oldfocus) [focus]

    set w .pomeshopt
    winDialog $w "Optimize PolyMesh"

    set f [frame $w.f1]
    pack $f -in $w -side top -fill x

    set ay(bca) $w.f2.bca
    set ay(bok) $w.f2.bok

    addCheck $f pomeshopt_options OptimizeCoords
    addCheck $f pomeshopt_options IgnoreNormals
    addCheck $f pomeshopt_options OptimizeFaces

    set f [frame $w.f2]
    button $f.bok -text "Ok" -width 5 -command {
	global ay_error pomeshopt_options

	set ay_error ""

	undo save OptPoMesh

	optiPo -c $pomeshopt_options(OptimizeCoords)\
		-n $pomeshopt_options(IgnoreNormals)\
		-f $pomeshopt_options(OptimizeFaces);

	rV

	if { $ay_error > 1 } {
	    ayError 2 "Optimize" "There were errors while optimizing!"
	}

	grab release .pomeshopt
	restoreFocus $pomeshopt_options(oldfocus)
	destroy .pomeshopt
    }

    button $f.bca -text "Cancel" -width 5 -command "\
	    grab release .pomeshopt;\
	    restoreFocus $pomeshopt_options(oldfocus);\
	    destroy .pomeshopt"

    pack $f.bok $f.bca -in $f -side left -fill x -expand yes
    pack $f -in $w -side bottom -fill x

    # Esc-key && close via window decoration == Cancel button
    bind $w <Escape> "$f.bca invoke"
    wm protocol $w WM_DELETE_WINDOW "$f.bca invoke"

    winCenter $w
    grab $w
    focus $w.f2.bok
    tkwait window $w

    winAutoFocusOn

    after idle viewMouseToCurrent

 return;
}
# pomesh_optimize


uplevel #0 { array set pomeshspl_options {
    Optimize 1
}   }


# pomesh_split:
#
#
proc pomesh_split { } {
    global ay ay_error pomeshspl_options

    winAutoFocusOff

    set pomeshspl_options(oldfocus) [focus]

    set w .pomeshspl
    winDialog $w "Split PolyMesh"

    set f [frame $w.f1]
    pack $f -in $w -side top -fill x

    set ay(cancelb) $w.f2.bca
    set oldappb $ay(appb)
    set ay(appb) $w.f2.bok

    addCheck $f pomeshspl_options Optimize

    set f [frame $w.f2]
    button $f.bok -text "Ok" -width 5 -command {
	global ay_error pomeshspl_options

	set ay_error ""

	undo save SplPoMesh

	splitPo

	uS; sL; rV

	if { $ay_error > 1 } {
	    ayError 2 "Split" "There were errors while splitting!"
	} else {
	    if { $pomeshspl_options(Optimize) } {
		pomesh_optimize
	    }
	}

	grab release .pomeshspl
	restoreFocus $pomeshspl_options(oldfocus)
	destroy .pomeshspl
    }

    button $f.bca -text "Cancel" -width 5 -command "\
	    grab release .pomeshspl;\
	    restoreFocus $pomeshspl_options(oldfocus);\
	    destroy .pomeshspl"

    pack $f.bok $f.bca -in $f -side left -fill x -expand yes
    pack $f -in $w -side bottom -fill x

    # Esc-key && close via window decoration == Cancel button
    bind $w <Escape> "$f.bca invoke"
    wm protocol $w WM_DELETE_WINDOW "$f.bca invoke"

    winCenter $w
    grab $w
    focus $w.f2.bok
    tkwait window $w

    winAutoFocusOn

    after idle viewMouseToCurrent

 return;
}
# pomesh_split
