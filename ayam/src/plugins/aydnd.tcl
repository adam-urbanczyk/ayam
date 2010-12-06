# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2008 by Randolf Schultz
# (randolf.schultz@gmail.com) and others.
#
# All rights reserved.
#
# See the file License for details.

# aydnd.tcl - Ayam dnd support plugin based on tkdnd by Georgios Petasis

# Usage:
# o place tkdnd.dll/tkdnd.so, pkgIndex.tcl, and tkDND_Utils.tcl
#   into the directory "ayam/bin/plugins/tkdnd"
# o source ayam/bin/plugins/aydnd.tcl manually via the console or
#   as startup script (add "aydnd" to preferences "Main/Scripts")
# o drag and drop Ayam scene files to the Ayam main window

# aydnd_handlefiledrop:
#
#
proc aydnd_handlefiledrop { data type } {
    global ay ayprefs ay_error

    set insert 0
    foreach newfilename $data {

    if { $newfilename != "" } {
	viewCloseAll
	cS; plb_update
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

	# change working directory
	if { [file exists $filename] } {
	    set dirname [file dirname $filename]
	    cd $dirname
	    set .fl.con(-prompt) {[file tail [pwd]]>}
	    .fl.con delete end-1lines end
	    Console:prompt .fl.con "\n"
	}

	# read the file
	if { $insert } {
	    set fname "insertScene"
	    insertScene $filename

	} else {
	    set fname "replaceScene"
	    replaceScene $filename
	}
	if { $ay_error < 2 } {
	    set windowfilename [file tail [file rootname $filename]]
	    wm title . "Ayam - Main - $windowfilename : --"
	    set ay(filename) $filename
	    ayError 4 $fname "Done reading scene from:"
	    ayError 4 $fname "$filename"

	    # restore main window geometry from tag
	    io_readMainGeom

	} else {
	    wm title . "Ayam - Main - : --"
	    set ay(filename) ""
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
	# add scene file to most recently used list
	io_mruAdd $filename
	# reset scene change indicator
	set ay(sc) 0
	update
	foreach view $ay(views) { viewBind $view }
	update

	after idle viewMouseToCurrent
    }
    # if
    set insert 1
    }
    # foreach
 return;
}
# aydnd_handlefiledrop


set origwd [pwd]

cd [file dirname [info nameofexecutable]]

lappend auto_path "plugins/tkdnd"

package require tkdnd

cd $origwd

tkdnd::drop_target register . DND_Files
bind . <<Drop>> {aydnd_handlefiledrop  %D %T}

# bind views to image files (load as bgimage)
# bind views to images (load as bgimage)
# bind script objects text widget to text
