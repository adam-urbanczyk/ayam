# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2005 by Randolf Schultz
# (rschultz@informatik.uni-rostock.de) and others.
#
# All rights reserved.
#
# See the file License for details.

# rrib.tcl - onio (OpenNURBS 3DM Rhino) plugin GUI code

uplevel #0 { array set onio_options {
    Accuracy 1.0e-12
    ReadCurves 1
    ReadLayers -1
    WriteCurves 1
}   }

proc onio_import { } {
    global ay

    set filename $ay(filename)

    set types {{"3DM (Rhino) Files" ".3dm"} {"All files" *}}

    if { $filename == "" } {
	set dirname [pwd]
    } else {
	set dirname [file dirname $filename]
	if { $dirname == "." } { set dirname [pwd] }
    }

    set ifilename [tk_getOpenFile -filetypes $types -parent .\
	    -initialfile [file tail $filename] -initialdir $dirname\
	    -title "Select file to import:"]

    if { $ifilename != "" } {
	cS; plb_update
	update
	global ay ay_error onio_options
	set ay_error ""
	set onio_options(FileName) $ifilename
	set w .onio
	catch {destroy $w}
	toplevel $w -class ayam
	wm title $w "OpenNURBS IO Options"
	wm iconname $w "Ayam"
	wm transient $w .

	set f [frame $w.f1]
	pack $f -in $w -side top -fill x

	addParam $f onio_options Accuracy [list 0.0 1.0e-12 0.1 1]
	addCheck $f onio_options ReadCurves
	addParam $f onio_options ReadLayers [list -1 1 1-10]

	set f [frame $w.f2]
	button $f.bok -text "Ok" -width 5 -command {
	    global onio_options;
	    set oldcd [pwd]
	    cd [file dirname $onio_options(FileName)]

	    onioRead [file tail $onio_options(FileName)]\
		    -a $onio_options(Accuracy)\
		    -c $onio_options(ReadCurves)\
		    -l $onio_options(ReadLayers)

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
		ayError 2 "Ayam" "There were errors while importing:"
		ayError 2 "Ayam" "$onio_options(FileName)"
	    }

	    grab release .onio;
	    focus .;
	    destroy .onio
	}

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
    }

 return;
}
# onio_import

proc onio_export { } {
    global ay

    set types {{"3DM (Rhino) Files" ".3dm"} {"All files" *}}

    if { $ay(filename) == "" } {
	set filename "unnamed.3dm"
	set dirname [pwd]
    } else {
	set dirname [file dirname $ay(filename)]
	if { $dirname == "." } { set dirname [pwd] }
	set filename [file rootname $ay(filename)].3dm
    }

    set efilename [tk_getSaveFile -filetypes $types -parent .\
	    -initialfile [file tail $filename] -initialdir $dirname\
	    -title "Select file to export:"]

    if { $efilename != "" } {
	cS; plb_update
	update
	global ay ay_error onio_options
	set ay_error ""
	set onio_options(FileName) $efilename
	set w .onio
	catch {destroy $w}
	toplevel $w -class ayam
	wm title $w "OpenNURBS IO Options"
	wm iconname $w "Ayam"
	wm transient $w .

	set f [frame $w.f1]
	pack $f -in $w -side top -fill x

	addParam $f onio_options Accuracy
	addCheck $f onio_options WriteCurves

	set f [frame $w.f2]
	button $f.bok -text "Ok" -width 5 -command {
	    global onio_options;
	    set oldcd [pwd]
	    cd [file dirname $onio_options(FileName)]

	    onioWrite [file tail $onio_options(FileName)]

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
		ayError 4 "onio_import" "Done exporting to:"
		ayError 4 "onio_import" "$onio_options(FileName)"
	    } else {
		ayError 2 "Ayam" "There were errors while exporting to:"
		ayError 2 "Ayam" "$onio_options(FileName)"
	    }

	    grab release .onio;
	    focus .;
	    destroy .onio
	}

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
    }

 return;
}
# onio_export

# link proc onio_import and onio_export to Custom menu
# we need access to global array "ay"
global ay
# always create a cascaded sub-menu
$ay(cm) add cascade -menu $ay(cm).onio -label "OpenNURBS IO"
# create menu
set m [menu $ay(cm).onio]
# create menu entry
$m add command -label "Import 3DM (Rhino)" -command onio_import

$m add command -label "Export 3DM (Rhino)" -command onio_export
