# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2007 by Randolf Schultz
# (randolf.schultz@gmail.com) and others.
#
# All rights reserved.
#
# See the file License for details.

# objio.tcl - objio (Wavefront OBJ IO) plugin GUI code

uplevel #0 { array set objio_options {
WriteSelected 0
TessPoMesh 0
WriteCurves 1
MergeFaces 1
MergePVTags 1
ScaleFactor 1.0
RescaleKnots 0.0
filename ""
FileName "unnamed.obj"
Progress 0
CheckDegen 1
ReadSTrim 1
}   }


# objio_export:
#  export scene to Wavefront OBJ format
#
proc objio_export { } {
    global ay ay_error objio_options aymainshortcuts
    set ay_error ""

    winAutoFocusOff

    set objio_options(oldfocus) [focus]

    if { $objio_options(filename) != "" } {
	set objio_options(FileName) $objio_options(filename)
    } else {
	if { $ay(filename) != "" &&\
		$objio_options(FileName) == "unnamed.obj" } {
	    set objio_options(FileName) [file rootname $ay(filename)].obj
	}
    }

    set w .objE
    set t "Export OBJ"
    winDialog $w $t

    set f [frame $w.f1]
    pack $f -in $w -side top -fill x

    set ay(bca) .objE.f2.bca
    set ay(bok) .objE.f2.bok

    set types {{"Wavefront OBJ" ".obj"} {"All files" *}}
    addSFileT $f objio_options FileName $types
    addCheckB $f objio_options WriteSelected [ms objio_options_Selected]
    addCheckB $f objio_options WriteCurves [ms objio_options_WriteCurves]
    addCheckB $f objio_options TessPoMesh [ms objio_options_TessPoMesh]
    addParam $f objio_options ScaleFactor [list 0.01 0.1 1.0 10.0 100.0]
    set objio_options(Progress) 0
    addProgress $f objio_options Progress

    set f [frame $w.f2]
    button $f.bok -text "Ok" -width 5 -command {
	global objio_options ay_error

	# append extension
	set objio_options(FileName) [io_appext $objio_options(FileName) ".obj"]

	set objio_options(filename) $objio_options(FileName)
	set oldcd [pwd]
	cd [file dirname $objio_options(FileName)]

	set filename [file tail $objio_options(FileName)]

	set objio_options(Cancel) 0
	update

	objioWrite $filename -s $objio_options(WriteSelected)\
	    -p $objio_options(TessPoMesh)\
	    -c $objio_options(WriteCurves)\
	    -f $objio_options(ScaleFactor)

	cd $oldcd

	if { $ay_error < 2 } {
	    ayError 4 "exportOBJ" "Done exporting to:"
	    ayError 4 "exportOBJ" "$objio_options(FileName)"
	} else {
	    ayError 2 "exportOBJ" "There were errors while exporting to:"
	    ayError 2 "exportOBJ" "$objio_options(FileName)"
	}

	grab release .objE
	restoreFocus $objio_options(oldfocus)
	destroy .objE
    }
    # button

    button $f.bca -text "Cancel" -width 5 -command "\
		global objio_options;\
                set objio_options(Cancel) 1;\
		grab release .objE;\
                restoreFocus $objio_options(oldfocus);\
		destroy .objE"

    pack $f.bok $f.bca -in $f -side left -fill x -expand yes
    pack $f -in $w -side bottom -fill x

    # Esc-key && close via window decoration == Cancel button
    bind $w <Escape> "$f.bca invoke"
    wm protocol $w WM_DELETE_WINDOW "$f.bca invoke"

    bind $w <Key-Return> "$::ay(bok) invoke;break"
    catch {bind $w <Key-KP_Enter> "$::ay(bok) invoke;break"}

    # establish "Help"-binding
    shortcut_addcshelp $w ayam-7.html expwav

    winRestoreOrCenter $w $t
    grab $w
    focus $w.f2.bok
    tkwait window $w

    winAutoFocusOn

    after idle viewMouseToCurrent

 return;
}
# objio_export


