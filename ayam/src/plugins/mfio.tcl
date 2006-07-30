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
$ay(im) add command -label "Apple 3DMF" -command mfio_import
$ay(em) add command -label "Apple 3DMF" -command mfio_export

proc mfio_import { } {
    global ay

    winAutoFocusOff

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

    winAutoFocusOn

 return;
}

proc mfio_export { } {
    global ay

    winAutoFocusOff

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

	# append extension
	set newfilename [io_appext $newfilename ".3dmf"]

	ay_mfio_export $newfilename
	uS
	rV
    }

    winAutoFocusOn

 return;
}