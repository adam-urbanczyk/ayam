# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2001 by Randolf Schultz
# (rschultz@informatik.uni-rostock.de) and others.
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

    set w .pomeshopt
    catch {destroy $w}
    toplevel $w
    wm title $w "Optimize PolyMesh"
    wm iconname $w "Ayam"
    wm transient $w .

    set f [frame $w.f1]
    pack $f -in $w -side top -fill x

    addCheck $f pomeshopt_options OptimizeCoords
    addCheck $f pomeshopt_options IgnoreNormals
    addCheck $f pomeshopt_options OptimizeFaces

    set f [frame $w.f2]
    button $f.bok -text "Ok" -width 5 -command {
	global pomeshopt_options;
	optiPo -c $pomeshopt_options(OptimizeCoords)\
		-n $pomeshopt_options(IgnoreNormals)\
		-f $pomeshopt_options(OptimizeFaces);

	rV

	if { $ay_error > 2 } {
	    ayError 2 "Ayam" "There were errors while optimizing!"
	}

	grab release .pomeshopt
	focus .
	destroy .pomeshopt
    }

    button $f.bca -text "Cancel" -width 5 -command "\
	    grab release .pomeshopt;\
	    focus .;\
	    destroy .pomeshopt"

    pack $f.bok $f.bca -in $f -side left -fill x -expand yes
    pack $f -in $w -side bottom -fill x

    winCenter $w
    grab $w
    focus $w.f2.bok
    tkwait window $w

    after idle viewMouseToCurrent

 return;
}
# pomesh_optimize
