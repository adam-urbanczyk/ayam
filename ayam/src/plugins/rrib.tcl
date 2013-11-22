# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2004 by Randolf Schultz
# (randolf.schultz@gmail.com) and others.
#
# All rights reserved.
#
# See the file License for details.

# rrib.tcl - RRIB plugin GUI code

uplevel #0 { array set rrib_options {
  ReadSTrim 1
  ReadFrame -1
  ReadCamera 1
  ReadOptions 1
  ReadLights 1
  ReadMaterial 1
  ReadPartial 0
  RescaleKnots 0.0
  ScaleFactor 1.0
  filename ""
  FileName "unnamed.rib"
}   }

# rrib_import:
#
#
proc rrib_import { } {
    global ay ay_error rrib_options aymainshortcuts

    winAutoFocusOff

    set rrib_options(oldfocus) [focus]

    if { $rrib_options(filename) != "" } {
	set rrib_options(FileName) $rrib_options(filename)
    } else {
	set rrib_options(FileName) "unnamed.rib"
    }

    cS; plb_update
    update

    set ay_error ""

    set w .ribI
    set t "RRIB Options"
    winDialog $w $t

    set f [frame $w.f1]
    pack $f -in $w -side top -fill x

    set ay(bca) .ribI.f2.bca
    set ay(bok) .ribI.f2.bok

    set types {{"RIB Files" ".rib"} {"All files" *}}
    addFileT $f rrib_options FileName $types

    addParam $f rrib_options ScaleFactor [list 0.01 0.1 1.0 10.0 100.0]

    addParam $f rrib_options ReadFrame
    addCheck $f rrib_options ReadCamera
    addCheck $f rrib_options ReadOptions
    addCheck $f rrib_options ReadLights
    addCheck $f rrib_options ReadMaterial
    addCheck $f rrib_options ReadPartial
    addCheck $f rrib_options ReadSTrim
    addParam $f rrib_options RescaleKnots [list 0.0 1.0e-4]

    set f [frame $w.f2]
    button $f.bok -text "Ok" -width 5 -command {
	global rrib_options

	set rrib_options(filename) $rrib_options(FileName)
	set oldcd [pwd]
	cd [file dirname $rrib_options(FileName)]

	rribRead [file tail $rrib_options(FileName)]\
	    -f $rrib_options(ReadFrame)\
	    -c $rrib_options(ReadCamera)\
	    -o $rrib_options(ReadOptions)\
	    -l $rrib_options(ReadLights)\
	    -m $rrib_options(ReadMaterial)\
	    -p $rrib_options(ReadPartial)\
	    -r $rrib_options(RescaleKnots)\
	    -s $rrib_options(ScaleFactor)\
	    -t $rrib_options(ReadSTrim);

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
	    ayError 4 "rrib_import" "Done importing:"
	    ayError 4 "rrib_import" "$rrib_options(FileName)"
	} else {
	    ayError 2 "rrib_import" "There were errors while importing:"
	    ayError 2 "rrib_import" "$rrib_options(FileName)"
	}

	grab release .ribI
	restoreFocus $rrib_options(oldfocus)
	destroy .ribI
    }
    # button

    button $f.bca -text "Cancel" -width 5 -command "\
		grab release .ribI;\
		restoreFocus $rrib_options(oldfocus);\
		destroy .ribI"

    pack $f.bok $f.bca -in $f -side left -fill x -expand yes
    pack $f -in $w -side bottom -fill x

    # Esc-key && close via window decoration == Cancel button
    bind $w <Escape> "$f.bca invoke"
    wm protocol $w WM_DELETE_WINDOW "$f.bca invoke"

    bind $w <Key-Return> "$::ay(bok) invoke;break"
    catch {bind $w <Key-KP_Enter> "$::ay(bok) invoke;break"}

    # establish "Help"-binding
    shortcut_addcshelp $w ayam-7.html imprib

    winRestoreOrCenter $w $t
    grab $w
    focus $w.f2.bok
    tkwait window $w

    winAutoFocusOn

    after idle viewMouseToCurrent

 return;
}
# rrib_import


# link proc rrib_import to File/Import menu
# we need access to global array "ay"
global ay
# create menu entry
$ay(im) add command -label "RenderMan RIB" -command rrib_import
