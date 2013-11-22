# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2007 by Randolf Schultz
# (randolf.schultz@gmail.com) and others.
#
# All rights reserved.
#
# See the file License for details.

# x3dio.tcl - x3dio (Web3D X3D) plugin GUI code

uplevel #0 { array set x3dio_options {
    Accuracy 1.0e-12
    Cancel 0
    ReadCurves 1
    ReadViewpoints 1
    ReadLayers -1
    MergeInlineDefs 0
    ReadSTrim 1
    WriteSelected 0
    ObeyNoExport 1
    IgnoreHidden 1
    WriteCurves 1
    WritePoWires 0
    WriteNpWires 0
    WriteViews 1
    WriteMaterials 1
    WriteParametrics 0
    WriteX3dom 0
    ResolveInstances 0
    RescaleKnots 0.0
    TopLevelLayers 0
    ScaleFactor 1.0
    Progress 0.0
    filename ""
    FileName "unnamed.x3d"
}   }


# x3dio_import:
#  import scene from the Web3D X3D format
#
proc x3dio_import { } {
    global ay ay_error x3dio_options aymainshortcuts

    winAutoFocusOff

    set x3dio_options(oldfocus) [focus]

    if { $x3dio_options(filename) != "" } {
	set x3dio_options(FileName) $x3dio_options(filename)
    } else {
	set x3dio_options(FileName) "unnamed.x3d"
    }

    cS; plb_update
    update

    set ay_error ""

    set w .x3dio
    set t "X3D Import"
    winDialog $w $t

    set f [frame $w.f1]
    pack $f -in $w -side top -fill x

    set ay(bca) .x3dio.f2.bca
    set ay(bok) .x3dio.f2.bok

    set types {{"X3D (Web3D) Files" ".x3d"} {"All files" *}}
    addFileT $f x3dio_options FileName $types

    addParam $f x3dio_options ScaleFactor [list 0.01 0.1 1.0 10.0 100.0]

#    addParam $f x3dio_options Accuracy [list 0.0 1.0e-12 0.1 1]
    addCheck $f x3dio_options ReadCurves
    addMenu $f x3dio_options ReadViewpoints [list "Never" "AsView" "AsCamera"]
    addCheck $f x3dio_options ReadSTrim
    addParam $f x3dio_options ReadLayers [list "-1" 1 1-10]
    addParam $f x3dio_options RescaleKnots [list 0.0 1.0e-4]
    addCheck $f x3dio_options MergeInlineDefs
    addProgress $f x3dio_options Progress

    set f [frame $w.f2]
    button $f.bok -text "Ok" -width 5 -command {
	global x3dio_options

	set x3dio_options(filename) $x3dio_options(FileName)
	set oldcd [pwd]
	cd [file dirname $x3dio_options(FileName)]

	x3dioRead [file tail $x3dio_options(FileName)]\
	    -a $x3dio_options(Accuracy)\
	    -c $x3dio_options(ReadCurves)\
	    -v $x3dio_options(ReadViewpoints)\
	    -l $x3dio_options(ReadLayers)\
	    -s $x3dio_options(ReadSTrim)\
	    -r $x3dio_options(RescaleKnots)\
	    -f $x3dio_options(ScaleFactor)\
	    -m $x3dio_options(MergeInlineDefs)

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
	    ayError 4 "x3dio_import" "Done importing:"
	    ayError 4 "x3dio_import" "$x3dio_options(FileName)"
	} else {
	    if { $ay_error != 15 } {
		ayError 2 "x3dio_import" "There were errors while importing:"
		ayError 2 "x3dio_import" "$x3dio_options(FileName)"
	    }
	}

	grab release .x3dio
	restoreFocus $x3dio_options(oldfocus)
	destroy .x3dio
    }
    # button

    button $f.bca -text "Cancel" -width 5 -command "\
                set ::x3dio_options(Cancel) 1;\
		grab release .x3dio;\
		restoreFocus $x3dio_options(oldfocus);\
		destroy .x3dio"

    pack $f.bok $f.bca -in $f -side left -fill x -expand yes
    pack $f -in $w -side bottom -fill x

    set ::x3dio_options(Cancel) 0

    # Esc-key && close via window decoration == Cancel button
    bind $w <Escape> "$f.bca invoke"
    wm protocol $w WM_DELETE_WINDOW "$f.bca invoke"

    bind $w <Key-Return> "$::ay(bok) invoke;break"
    catch {bind $w <Key-KP_Enter> "$::ay(bok) invoke;break"}

    # establish "Help"-binding
    shortcut_addcshelp $w ayam-7.html impx3d

    winRestoreOrCenter $w $t
    grab $w
    focus $w.f2.bok
    tkwait window $w

    winAutoFocusOn

    after idle viewMouseToCurrent

 return;
}
# x3dio_import


