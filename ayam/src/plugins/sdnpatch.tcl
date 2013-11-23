# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2009 by Randolf Schultz
# (randolf.schultz@gmail.com) and others.
#
# All rights reserved.
#
# See the file License for details.

# sdnpatch.tcl - SDNPatch objects Tcl code

global ay SDNPatch_props SDNPatchAttr SDNPatchAttrData

set SDNPatch_props { Transformations Attributes Material Tags SDNPatchAttr }

array set SDNPatchAttr {
arr   SDNPatchAttrData
sproc ""
gproc ""
w     fSDNPatchAttr
}
# SDNPatchAttr

# create SDNPatchAttr-UI
set w [frame $ay(pca).$SDNPatchAttr(w)]


addParam $w SDNPatchAttrData Degree
addParam $w SDNPatchAttrData Level
addText $w SDNPatchAttrData "Subdivision:"
addInfo $w SDNPatchAttrData NPolys

# add menu entry to the "Create/Custom Object" sub-menu

uplevel #0 { array set sdncrt_options {
    Width 3
    Height 3
    Depth 3
    DX 1
    DY 1
    DZ 1
} }

mmenu_addcustom SDNPatch "crtOb SDNPatch;uS;sL;rV"
mmenu_addcustom SDNCube {
    runTool {\
     sdncrt_options(Width) sdncrt_options(Height) sdncrt_options(Depth)\
     sdncrt_options(DX) sdncrt_options(DY) sdncrt_options(DZ) }\
     {"Width:" "Height:" "Depth:" "DX:" "DY:" "DZ:"}\
	"sdnpatch_crtcube %0 %1 %2 %3 %4 %5;"\
	"Create SDNCube"
}

global ay
$ay(cm) add cascade -menu $ay(cm).sdn -label "SDNPatch"
menu $ay(cm).sdn -tearoff 0
set m $ay(cm).sdn
set ay(felength) 1.0
set ay(fescale) 1.0
$m add command -label "Face Extrude" -command {
    runTool {ay(felength) ay(fescale)} {"Length:" "Scale:"} \
	"undo save SDNExtFace; sdnextrudeFace %0 %1; rV;"\
	"Face Extrude"
}
$m add command -label "Face Remove" -command {
    undo save SDNRemFace; sdnremoveFace; rV; }
$m add command -label "Face Merge" -command {
    undo save SDNMergeFace; sdnmergeFace; rV; }
$m add command -label "Face Connect" -command {
    undo save SDNConnFace; sdnconnectFace; rV; }
$m add command -label "Reset All Knots" -command {
    undo save SDNResetKnots; sdneditKnots -r; rV; }
$m add command -label "Set Knot" -command sdnpatch_setknot

$m add command -label "Revert" -command  {
    undo save SDNRevert; sdnrevert; rV; }

$m add command -label "Merge" -command  {
    sdnmergePatch; uCR; rV; set ay(sc) 1}

$m add separator

$m add command -label "NPatch To SDNPatch" -command {
    sdnconvertNP; uS; rV; set ay(sc) 1 }
$m add command -label "PolyMesh To SDNPatch" -command {
    sdnconvertPO; uS; rV; set ay(sc) 1 }

$m add separator

$m add command -label "Import PLY" -command plyio_import
$m add command -label "Export PLY" -command plyio_export

# arrange for all tools to update the "last used" tool menu entry
mmenu_addlume $ay(cm).sdn

# XXXX ToDo: add own keyboard shortcuts for face extrude etc.?

#lappend ay(customkeys) [list {<Key-f>} {sdnpatch_selfaceact $w.f3D.togl} ]

# tell the rest of Ayam (or other custom objects), that we are loaded
lappend ay(co) SDNPatch


uplevel #0 { array set plyio_options {
    filename ""
    FileName "unnamed.ply"
} }



