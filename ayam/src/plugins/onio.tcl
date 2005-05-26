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
    IgnoreFirstTrim 1
    WriteCurves 1
    QuadAsBRep 1
    FileName "unnamed.3dm"
}   }


#
proc onio_import { } {
    global ay ay_error onio_options

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

    set types {{"3DM (Rhino) Files" ".3dm"} {"All files" *}}
    addFileT $f onio_options FileName $types

    addParam $f onio_options Accuracy [list 0.0 1.0e-12 0.1 1]
    addCheck $f onio_options ReadCurves
    addParam $f onio_options ReadLayers [list -1 1 1-10]
    addCheck $f onio_options IgnoreFirstTrim

    set f [frame $w.f2]
    button $f.bok -text "Ok" -width 5 -command {
	global onio_options

	set oldcd [pwd]
	cd [file dirname $onio_options(FileName)]

	onioRead [file tail $onio_options(FileName)]\
	    -a $onio_options(Accuracy)\
	    -c $onio_options(ReadCurves)\
	    -l $onio_options(ReadLayers)\
	    -i $onio_options(IgnoreFirstTrim)

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

    after idle viewMouseToCurrent

 return;
}
# onio_import


proc onio_export { } {
    global ay ay_error onio_options

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

    set types {{"3DM (Rhino) Files" ".3dm"} {"All files" *}}
    addSFileT $f onio_options FileName $types

    addParam $f onio_options Accuracy
    addCheck $f onio_options WriteCurves
    addCheck $f onio_options QuadAsBRep

    set f [frame $w.f2]
    button $f.bok -text "Ok" -width 5 -command {
	global onio_options;
	set oldcd [pwd]
	cd [file dirname $onio_options(FileName)]

	onioWrite [file tail $onio_options(FileName)]\
	    -c $onio_options(WriteCurves)\
	    -q $onio_options(QuadAsBRep)

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

    after idle viewMouseToCurrent

 return;
}
# onio_export

# link procs onio_import and onio_export to Custom menu
# we need access to global array "ay"
global ay
# always create a cascaded sub-menu
$ay(cm) add cascade -menu $ay(cm).onio -label "OpenNURBS IO"
# create sub-menu
set m [menu $ay(cm).onio]
# create sub-menu entries
$m add command -label "Import 3DM (Rhino)" -command onio_import

$m add command -label "Export 3DM (Rhino)" -command onio_export
