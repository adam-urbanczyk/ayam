# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2007 by Randolf Schultz
# (rschultz@informatik.uni-rostock.de) and others.
#
# All rights reserved.
#
# See the file License for details.

# dxfio.tcl - dxfio (AutoCAD DXF) plugin GUI code

uplevel #0 { array set dxfio_options {
    Accuracy 1.0e-12
    Cancel 0
    ErrorLevel 1
    ReadCurves 1
    ReadLayers -1
    IgnoreFirstTrim 0
    WriteSelected 0
    ObeyNoExport 1
    IgnoreHidden 1
    WriteCurves 1
    QuadAsBRep 1
    RescaleKnots 0.0
    TopLevelLayers 0
    ScaleFactor 1.0
    Progress 0.0
    filename ""
    FileName "unnamed.dxf"
    STagName "mys"
    TTagName "myt"
}   }


#
proc dxfio_import { } {
    global ay ay_error dxfio_options

    winAutoFocusOff

    if { $dxfio_options(filename) != "" } {
	set dxfio_options(FileName) $dxfio_options(filename)
    } else {
	set dxfio_options(FileName) "unnamed.dxf"
    }

    cS; plb_update
    update

    set ay_error ""

    set w .dxfio
    catch {destroy $w}
    toplevel $w -class ayam
    wm title $w "DXF Import Options"
    wm iconname $w "Ayam"
    wm transient $w .

    set f [frame $w.f1]
    pack $f -in $w -side top -fill x

    set ay(iapplydisable) 1

    set types {{"DXF (AutoCAD) Files" ".dxf"} {"All files" *}}
    addFileT $f dxfio_options FileName $types

    addParam $f dxfio_options ScaleFactor [list 0.01 0.1 1.0 10.0 100.0]

#    addParam $f dxfio_options Accuracy [list 0.0 1.0e-12 0.1 1]
    addCheck $f dxfio_options ReadCurves
#    addCheck $f dxfio_options IgnoreFirstTrim
    addParam $f dxfio_options ReadLayers [list "-1" 1 1-10]
    addParam $f dxfio_options RescaleKnots [list 0.0 1.0e-4]
#    addString $f dxfio_options STagName
#    addString $f dxfio_options TTagName
    addMenu $f dxfio_options ErrorLevel [list Silence Errors Warnings All]
    addProgress $f dxfio_options Progress

    set ay(iapplydisable) 0

    set f [frame $w.f2]
    button $f.bok -text "Ok" -width 5 -command {
	global dxfio_options

	set dxfio_options(filename) $dxfio_options(FileName)
	set oldcd [pwd]
	cd [file dirname $dxfio_options(FileName)]

	dxfioRead [file tail $dxfio_options(FileName)]\
	    -a $dxfio_options(Accuracy)\
	    -c $dxfio_options(ReadCurves)\
	    -e $dxfio_options(ErrorLevel)\
	    -l $dxfio_options(ReadLayers)\
	    -i $dxfio_options(IgnoreFirstTrim)\
	    -r $dxfio_options(RescaleKnots)\
	    -f $dxfio_options(ScaleFactor)\
	    -t $dxfio_options(STagName) $dxfio_options(TTagName)

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
	    ayError 4 "dxfio_import" "Done importing:"
	    ayError 4 "dxfio_import" "$dxfio_options(FileName)"
	} else {
	    ayError 2 "dxfio_import" "There were errors while importing:"
	    ayError 2 "dxfio_import" "$dxfio_options(FileName)"
	}

	grab release .dxfio
	focus .
	destroy .dxfio
    }
    # button

    button $f.bca -text "Cancel" -width 5 -command "\
                set ::dxfio_options(Cancel) 1;\
		grab release .dxfio;\
		focus .;\
		destroy .dxfio"

    pack $f.bok $f.bca -in $f -side left -fill x -expand yes
    pack $f -in $w -side bottom -fill x

    set ::dxfio_options(Cancel) 0

    winCenter $w
    grab $w
    focus $w.f2.bok
    tkwait window $w

    winAutoFocusOn

    after idle viewMouseToCurrent

 return;
}
# dxfio_import


proc dxfio_export { } {
    global ay ay_error dxfio_options

    winAutoFocusOff

    cS; plb_update
    update

    set ay_error ""

    set w .dxfio
    catch {destroy $w}
    toplevel $w -class ayam
    wm title $w "DXF Export Options"
    wm iconname $w "Ayam"
    wm transient $w .

    set f [frame $w.f1]
    pack $f -in $w -side top -fill x

    if { $ay(filename) != "" &&\
	    $dxfio_options(FileName) == "unnamed.dxf" } {
	set dxfio_options(FileName) [file rootname $ay(filename)].dxf
    }

    set ay(iapplydisable) 1

    set types {{"DXF (AutoCAD) Files" ".dxf"} {"All files" *}}
    addSFileT $f dxfio_options FileName $types

    addParam $f dxfio_options ScaleFactor [list 0.01 0.1 1.0 10.0 100.0]

    addParam $f dxfio_options Accuracy
    addCheck $f dxfio_options WriteSelected
    addCheck $f dxfio_options ObeyNoExport
    addCheck $f dxfio_options IgnoreHidden
    addCheck $f dxfio_options WriteCurves
    addCheck $f dxfio_options QuadAsBRep
    addCheck $f dxfio_options TopLevelLayers
    addString $f dxfio_options STagName
    addString $f dxfio_options TTagName
    addProgress $f dxfio_options Progress

    set ay(iapplydisable) 0

    set f [frame $w.f2]
    button $f.bok -text "Ok" -width 5 -command {
	global dxfio_options;

	# append extension
	set dxfio_options(FileName) [io_appext $dxfio_options(FileName) ".dxf"]

	set dxfio_options(filename) $dxfio_options(FileName)
	set oldcd [pwd]
	cd [file dirname $dxfio_options(FileName)]

	dxfioWrite [file tail $dxfio_options(FileName)]\
	    -c $dxfio_options(WriteCurves)\
	    -q $dxfio_options(QuadAsBRep)\
	    -s $dxfio_options(WriteSelected)\
	    -o $dxfio_options(ObeyNoExport)\
	    -i $dxfio_options(IgnoreHidden)\
	    -l $dxfio_options(TopLevelLayers)\
	    -f $dxfio_options(ScaleFactor)\
	    -t $dxfio_options(STagName) $dxfio_options(TTagName)

	cd $oldcd
	update

	if { $ay_error < 2 } {
	    ayError 4 "dxfio_export" "Done exporting to:"
	    ayError 4 "dxfio_export" "$dxfio_options(FileName)"
	} else {
	    ayError 2 "dxfio_export" "There were errors while exporting to:"
	    ayError 2 "dxfio_export" "$dxfio_options(FileName)"
	}
	# if

	grab release .dxfio;
	focus .;
	destroy .dxfio
    }
    # button

    button $f.bca -text "Cancel" -width 5 -command "\
		grab release .dxfio;\
		focus .;\
		destroy .dxfio"

    pack $f.bok $f.bca -in $f -side left -fill x -expand yes
    pack $f -in $w -side bottom -fill x

    winCenter $w
    grab $w
    focus $w.f2.bok
    tkwait window $w

    winAutoFocusOn

    after idle viewMouseToCurrent

 return;
}
# dxfio_export

# link procs dxfio_import and dxfio_export to File/Import and File/Export menu
# we need access to global array "ay"
global ay
# create menu entries
$ay(im) add command -label "AutoCAD DXF" -command dxfio_import

$ay(em) add command -label "AutoCAD DXF" -command dxfio_export
