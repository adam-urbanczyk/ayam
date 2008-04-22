# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2008 by Randolf Schultz
# (randolf.schultz@gmail.com) and others.
#
# All rights reserved.
#
# See the file License for details.

# aydnd.tcl - ayam dnd support plugin

# Usage:
# o place tkdnd.dll/tkdnd.so, pkgIndex.tcl, and tkDND_Utils.tcl
#   into the directory "ayam/bin/plugins/tkdnd"
# o source ayam/bin/plugins/aydnd.tcl manually via the console or
#   as startup script (preferences "Main/Scripts")
# o drag and drop Ayam scene files to the Ayam main window

# aydnd_handlefiledrop:
#
#
proc aydnd_handlefiledrop { data type } {
    global ay ayprefs ay_error

    puts $data

    set newfilename $data

    if { $newfilename != "" } {
	viewCloseAll
	cS; plb_update
	#    .fu.fMain.fHier.fsel.bnon invoke
	set ay(askedscriptdisable) 0
	update
	set filename $newfilename

	set ay_error ""
	update

	# see, if this is an Ayam scene file
	set ext [file extension $filename ]
	if { ($ext != "") && ([string compare -nocase $ext ".ay"]) } {
	    # no, try to import it
	    io_importScene $filename
	    return;
	}

	# make backup copy
	if { $ayprefs(BakOnReplace) == 1 } {
	    set err [ catch {
		file copy -force -- $filename ${filename}${ayprefs(BackupExt)}
	    } ]
	}

	replaceScene $filename

	if { $ay_error < 2 } {
	    set windowfilename [file tail [file rootname $filename]]
	    wm title . "Ayam - Main - $windowfilename : --"
	    set ay(filename) $filename
	    ayError 4 "replaceScene" "Done reading scene from:"
	    ayError 4 "replaceScene" "$filename"

	    if { [file exists $filename] } {
		set dirname [file dirname $filename]
		cd $dirname
		set .fl.con(-prompt) {[file tail [pwd]]>}
		.fl.con delete end-1lines end
		Console:prompt .fl.con "\n"
	    }

	    io_readMainGeom

	} else {
	    ayError 2 "Ayam" "There were errors while loading:"
	    ayError 2 "Ayam" "$filename"
	}

	goTop
	selOb
	set ay(CurrentLevel) "root"
	set ay(SelectedLevel) "root"
	update
	uS
	rV
	io_mruAdd $filename
	set ay(sc) 0
	update
	foreach view $ay(views) { viewBind $view }
	update

	after idle viewMouseToCurrent
    }
    # if

 return;
}
# aydnd_handlefiledrop


set origwd [pwd]

cd [file dirname [info nameofexecutable]]

lappend auto_path "plugins/tkdnd"

package require tkdnd

cd $origwd

dnd bindtarget . Files <Drop> {aydnd_handlefiledrop  %D %T}

# bind views to image files (load as bgimage)
# bind views to images (load as bgimage)
# bind script objects text widget to text