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


# add menu entry to the "Create/Custom Object" sub-menu
mmenu_addcustom SDNPatch "crtOb SDNPatch;uS;sL;rV"

global ay
$ay(cm) add cascade -menu $ay(cm).sdn -label "SDNPatch"
menu $ay(cm).sdn -tearoff 0
set m $ay(cm).sdn

$m add command -label "Face Extrude" -command {
    undo save SDNExtFace; sdnextrudeFace; rV; }
$m add command -label "Face Remove" -command {
    undo save SDNRemFace; sdnremoveFace; rV; }
$m add command -label "Face Merge" -command {
    undo save SDNMergeFace; sdnmergeFace; rV; }
$m add command -label "Face Connect" -command {
    undo save SDNConnFace; sdnconnectFace; rV; }
$m add command -label "Reset All Knots" -command {
    undo save SDNResetKnots; sdneditKnots -r; rV; }
$m add command -label "Set Knot" -command sdnpatch_setknot

$m add separator

$m add command -label "NPatch To SDNPatch" -command {
    sdnconvertNP; uS; rV; }
$m add command -label "PoMesh To SDNPatch" -command {
    sdnconvertPO; uS; rV; }

$m add separator

$m add command -label "Import PLY" -command plyio_import
$m add command -label "Export PLY" -command plyio_export

# arrange for all tools to update the "last used" tool menu entry
mmenu_addlume $ay(cm).sdn

# XXXX ToDo: add own keyboard shortcuts for face extrude etc.?

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
    catch {destroy $w}
    toplevel $w -class ayam
    wm title $w "PLY Import Options"
    wm iconname $w "Ayam"
    if { $ay(ws) == "Aqua" } {
	winMakeFloat $w
    } else {
	wm transient $w .
    }
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
    #bind $w <[repcont $aymainshortcuts(Help)]> { cHelp ayam-7.html\#impply }

    winCenter $w
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

    cS; plb_update
    update

    set ay_error ""

    set w .plyio
    catch {destroy $w}
    toplevel $w -class ayam
    wm title $w "PLY Export Options"
    wm iconname $w "Ayam"
    if { $ay(ws) == "Aqua" } {
	winMakeFloat $w
    } else {
	wm transient $w .
    }
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
	sdnimpPly [file tail $plyio_options(FileName)]

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
    #bind $w <[repcont $aymainshortcuts(Help)]> { cHelp ayam-7.html\#expply }

    winCenter $w
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

    # XXXX get old knot value instead!
    #set ::sdnsetkn_options(Knot) 1.0

    runTool {sdnsetkn_options(Knot)} {"Knot:"}\
	"undo save SDNSetKnot; sdneditKnots -i %0; rV"

 return;
}
# sdnpatch_setknot