# objio_import:
#  import scene from the Wavefront OBJ format
#
proc objio_import { } {
    global ay ay_error objio_options aymainshortcuts
    set ay_error ""

    winAutoFocusOff

    set objio_options(oldfocus) [focus]

    if { $objio_options(filename) != "" } {
	set objio_options(FileName) $objio_options(filename)
    } else {
	set objio_options(FileName) "unnamed.obj"
    }

    set w .objI
    set t "Import OBJ"
    winDialog $w $t

    set f [frame $w.f1]
    pack $f -in $w -side top -fill x

    set ay(bca) .objI.f2.bca
    set ay(bok) .objI.f2.bok

    set types {{"Wavefront OBJ" ".obj"} {"All files" *}}
    addFileT $f objio_options FileName $types
    addCheck $f objio_options MergeFaces
    addCheck $f objio_options MergePVTags
    addCheck $f objio_options ReadCurves
    addCheck $f objio_options ReadSTrim
    addParam $f objio_options RescaleKnots [list 0.0 1.0e-4]
    addParam $f objio_options ScaleFactor  [list 0.01 0.1 1.0 10.0 100.0]
    addCheck $f objio_options CheckDegen
    set objio_options(Progress) 0
    addProgress $f objio_options Progress

    set f [frame $w.f2]
    button $f.bok -text "Ok" -width 5 -command {
	global objio_options ay_error

	set objio_options(filename) $objio_options(FileName)
	set oldcd [pwd]
	cd [file dirname $objio_options(FileName)]

	set objio_options(Cancel) 0
	update
	objioRead [file tail $objio_options(FileName)]\
		-m $objio_options(MergeFaces)\
		-c $objio_options(ReadCurves)\
		-p $objio_options(MergePVTags)\
		-r $objio_options(RescaleKnots)\
	        -f $objio_options(ScaleFactor)\
	        -d $objio_options(CheckDegen)\
	        -s $objio_options(ReadSTrim)

	if { $ay_error < 2 } {
	    ayError 4 "importOBJ" "Done importing scene from:"
	    ayError 4 "importOBJ" "$objio_options(FileName)"
	} else {
	    ayError 2 "importOBJ"\
		    "There were errors while importing scene from:"
	    ayError 2 "importOBJ" "$objio_options(FileName)"
	}

	cd $oldcd

	goTop
	selOb
	set ay(CurrentLevel) "root"
	set ay(SelectedLevel) "root"
	update

	uS
	rV

	set ay(sc) 1

	grab release .objI
	restoreFocus $objio_options(oldfocus)
	destroy .objI
    }
    # button

    button $f.bca -text "Cancel" -width 5 -command "\
                global objio_options;\
                set objio_options(Cancel) 1;\
		grab release .objI;\
                restoreFocus $objio_options(oldfocus);\
		destroy .objI"

    pack $f.bok $f.bca -in $f -side left -fill x -expand yes
    pack $f -in $w -side bottom -fill x

    # Esc-key && close via window decoration == Cancel button
    bind $w <Escape> "$f.bca invoke"
    wm protocol $w WM_DELETE_WINDOW "$f.bca invoke"

    bind $w <Key-Return> "$::ay(bok) invoke;break"
    catch {bind $w <Key-KP_Enter> "$::ay(bok) invoke;break"}

    # establish "Help"-binding
    shortcut_addcshelp $w ayam-7.html impwav

    winRestoreORCenter $w $t
    grab $w
    focus $w.f2.bok
    tkwait window $w

    winAutoFocusOn

    after idle viewMouseToCurrent

 return;
}
# objio_import


global ay

$ay(im) add command -label "Wavefront OBJ" -command "objio_import"
$ay(em) add command -label "Wavefront OBJ" -command "objio_export"
