# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2007 by Randolf Schultz
# (randolf.schultz@gmail.com) and others.
#
# All rights reserved.
#
# See the file License for details.

# mopsi.tcl - mopsi (Mops Import) plugin GUI code

uplevel #0 { array set mopsi_options {
ResetDM 1
ResetST 1
filename ""
}   }

# mopsi_import:
#  import a Mops scene file
proc mopsi_import { } {
    global ay ay_error mopsi_options tcl_platform aymainshortcuts

    winAutoFocusOff

    if { $mopsi_options(filename) != "" } {
	set mopsi_options(FileName) $mopsi_options(filename)
    } else {
	set mopsi_options(FileName) "unnamed.mop"
    }

    set w .mopi
    catch {destroy $w}
    toplevel $w -class ayam
    wm title $w "Import Options"
    wm iconname $w "Ayam"
    if { $ay(ws) == "Aqua" } {
	winMakeFloat $w
    } else {
	wm transient $w .
    }
    set f [frame $w.f1]
    pack $f -in $w -side top -fill x

    set types {{"Mops Scene" ".mop"} {"All files" *}}

    set ay(cancelb) .mopi.f2.bca
    set oldappb $ay(appb)
    set ay(appb) .mopi.f2.bok

    addFileT $f mopsi_options FileName $types
    addCheckB $f mopsi_options ResetDM [ms mopsi_options_ResetDM]
    addCheckB $f mopsi_options ResetST [ms mopsi_options_ResetST]

    set ay(appb) $oldappb

    set f [frame $w.f2]
    button $f.bok -text "Ok" -width 5 -command {
	global mopsi_options ay_error

	cS; plb_update

	set ay_error ""

	update
	foreach view $ay(views) { viewUnBind $view }
	update

	set filename $mopsi_options(FileName)
	mopsiRead $filename

	if { $ay_error < 2 } {
	    ayError 4 "importMops" "Done importing the Mops scene from:"
	    ayError 4 "importMops" "$filename"
	} else {
	    ayError 2 "importMops"\
		"There were errors while importing the Mops scene from:"
	    ayError 2 "importMops" "$filename"
	}

	goTop
	selOb
	set ay(CurrentLevel) "root"
	set ay(SelectedLevel) "root"
	update

	uS
	rV
	set ay(sc) 1

	destroy .mopi
	foreach view $ay(views) { viewBind $view }
	update

	set mopsi_options(filename) $filename

	after idle viewMouseToCurrent
    }
    # button

    button $f.bca -text "Cancel" -width 5 -command "\
		focus .;\
		destroy .mopi"

    pack $f.bok $f.bca -in $f -side left -fill x -expand yes
    pack $f -in $w -side bottom -fill x

    # Esc-key && close via window decoration == Cancel button
    bind $w <Escape> "$f.bca invoke"
    wm protocol $w WM_DELETE_WINDOW "$f.bca invoke"

    # context help
    bind $w <[repcont $aymainshortcuts(Help)]> { cHelp ayam-7.html\#impmops }

    winCenter $w
    focus $w.f2.bok
    winAutoFocusOn

 return;
}
# mopsi_import

global ay
$ay(im) add command -label "Mops" -command "mopsi_import"
