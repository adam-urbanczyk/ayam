# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2001 by Randolf Schultz
# (rschultz@informatik.uni-rostock.de) and others.
#
# All rights reserved.
#
# See the file License for details.

# mfio.tcl - 3DMF Import/Export

global ay
$ay(cm) add cascade -menu $ay(cm).foo -label "3DMF"
set m [menu $ay(cm).foo]
$m add command -label "Import" -command mfio_import
$m add command -label "Export" -command mfio_export

proc mfio_import { } {

    global ay

    set filename $ay(filename)

    if { $filename == "" } {
	set dirname [pwd]
    } else {
	set dirname [file dirname $filename]
	if { $dirname == "." } { set dirname [pwd] }
    }

    set types {{"3DMF Files" ".3dmf"} {"All files" *}}
    
    set newfilename [tk_getOpenFile -filetypes $types -parent .\
	    -initialdir $dirname\
	    -title "Select file to load:"]

    if { $newfilename != "" } {

	ay_mfio_import $newfilename

    }

 return;
}

proc mfio_export { } {

    global ay

    set filename $ay(filename)

    if { $filename == "" } {
	set dirname [pwd]
    } else {
	set dirname [file dirname $filename]
	if { $dirname == "." } { set dirname [pwd] }
    }

    set types {{"3DMF Files" ".3dmf"} {"All files" *}}
    
    set newfilename [tk_getSaveFile -filetypes $types -parent .\
	    -initialdir $dirname\
	    -title "Select file to save to:"]

    if { $newfilename != "" } {

	ay_mfio_export $newfilename
	uS
	rV
    }

 return;
}