# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2001 by Randolf Schultz
# (rschultz@informatik.uni-rostock.de) and others.
#
# All rights reserved.
#
# See the file License for details.

# rrib.tcl - RRIB plugin GUI code

uplevel #0 { array set rrib_options {
ReadFrame -1
ReadCamera 1
ReadOptions 1
ReadLights 1
ReadMaterial 1
ReadPartial 0

}   }

# rrib_import:
#
#
proc rrib_import { } {
    global ay

    set filename $ay(filename)

    set types {{"RIB Files" ".rib"} {"All files" *}}

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
	global ay ay_error rrib_options
	set ay_error ""

	set w .ribI
	catch {destroy $w}
	toplevel $w
	wm title $w "RRIB Options"
	wm iconname $w "Ayam"
	wm transient $w .

	set f [frame $w.f1]
	pack $f -in $w -side top -fill x

	addParam $f rrib_options ReadFrame
	addCheck $f rrib_options ReadCamera
	addCheck $f rrib_options ReadOptions
	addCheck $f rrib_options ReadLights
	addCheck $f rrib_options ReadMaterial
	addCheck $f rrib_options ReadPartial


	set f [frame $w.f2]
	button $f.bok -text "Ok" -width 5 -command {
	    global rrib_options;
	    rrib $ifilename -f $rrib_options(ReadFrame)\
		    -c $rrib_options(ReadCamera)\
		    -o $rrib_options(ReadOptions)\
		    -l $rrib_options(ReadLights)\
		    -m $rrib_options(ReadMaterial)\
		    -p $rrib_options(ReadPartial);
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
		ayError 4 "rrib_import" "$ifilename"
	    } else {
		ayError 2 "Ayam" "There were errors while importing:"
		ayError 2 "Ayam" "$ifilename"
	    }


	    grab release .ribI;
	    focus .;
	    destroy .ribI
	}

	button $f.bca -text "Cancel" -width 5 -command "\
		grab release .ribI;\
		focus .;\
		destroy .ribI"

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
# rrib_import

# link proc rrib_import to Custom menu
# we need access to global array "ay"
global ay
# always create a cascaded sub-menu
$ay(cm) add cascade -menu $ay(cm).rrib -label "RRIB"
# create menu
set m [menu $ay(cm).rrib]
# create menu entry
$m add command -label "Import RIB" -command rrib_import