# plyio_import:
#  import PLY file
#
proc plyio_import { } {
    global ay ay_error plyio_options aymainshortcuts

    winAutoFocusOff

    set plyio_options(oldfocus) [focus]

    if { $plyio_options(filename) != "" } {
	set plyio_options(FileName) $plyio_options(filename)
    } else {
	set plyio_options(FileName) "unnamed.ply"
    }

    cS; plb_update
    update

    set ay_error ""

    set w .plyio
    set t "PLY Import"
    winDialog $w $t

    set f [frame $w.f1]
    pack $f -in $w -side top -fill x

    set ay(bca) .plyio.f2.bca
    set ay(bok) .plyio.f2.bok

    set types {{"PLY Files" ".ply"} {"All files" *}}
    addFileT $f plyio_options FileName $types
    #addParam $f plyio_options ScaleFactor [list 0.01 0.1 1.0 10.0 100.0]

    set f [frame $w.f2]
    button $f.bok -text "Ok" -width 5 -command {
	global plyio_options

	set plyio_options(filename) $plyio_options(FileName)
	set oldcd [pwd]
	cd [file dirname $plyio_options(FileName)]
	sdnimpPly [file tail $plyio_options(FileName)]

	cd $oldcd
	goTop
	selOb
	set ay(CurrentLevel) "root"
	set ay(SelectedLevel) "root"
	update

	uS
	rV

	set ay(sc) 1

	if { $ay_error < 2 } {
	    ayError 4 "plyio_import" "Done importing:"
	    ayError 4 "plyio_import" "$plyio_options(FileName)"
	} else {
	    ayError 2 "plyio_import" "There were errors while importing:"
	    ayError 2 "plyio_import" "$plyio_options(FileName)"
	}

	grab release .plyio
	restoreFocus $plyio_options(oldfocus)
	destroy .plyio
    }
    # button

    button $f.bca -text "Cancel" -width 5 -command "\
		grab release .plyio;\
		restoreFocus $plyio_options(oldfocus);\
		destroy .plyio"

    pack $f.bok $f.bca -in $f -side left -fill x -expand yes
    pack $f -in $w -side bottom -fill x

    # Esc-key && close via window decoration == Cancel button
    bind $w <Escape> "$f.bca invoke"
    wm protocol $w WM_DELETE_WINDOW "$f.bca invoke"

    bind $w <Key-Return> "$::ay(bok) invoke;break"
    catch {bind $w <Key-KP_Enter> "$::ay(bok) invoke;break"}

    # context help
    #shortcut_addcshelp $w ayam-7.html impply

    winRestoreOrCenter $w $t
    grab $w
    focus $w.f2.bok
    tkwait window $w

    winAutoFocusOn

    after idle viewMouseToCurrent

 return;
}
# plyio_import


