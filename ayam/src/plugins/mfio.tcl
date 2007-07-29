# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2001 by Randolf Schultz
# (rschultz@informatik.uni-rostock.de) and others.
#
# All rights reserved.
#
# See the file License for details.

# mfio.tcl - Apple 3DMF Import/Export

uplevel #0 { array set mfio_options {
    ReadCurves 1
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
    STagName "mys"
    TTagName "myt"
}   }


#
proc mfio_import { } {
    global ay ay_error mfio_options

    winAutoFocusOff

    if { $mfio_options(filename) != "" } {
	set mfio_options(FileName) $mfio_options(filename)
    } else {
	set mfio_options(FileName) "unnamed.3dmf"
    }

    cS; plb_update
    update

    set ay_error ""

    set w .mfio
    catch {destroy $w}
    toplevel $w -class ayam
    wm title $w "3DMF Import Options"
    wm iconname $w "Ayam"
    wm transient $w .

    set f [frame $w.f1]
    pack $f -in $w -side top -fill x

    set ay(iapplydisable) 1

    set types {{"3DMF Files" ".3dmf"} {"All files" *}}
    addFileT $f mfio_options FileName $types

    addParam $f mfio_options ScaleFactor [list 0.01 0.1 1.0 10.0 100.0]

#    addCheck $f mfio_options ReadCurves
#    addCheck $f mfio_options IgnoreFirstTrim
#    addParam $f mfio_options RescaleKnots [list 0.0 1.0e-4]
#    addString $f mfio_options STagName
#    addString $f mfio_options TTagName
#    addProgress $f mfio_options Progress

    set ay(iapplydisable) 0

    set f [frame $w.f2]
    button $f.bok -text "Ok" -width 5 -command {
	global mfio_options

	set mfio_options(filename) $mfio_options(FileName)
	set oldcd [pwd]
	cd [file dirname $mfio_options(FileName)]
	ay_mfio_import [file tail $mfio_options(FileName)]
	if { 0 } {
	mfioRead [file tail $mfio_options(FileName)]\
	    -a $mfio_options(Accuracy)\
	    -c $mfio_options(ReadCurves)\
	    -l $mfio_options(ReadLayers)\
	    -i $mfio_options(IgnoreFirstTrim)\
	    -r $mfio_options(RescaleKnots)\
	    -f $mfio_options(ScaleFactor)\
	    -t $mfio_options(STagName) $mfio_options(TTagName)
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

	if { $ay_error < 2 } {
	    ayError 4 "mfio_import" "Done importing:"
	    ayError 4 "mfio_import" "$mfio_options(FileName)"
	} else {
	    ayError 2 "mfio_import" "There were errors while importing:"
	    ayError 2 "mfio_import" "$mfio_options(FileName)"
	}

	grab release .mfio
	focus .
	destroy .mfio
    }
    # button

    button $f.bca -text "Cancel" -width 5 -command "\
		grab release .mfio;\
		focus .;\
		destroy .mfio"

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
# mfio_import


proc mfio_export { } {
    global ay ay_error mfio_options

    winAutoFocusOff

    cS; plb_update
    update

    set ay_error ""

    set w .mfio
    catch {destroy $w}
    toplevel $w -class ayam
    wm title $w "3DMF Export Options"
    wm iconname $w "Ayam"
    wm transient $w .

    set f [frame $w.f1]
    pack $f -in $w -side top -fill x

    if { $ay(filename) != "" &&\
	    $mfio_options(FileName) == "unnamed.3dmf" } {
	set mfio_options(FileName) [file rootname $ay(filename)].3dmf
    }

    set ay(iapplydisable) 1

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
#    addString $f mfio_options STagName
#    addString $f mfio_options TTagName
#    addProgress $f mfio_options Progress

    set ay(iapplydisable) 0

    set f [frame $w.f2]
    button $f.bok -text "Ok" -width 5 -command {
	global mfio_options;

	# append extension
	set mfio_options(FileName) [io_appext $mfio_options(FileName) ".3dmf"]

	set mfio_options(filename) $mfio_options(FileName)
	set oldcd [pwd]
	cd [file dirname $mfio_options(FileName)]
	ay_mfio_export [file tail $mfio_options(FileName)]\
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

	grab release .mfio;
	focus .;
	destroy .mfio
    }
    # button

    button $f.bca -text "Cancel" -width 5 -command "\
		grab release .mfio;\
		focus .;\
		destroy .mfio"

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
# mfio_export

# link procs mfio_import and mfio_export to File/Import and File/Export menu
# we need access to global array "ay"
global ay
# create menu entries
$ay(im) add command -label "Apple 3DMF" -command mfio_import

$ay(em) add command -label "Apple 3DMF" -command mfio_export