# x3dio_export:
#  export scene to the Web3D X3D format
#
proc x3dio_export { } {
    global ay ay_error x3dio_options aymainshortcuts

    winAutoFocusOff

    set x3dio_options(oldfocus) [focus]

    update

    set ay_error ""

    set w .x3dio
    set t "X3D Export"
    winDialog $w $t

    set f [frame $w.f1]
    pack $f -in $w -side top -fill x

    if { $x3dio_options(filename) != "" } {
	set x3dio_options(FileName) $x3dio_options(filename)
    } else {
	if { $ay(filename) != "" &&\
		 $x3dio_options(FileName) == "unnamed.x3d" } {
	    set x3dio_options(FileName) [file rootname $ay(filename)].x3d
	}
    }

    set ay(bca) .x3dio.f2.bca
    set ay(bok) .x3dio.f2.bok

    set types {{"X3D (Web3D) Files" ".x3d"} {"All files" *}}
    addSFileT $f x3dio_options FileName $types

    addParam $f x3dio_options ScaleFactor [list 0.01 0.1 1.0 10.0 100.0]

    #addParam $f x3dio_options Accuracy
    addCheck $f x3dio_options WriteSelected
    #addCheck $f x3dio_options ObeyNoExport
    #addCheck $f x3dio_options IgnoreHidden
    addCheck $f x3dio_options WriteCurves
    addCheck $f x3dio_options WriteViews
    addCheck $f x3dio_options WriteParametrics
    addCheck $f x3dio_options WriteMaterials
    addCheck $f x3dio_options ResolveInstances
    addCheck $f x3dio_options TopLevelLayers
    addCheck $f x3dio_options WriteX3dom
    addProgress $f x3dio_options Progress

    set f [frame $w.f2]
    button $f.bok -text "Ok" -width 5 -command {
	global x3dio_options;

	# append extension
	if { $x3dio_options(WriteX3dom) == 0 } {
	    set x3dio_options(FileName) \
		[io_appext $x3dio_options(FileName) ".x3d"]
	} else {
	    set x3dio_options(FileName) \
		[io_appext $x3dio_options(FileName) ".xhtml"]
	}
	set x3dio_options(filename) $x3dio_options(FileName)
	set oldcd [pwd]
	cd [file dirname $x3dio_options(FileName)]

	x3dioWrite [file tail $x3dio_options(FileName)]\
	    -c $x3dio_options(WriteCurves)\
	    -v $x3dio_options(WriteViews)\
	    -s $x3dio_options(WriteSelected)\
	    -o $x3dio_options(ObeyNoExport)\
	    -i $x3dio_options(IgnoreHidden)\
	    -l $x3dio_options(TopLevelLayers)\
	    -m $x3dio_options(WriteMaterials)\
	    -f $x3dio_options(ScaleFactor)\
	    -p $x3dio_options(WriteParametrics)\
	    -x $x3dio_options(WriteX3dom)\

	cd $oldcd
	update

	if { $ay_error < 2 } {
	    ayError 4 "x3dio_export" "Done exporting to:"
	    ayError 4 "x3dio_export" "$x3dio_options(FileName)"
	} else {
	    ayError 2 "x3dio_export" "There were errors while exporting to:"
	    ayError 2 "x3dio_export" "$x3dio_options(FileName)"
	}
	# if

	grab release .x3dio
	restoreFocus $x3dio_options(oldfocus)
	destroy .x3dio
    }
    # button

    button $f.bca -text "Cancel" -width 5 -command "\
                set ::x3dio_options(Cancel) 1;\
		grab release .x3dio;\
		restoreFocus $x3dio_options(oldfocus);\
		destroy .x3dio"

    pack $f.bok $f.bca -in $f -side left -fill x -expand yes
    pack $f -in $w -side bottom -fill x

    set ::x3dio_options(Cancel) 0

    # Esc-key && close via window decoration == Cancel button
    bind $w <Escape> "$f.bca invoke"
    wm protocol $w WM_DELETE_WINDOW "$f.bca invoke"

    bind $w <Key-Return> "$::ay(bok) invoke;break"
    catch {bind $w <Key-KP_Enter> "$::ay(bok) invoke;break"}

    # establish "Help"-binding
    shortcut_addcshelp $w ayam-7.html expx3d

    winRestoreOrCenter $w $t
    grab $w
    focus $w.f2.bok
    tkwait window $w

    winAutoFocusOn

    after idle viewMouseToCurrent

 return;
}
# x3dio_export

# link procs x3dio_import and x3dio_export to File/Import and File/Export menu
# we need access to global array "ay"
global ay
# create menu entries
$ay(im) add command -label "Web3D X3D" -command x3dio_import

$ay(em) add command -label "Web3D X3D" -command x3dio_export