# plyio_export:
#  export PLY file
#
proc plyio_export { } {
    global ay ay_error plyio_options aymainshortcuts

    winAutoFocusOff

    set plyio_options(oldfocus) [focus]

    update

    set ay_error ""

    set w .plyio
    set t "PLY Export"
    winDialog $w $t

    set f [frame $w.f1]
    pack $f -in $w -side top -fill x

    if { $plyio_options(filename) != "" } {
	set plyio_options(FileName) $plyio_options(filename)
    } else {
	if { $ay(filename) != "" &&\
		 $plyio_options(FileName) == "unnamed.ply" } {
	    set plyio_options(FileName) [file rootname $ay(filename)].ply
	}
    }

    set ay(bca) .plyio.f2.bca
    set ay(bok) .plyio.f2.bok

    set types {{"PLY Files" ".ply"} {"All files" *}}
    addSFileT $f plyio_options FileName $types

    set f [frame $w.f2]
    button $f.bok -text "Ok" -width 5 -command {
	global plyio_options;

	# append extension
	set plyio_options(FileName) [io_appext $plyio_options(FileName) ".ply"]

	set plyio_options(filename) $plyio_options(FileName)
	set oldcd [pwd]
	cd [file dirname $plyio_options(FileName)]
	sdnexpPly [file tail $plyio_options(FileName)]

	cd $oldcd
	update

	if { $ay_error < 2 } {
	    ayError 4 "plyio_export" "Done exporting to:"
	    ayError 4 "plyio_export" "$plyio_options(FileName)"
	} else {
	    ayError 2 "plyio_export" "There were errors while exporting to:"
	    ayError 2 "plyio_export" "$plyio_options(FileName)"
	}
	# if

	grab release .plyio
	restoreFocus $plyio_options(oldfocus)
	destroy .plyio
    }
    # button

    button $f.bca -text "Cancel" -width 5 -command "\
		grab release .plyio;\
		restoreFocus $plyio_options(oldfocus);\
		destroy .plyio"

    pack $f.bok $f.bca -in $f -side left -fill x -expand yes
    pack $f -in $w -side bottom -fill x

    # Esc-key && close via window decoration == Cancel button
    bind $w <Escape> "$f.bca invoke"
    wm protocol $w WM_DELETE_WINDOW "$f.bca invoke"

    bind $w <Key-Return> "$::ay(bok) invoke;break"
    catch {bind $w <Key-KP_Enter> "$::ay(bok) invoke;break"}

    # context help
    #shortcut_addcshelp $w ayam-7.html expply
    winRestoreOrCenter $w $t
    grab $w
    focus $w.f2.bok
    tkwait window $w

    winAutoFocusOn

    after idle viewMouseToCurrent

 return;
}
# plyio_export

uplevel #0 { array set sdnsetkn_options {
    Knot 1.0
} }

# sdnpatch_setknot:
#  set knot value
#
proc sdnpatch_setknot { } {
    global sdnsetkn_options

    # get old knot value first
    sdneditKnots -s sdnsetkn_options(Knot)

    # open edit window
    runTool {sdnsetkn_options(Knot)} {"Knot:"}\
	"undo save SDNSetKnot; sdneditKnots -i %0; rV"\
	"Set Knot"

 return;
}
# sdnpatch_setknot


