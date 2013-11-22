# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2001 by Randolf Schultz
# (randolf.schultz@gmail.com) and others.
#
# All rights reserved.
#
# See the file License for details.

# mfio.tcl - Apple 3DMF Import/Export

uplevel #0 { array set mfio_options {
    ReadCurves 1
    ReadSTrim 1
    IgnoreFirstTrim 0
    WriteSelected 0
    WriteBinary 0
    ObeyNoExport 1
    IgnoreHidden 1
    WriteCurves 1
    QuadAsBRep 1
    RescaleKnots 0.0
    TopLevelLayers 0
    ScaleFactor 1.0
    Progress 0.0
    filename ""
    FileName "unnamed.3dmf"
}   }


# mfio_import:
#  import scene from the Apple 3DMF format
#
proc mfio_import { } {
    global ay ay_error mfio_options aymainshortcuts

    winAutoFocusOff

    set mfio_options(oldfocus) [focus]

    if { $mfio_options(filename) != "" } {
	set mfio_options(FileName) $mfio_options(filename)
    } else {
	set mfio_options(FileName) "unnamed.3dmf"
    }

    cS; plb_update
    update

    set ay_error ""

    set w .mfio
    set t "3DMF Import Options"
    winDialog $w $t

    set f [frame $w.f1]
    pack $f -in $w -side top -fill x

    set ay(bca) .mfio.f2.bca
    set ay(bok) .mfio.f2.bok

    set types {{"3DMF Files" ".3dmf"} {"All files" *}}
    addFileT $f mfio_options FileName $types
    addParam $f mfio_options ScaleFactor [list 0.01 0.1 1.0 10.0 100.0]
    addCheck $f mfio_options ReadCurves
    addCheck $f mfio_options ReadSTrim
    addParam $f mfio_options RescaleKnots [list 0.0 1.0e-4]
#    addProgress $f mfio_options Progress

    set f [frame $w.f2]
    button $f.bok -text "Ok" -width 5 -command {
	global mfio_options

	set mfio_options(filename) $mfio_options(FileName)
	set oldcd [pwd]
	cd [file dirname $mfio_options(FileName)]
	mfioRead [file tail $mfio_options(FileName)]\
	    -c $mfio_options(ReadCurves)\
	    -i $mfio_options(IgnoreFirstTrim)\
	    -r $mfio_options(RescaleKnots)\
	    -s $mfio_options(ReadSTrim)\
	    -f $mfio_options(ScaleFactor)

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
	    ayError 4 "mfio_import" "Done importing:"
	    ayError 4 "mfio_import" "$mfio_options(FileName)"
	} else {
	    ayError 2 "mfio_import" "There were errors while importing:"
	    ayError 2 "mfio_import" "$mfio_options(FileName)"
	}

	grab release .mfio
	restoreFocus $mfio_options(oldfocus)
	destroy .mfio
    }
    # button

    button $f.bca -text "Cancel" -width 5 -command "\
		grab release .mfio;\
		restoreFocus $mfio_options(oldfocus);\
		destroy .mfio"

    pack $f.bok $f.bca -in $f -side left -fill x -expand yes
    pack $f -in $w -side bottom -fill x

    # Esc-key && close via window decoration == Cancel button
    bind $w <Escape> "$f.bca invoke"
    wm protocol $w WM_DELETE_WINDOW "$f.bca invoke"

    bind $w <Key-Return> "$::ay(bok) invoke;break"
    catch {bind $w <Key-KP_Enter> "$::ay(bok) invoke;break"}

    # establish "Help"-binding
    shortcut_addcshelp $w ayam-7.html imp3dmf

    winRestoreOrCenter $w $t
    grab $w
    focus $w.f2.bok
    tkwait window $w

    winAutoFocusOn

    after idle viewMouseToCurrent

 return;
}
# mfio_import


# mfio_export:
#  export scene to the Apple 3DMF format
#
proc mfio_export { } {
    global ay ay_error mfio_options aymainshortcuts

    winAutoFocusOff

    set mfio_options(oldfocus) [focus]

    update

    set ay_error ""

    set w .mfio
    set t "3DMF Export Options"
    winDialog $w $t

    set f [frame $w.f1]
    pack $f -in $w -side top -fill x

    if { $mfio_options(filename) != "" } {
	set mfio_options(FileName) $mfio_options(filename)
    } else {
	if { $ay(filename) != "" &&\
		 $mfio_options(FileName) == "unnamed.3dmf" } {
	    set mfio_options(FileName) [file rootname $ay(filename)].3dmf
	}
    }

    set ay(bca) .mfio.f2.bca
    set ay(bok) .mfio.f2.bok

    set types {{"3DMF Files" ".3dmf"} {"All files" *}}
    addSFileT $f mfio_options FileName $types
    addCheck $f mfio_options WriteBinary
    addParam $f mfio_options ScaleFactor [list 0.01 0.1 1.0 10.0 100.0]
    addCheck $f mfio_options WriteSelected
#    addCheck $f mfio_options ObeyNoExport
#    addCheck $f mfio_options IgnoreHidden
    addCheck $f mfio_options WriteCurves
#    addCheck $f mfio_options QuadAsBRep
#    addCheck $f mfio_options TopLevelLayers
#    addProgress $f mfio_options Progress

    set f [frame $w.f2]
    button $f.bok -text "Ok" -width 5 -command {
	global mfio_options;

	# append extension
	set mfio_options(FileName) [io_appext $mfio_options(FileName) ".3dmf"]

	set mfio_options(filename) $mfio_options(FileName)
	set oldcd [pwd]
	cd [file dirname $mfio_options(FileName)]
	mfioWrite [file tail $mfio_options(FileName)]\
	    -c $mfio_options(WriteCurves)\
	    -b $mfio_options(WriteBinary)\
	    -s $mfio_options(WriteSelected)\
	    -f $mfio_options(ScaleFactor)

	cd $oldcd
	update

	if { $ay_error < 2 } {
	    ayError 4 "mfio_export" "Done exporting to:"
	    ayError 4 "mfio_export" "$mfio_options(FileName)"
	} else {
	    ayError 2 "mfio_export" "There were errors while exporting to:"
	    ayError 2 "mfio_export" "$mfio_options(FileName)"
	}
	# if

	grab release .mfio
	restoreFocus $mfio_options(oldfocus)
	destroy .mfio
    }
    # button

    button $f.bca -text "Cancel" -width 5 -command "\
		grab release .mfio;\
		restoreFocus $mfio_options(oldfocus);\
		destroy .mfio"

    pack $f.bok $f.bca -in $f -side left -fill x -expand yes
    pack $f -in $w -side bottom -fill x

    # Esc-key && close via window decoration == Cancel button
    bind $w <Escape> "$f.bca invoke"
    wm protocol $w WM_DELETE_WINDOW "$f.bca invoke"

    bind $w <Key-Return> "$::ay(bok) invoke;break"
    catch {bind $w <Key-KP_Enter> "$::ay(bok) invoke;break"}

    # establish "Help"-binding
    shortcut_addcshelp $w ayam-7.html exp3dmf

    winRestoreOrCenter $w $t
    grab $w
    focus $w.f2.bok
    tkwait window $w

    winAutoFocusOn

    after idle viewMouseToCurrent

 return;
}
# mfio_export

# link procs mfio_import and mfio_export to File/Import and File/Export menu
# we need access to global array "ay"
global ay
# create menu entries
$ay(im) add command -label "Apple 3DMF" -command mfio_import

$ay(em) add command -label "Apple 3DMF" -command mfio_export

