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

    if { $data != "" } {
	viewCloseAll
	cS; plb_update
	set ay(askedscriptdisable) 0
	update
    } else {
	# issue error message?
	return;
    }

    set insert 0
    foreach filename $data {
	set ay_error ""
	update

	# see, if this is an Ayam scene file
	set ext [file extension $filename]
	if { ($ext != "") && ([string compare -nocase $ext ".ay"]) } {
	    # no, try to import it
	    io_importScene $filename
	} else {
	    # this is a Ayam scene

	    # make backup copy
	    if { $ayprefs(BakOnReplace) == 1 } {
		set err [ catch {
		file copy -force -- $filename ${filename}${ayprefs(BackupExt)}
		} ]
	    }

	    # change working directory
	    if { [file exists $filename] } {
		cd [file dirname $filename]
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
		if { ! $insert } {
		    set windowfilename [file tail [file rootname $filename]]
		    wm title . "Ayam - Main - $windowfilename : --"
		    set ay(filename) $filename
		    ayError 4 $fname "Done reading scene from: $filename"
		    # restore main window geometry from tag
		    io_readMainGeom
		} else {
		    ayError 4 $fname "Done inserting scene from: $filename"
		}
	    } else {
		wm title . "Ayam - Main - : --"
		set ay(filename) ""
		ayError 2 $fname "Failed reading scene from: $filename"
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
	    # the following Ayam scene files will be inserted
	    set insert 1
	}
	# if
    }
    # foreach

    # reset scene change indicator
    set ay(sc) 0
    update
    foreach view $ay(views) { viewBind $view }
    update

    after idle viewMouseToCurrent

 return;
}
# aydnd_handlefiledrop


set origwd [pwd]

cd [file dirname [info nameofexecutable]]

lappend auto_path "plugins/tkdnd"

package require tkdnd

cd $origwd

tkdnd::drop_target register . DND_Files
bind . <<Drop>> {aydnd_handlefiledrop %D %T}

# bind views to image files (load as bgimage)
# bind views to images (load as bgimage)
# bind script objects text widget to text