# sdnpatch_crtcube:
#
#
proc sdnpatch_crtcube { w h d dx dy dz } {

    # create a temporary working level
    crtOb Level 1

    # enter the level
    goDown -1

    # now create 6 polymesh objects

    # front
    set np [expr $w * $h]

    set nv {}
    set i 0
    while { $i < $np } {
	lappend nv 4
	incr i
    }

    set iv {}
    set i 0
    set ii 0
    while { $i < $h } {
	set j 0
	while { $j < $w } {
	    lappend iv $ii [expr $ii + 1]\
		[expr $ii + $w + 2] [expr $ii + $w + 1]
	    incr ii
	    incr j
	}
	incr ii
	incr i
    }

    set x [expr -($w * $dx) / 2.0]
    set y [expr -($h * $dy) / 2.0]
    set z [expr ($d * $dz) / 2.0]
    set cv {}
    set i 0
    set sx $x
    while { $i < [expr $h + 1] } {
	set j 0
	set x $sx
	while { $j < [expr $w + 1] } {
	    lappend cv $x $y $z
	    set x [expr $x + $dx]
	    incr j
	}
	set y [expr $y + $dy]
	incr i
    }

    crtOb PolyMesh -polys $np -nverts $nv -iverts $iv -cv $cv

    # back
    set x [expr ($w * $dx) / 2.0]
    set y [expr -($h * $dy) / 2.0]
    set z [expr -($d * $dz) / 2.0]
    set cv {}
    set i 0
    set sx $x
    while { $i < [expr $h + 1] } {
	set j 0
	set x $sx
	while { $j < [expr $w + 1] } {
	    lappend cv $x $y $z
	    set x [expr $x - $dx]
	    incr j
	}
	set y [expr $y + $dy]
	incr i
    }

    crtOb PolyMesh -polys $np -nverts $nv -iverts $iv -cv $cv

    # left
    set np [expr $h * $d]

    set nv {}
    set i 0
    while { $i < $np } {
	lappend nv 4
	incr i
    }

    set iv {}
    set i 0
    set ii 0
    while { $i < $h } {
	set j 0
	while { $j < $d } {
	    lappend iv $ii [expr $ii + 1]\
		[expr $ii + $d + 2] [expr $ii + $d + 1]
	    incr ii
	    incr j
	}
	incr ii
	incr i
    }

    set x [expr -($w * $dx) / 2.0]
    set y [expr -($h * $dy) / 2.0]
    set z [expr -($d * $dz) / 2.0]
    set cv {}
    set i 0
    set sz $z
    while { $i < [expr $h + 1] } {
	set j 0
	set z $sz
	while { $j < [expr $d + 1] } {
	    lappend cv $x $y $z
	    set z [expr $z + $dz]
	    incr j
	}
	set y [expr $y + $dy]
	incr i
    }

    crtOb PolyMesh -polys $np -nverts $nv -iverts $iv -cv $cv

    # right
    set x [expr ($w * $dx) / 2.0]
    set y [expr -($h * $dy) / 2.0]
    set z [expr ($d * $dz) / 2.0]
    set cv {}
    set i 0
    set sz $z
    while { $i < [expr $h + 1] } {
	set j 0
	set z $sz
	while { $j < [expr $d + 1] } {
	    lappend cv $x $y $z
	    set z [expr $z - $dz]
	    incr j
	}
	set y [expr $y + $dy]
	incr i
    }

    crtOb PolyMesh -polys $np -nverts $nv -iverts $iv -cv $cv


    # bottom
    set np [expr $w * $d]

    set nv {}
    set i 0
    while { $i < $np } {
	lappend nv 4
	incr i
    }

    set iv {}
    set i 0
    set ii 0
    while { $i < $d } {
	set j 0
	while { $j < $w } {
	    lappend iv $ii [expr $ii + 1]\
		[expr $ii + $w + 2] [expr $ii + $w + 1]
	    incr ii
	    incr j
	}
	incr ii
	incr i
    }

    set x [expr -($w * $dx) / 2.0]
    set y [expr -($h * $dy) / 2.0]
    set z [expr -($d * $dz) / 2.0]
    set cv {}
    set i 0
    set sx $x
    while { $i < [expr $d + 1] } {
	set j 0
	set x $sx
	while { $j < [expr $w + 1] } {
	    lappend cv $x $y $z
	    set x [expr $x + $dx]
	    incr j
	}
	set z [expr $z + $dz]
	incr i
    }

    crtOb PolyMesh -polys $np -nverts $nv -iverts $iv -cv $cv

    # top
    set x [expr ($w * $dx) / 2.0]
    set y [expr ($h * $dy) / 2.0]
    set z [expr -($d * $dz) / 2.0]
    set cv {}
    set i 0
    set sx $x
    while { $i < [expr $d + 1] } {
	set j 0
	set x $sx
	while { $j < [expr $w + 1] } {
	    lappend cv $x $y $z
	    set x [expr $x - $dx]
	    incr j
	}
	set z [expr $z + $dz]
	incr i
    }

    crtOb PolyMesh -polys $np -nverts $nv -iverts $iv -cv $cv

    # select all polymeshes
    selOb 0 1 2 3 4 5

    # merge them into one
    mergePo

    selOb 6

    # optimize the merged mesh
    optiPo

    # convert to SDNPatch
    sdnconvertPO

    # move the SDNPatch to parent level, go up and remove this
    # level where the temporary polymesh objects were created
    selOb 7

    cutOb

    goUp

    uCR

    sL

    delOb

    # paste/move the SDNPatch back to the scene
    pasmovOb

    # update tree view and redraw
    uCL cl

    sL

    rV

return;
}
# sdnpatch_crtcube
