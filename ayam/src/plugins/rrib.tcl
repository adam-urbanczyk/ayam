# io_insertScene:
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
	global ay ay_error
	set ay_error ""
	rrib $ifilename

	if { $ay_error < 2 } {
	    ayError 4 "rrib_import" "Done importing:"
	    ayError 4 "rrib_import" "$ifilename"
	} else {
	    ayError 2 "Ayam" "There were errors while importing:"
	    ayError 2 "Ayam" "$ifilename"
	}

	goTop
	selOb
	set ay(CurrentLevel) "root"
	set ay(SelectedLevel) "root"
	update

	uS
	rV

	set ay(sc) 1

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