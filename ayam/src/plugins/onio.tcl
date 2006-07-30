# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2005 by Randolf Schultz
# (rschultz@informatik.uni-rostock.de) and others.
#
# All rights reserved.
#
# See the file License for details.

# onio.tcl - onio (OpenNURBS 3DM Rhino) plugin GUI code

uplevel #0 { array set onio_options {
    Accuracy 1.0e-12
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
    FileName "unnamed.3dm"
    STagName "mys"
    TTagName "myt"
}   }


#
proc onio_import { } {
    global ay ay_error onio_options

    winAutoFocusOff

    if { $onio_options(filename) != "" } {
	set onio_options(FileName) $onio_options(filename)
    } else {
	set onio_options(FileName) "unnamed.3dm"
    }

    cS; plb_update
    update

    set ay_error ""

    set w .onio
    catch {destroy $w}
    toplevel $w -class ayam
    wm title $w "OpenNURBS Import Options"
    wm iconname $w "Ayam"
    wm transient $w .

    set f [frame $w.f1]
    pack $f -in $w -side top -fill x

    set ay(iapplydisable) 1

    set types {{"3DM (Rhino) Files" ".3dm"} {"All files" *}}
    addFileT $f onio_options FileName $types

    addParam $f onio_options ScaleFactor [list 0.01 0.1 1.0 10.0 100.0]

    addParam $f onio_options Accuracy [list 0.0 1.0e-12 0.1 1]
    addCheck $f onio_options ReadCurves
    addCheck $f onio_options IgnoreFirstTrim
    addParam $f onio_options ReadLayers [list -1 1 1-10]
    addParam $f onio_options RescaleKnots [list 0.0 1.0e-4]
    addString $f onio_options STagName
    addString $f onio_options TTagName
    addProgress $f onio_options Progress

    set ay(iapplydisable) 0

    set f [frame $w.f2]
    button $f.bok -text "Ok" -width 5 -command {
	global onio_options

	set onio_options(filename) $onio_options(FileName)
	set oldcd [pwd]
	cd [file dirname $onio_options(FileName)]

	onioRead [file tail $onio_options(FileName)]\
	    -a $onio_options(Accuracy)\
	    -c $onio_options(ReadCurves)\
	    -l $onio_options(ReadLayers)\
	    -i $onio_options(IgnoreFirstTrim)\
	    -r $onio_options(RescaleKnots)\
	    -f $onio_options(ScaleFactor)\
	    -t $onio_options(STagName) $onio_options(TTagName)

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
	    ayError 4 "onio_import" "Done importing:"
	    ayError 4 "onio_import" "$onio_options(FileName)"
	} else {
	    ayError 2 "onio_import" "There were errors while importing:"
	    ayError 2 "onio_import" "$onio_options(FileName)"
	}

	grab release .onio
	focus .
	destroy .onio
    }
    # button

    button $f.bca -text "Cancel" -width 5 -command "\
		grab release .onio;\
		focus .;\
		destroy .onio"

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
# onio_import


proc onio_export { } {
    global ay ay_error onio_options

    winAutoFocusOff

    cS; plb_update
    update

    set ay_error ""

    set w .onio
    catch {destroy $w}
    toplevel $w -class ayam
    wm title $w "OpenNURBS Export Options"
    wm iconname $w "Ayam"
    wm transient $w .

    set f [frame $w.f1]
    pack $f -in $w -side top -fill x

    if { $ay(filename) != "" &&\
	    $onio_options(FileName) == "unnamed.3dm" } {
	set onio_options(FileName) [file rootname $ay(filename)].3dm
    }

    set ay(iapplydisable) 1

    set types {{"3DM (Rhino) Files" ".3dm"} {"All files" *}}
    addSFileT $f onio_options FileName $types

    addParam $f onio_options ScaleFactor [list 0.01 0.1 1.0 10.0 100.0]

    addParam $f onio_options Accuracy
    addCheck $f onio_options WriteSelected
    addCheck $f onio_options ObeyNoExport
    addCheck $f onio_options IgnoreHidden
    addCheck $f onio_options WriteCurves
    addCheck $f onio_options QuadAsBRep
    addCheck $f onio_options TopLevelLayers
    addString $f onio_options STagName
    addString $f onio_options TTagName
    addProgress $f onio_options Progress

    set ay(iapplydisable) 0

    set f [frame $w.f2]
    button $f.bok -text "Ok" -width 5 -command {
	global onio_options;

	# append extension
	set onio_options(FileName) [io_appext $onio_options(FileName) ".3dm"]

	set onio_options(filename) $onio_options(FileName)
	set oldcd [pwd]
	cd [file dirname $onio_options(FileName)]

	onioWrite [file tail $onio_options(FileName)]\
	    -c $onio_options(WriteCurves)\
	    -q $onio_options(QuadAsBRep)\
	    -s $onio_options(WriteSelected)\
	    -o $onio_options(ObeyNoExport)\
	    -i $onio_options(IgnoreHidden)\
	    -l $onio_options(TopLevelLayers)\
	    -f $onio_options(ScaleFactor)\
	    -t $onio_options(STagName) $onio_options(TTagName)

	cd $oldcd
	update

	if { $ay_error < 2 } {
	    ayError 4 "onio_export" "Done exporting to:"
	    ayError 4 "onio_export" "$onio_options(FileName)"
	} else {
	    ayError 2 "onio_export" "There were errors while exporting to:"
	    ayError 2 "onio_export" "$onio_options(FileName)"
	}
	# if

	grab release .onio;
	focus .;
	destroy .onio
    }
    # button

    button $f.bca -text "Cancel" -width 5 -command "\
		grab release .onio;\
		focus .;\
		destroy .onio"

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
# onio_export

# link procs onio_import and onio_export to File/Import and File/Export menu
# we need access to global array "ay"
global ay
# create menu entries
$ay(im) add command -label "Rhino 3DM" -command onio_import

$ay(em) add command -label "Rhino 3DM" -command onio_export
