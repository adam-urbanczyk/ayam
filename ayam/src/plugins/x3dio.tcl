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
    ErrorLevel 1
    ReadCurves 1
    ReadViewpoints 1
    ReadLayers -1
    MergeInlineDefs 0
    ReadSTrim 1
    WriteSelected 0
    ObeyNoExport 1
    IgnoreHidden 1
    WriteCurves 1
    WriteViews 1
    WriteParametrics 1
    ResolveInstances 0
    RescaleKnots 0.0
    TopLevelLayers 0
    ScaleFactor 1.0
    Progress 0.0
    filename ""
    FileName "unnamed.x3d"
    STagName "mys"
    TTagName "myt"
}   }


#
proc x3dio_import { } {
    global ay ay_error x3dio_options aymainshortcuts

    winAutoFocusOff

    if { $x3dio_options(filename) != "" } {
	set x3dio_options(FileName) $x3dio_options(filename)
    } else {
	set x3dio_options(FileName) "unnamed.x3d"
    }

    cS; plb_update
    update

    set ay_error ""

    set w .x3dio
    catch {destroy $w}
    toplevel $w -class ayam
    wm title $w "X3D Import Options"
    wm iconname $w "Ayam"
    if { $ay(ws) == "Aqua" } {
	winMakeFloat $w
    } else {
	wm transient $w .
    }

    set f [frame $w.f1]
    pack $f -in $w -side top -fill x

    set ay(iapplydisable) 1

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
    addString $f x3dio_options STagName
    addString $f x3dio_options TTagName
    addMenu $f x3dio_options ErrorLevel [list Silence Errors Warnings All]
    addProgress $f x3dio_options Progress

    set ay(iapplydisable) 0

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
	    -e $x3dio_options(ErrorLevel)\
	    -l $x3dio_options(ReadLayers)\
	    -s $x3dio_options(ReadSTrim)\
	    -r $x3dio_options(RescaleKnots)\
	    -f $x3dio_options(ScaleFactor)\
	    -m $x3dio_options(MergeInlineDefs)\
	    -t $x3dio_options(STagName) $x3dio_options(TTagName)

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
	focus .
	destroy .x3dio
    }
    # button

    button $f.bca -text "Cancel" -width 5 -command "\
                set ::x3dio_options(Cancel) 1;\
		grab release .x3dio;\
		focus .;\
		destroy .x3dio"

    pack $f.bok $f.bca -in $f -side left -fill x -expand yes
    pack $f -in $w -side bottom -fill x

    set ::x3dio_options(Cancel) 0

    # Esc-key && close via window decoration == Cancel button
    bind $w <Escape> "$f.bca invoke"
    wm protocol $w WM_DELETE_WINDOW "$f.bca invoke"

    # context help
    bind $w <[repcont $aymainshortcuts(Help)]> { cHelp ayam-7.html\#impx3d }

    winCenter $w
    grab $w
    focus $w.f2.bok
    tkwait window $w

    winAutoFocusOn

    after idle viewMouseToCurrent

 return;
}
# x3dio_import


proc x3dio_export { } {
    global ay ay_error x3dio_options aymainshortcuts

    winAutoFocusOff

    cS; plb_update
    update

    set ay_error ""

    set w .x3dio
    catch {destroy $w}
    toplevel $w -class ayam
    wm title $w "X3D Export Options"
    wm iconname $w "Ayam"
    if { $ay(ws) == "Aqua" } {
	winMakeFloat $w
    } else {
	wm transient $w .
    }

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

    set ay(iapplydisable) 1

    set types {{"X3D (Web3D) Files" ".x3d"} {"All files" *}}
    addSFileT $f x3dio_options FileName $types

    addParam $f x3dio_options ScaleFactor [list 0.01 0.1 1.0 10.0 100.0]

    #addParam $f x3dio_options Accuracy
    addCheck $f x3dio_options WriteSelected
    addCheck $f x3dio_options ObeyNoExport
    addCheck $f x3dio_options IgnoreHidden
    addCheck $f x3dio_options WriteCurves
    addCheck $f x3dio_options WriteViews
    addCheck $f x3dio_options WriteParametrics
    addCheck $f x3dio_options ResolveInstances
    addCheck $f x3dio_options TopLevelLayers
    addString $f x3dio_options STagName
    addString $f x3dio_options TTagName
    addMenu $f x3dio_options ErrorLevel [list Silence Errors Warnings All]
    addProgress $f x3dio_options Progress

    set ay(iapplydisable) 0

    set f [frame $w.f2]
    button $f.bok -text "Ok" -width 5 -command {
	global x3dio_options;

	# append extension
	set x3dio_options(FileName) [io_appext $x3dio_options(FileName) ".x3d"]

	set x3dio_options(filename) $x3dio_options(FileName)
	set oldcd [pwd]
	cd [file dirname $x3dio_options(FileName)]

	x3dioWrite [file tail $x3dio_options(FileName)]\
	    -c $x3dio_options(WriteCurves)\
	    -e $x3dio_options(ErrorLevel)\
	    -v $x3dio_options(WriteViews)\
	    -s $x3dio_options(WriteSelected)\
	    -o $x3dio_options(ObeyNoExport)\
	    -i $x3dio_options(IgnoreHidden)\
	    -l $x3dio_options(TopLevelLayers)\
	    -f $x3dio_options(ScaleFactor)\
	    -x $x3dio_options(WriteParametrics)\
	    -r $x3dio_options(ResolveInstances)\
	    -t $x3dio_options(STagName) $x3dio_options(TTagName)

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

	grab release .x3dio;
	focus .;
	destroy .x3dio
    }
    # button

    button $f.bca -text "Cancel" -width 5 -command "\
                set ::x3dio_options(Cancel) 1;\
		grab release .x3dio;\
		focus .;\
		destroy .x3dio"

    pack $f.bok $f.bca -in $f -side left -fill x -expand yes
    pack $f -in $w -side bottom -fill x

    set ::x3dio_options(Cancel) 0

    # Esc-key && close via window decoration == Cancel button
    bind $w <Escape> "$f.bca invoke"
    wm protocol $w WM_DELETE_WINDOW "$f.bca invoke"

    # context help
    bind $w <[repcont $aymainshortcuts(Help)]> { cHelp ayam-7.html\#expx3d }

    winCenter $w
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
