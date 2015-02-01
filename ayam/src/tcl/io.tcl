# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2006 by Randolf Schultz
# (randolf.schultz@gmail.com) and others.
#
# All rights reserved.
#
# See the file License for details.

# io.tcl - scene io helper procedures

# io_checkType:
#  check, whether the provided file is an Ayam scene file
#
proc io_checkType { filename } {
    if { [string first "Ayam" [gets [open $filename "r"]]] == 0 } {
	return 1;
    }
 return 0;
}
# io_checkType


# io_replaceScene:
# clear scene, then read new scene file
#
proc io_replaceScene { {newfilename ""} } {
    global ay ayprefs tcl_platform

    winAutoFocusOff

    if { $newfilename eq "" } {

	set filename $ay(filename)

	if { $filename == "" } {
	    set dirname [pwd]
	} else {
	    set dirname [file dirname $filename]
	    if { $dirname == "." } { set dirname [pwd] }
	}

	set types [subst {
	    {"Ayam Scene" ".ay"}
	    {"Supported Files" {$ayprefs(ALFileTypes)}}
	    {"All files" *}}]

	if { $ay(ws) != "Aqua" } {
	    set newfilename [tk_getOpenFile -filetypes $types -parent .\
				 -initialfile [file tail $filename]\
				 -initialdir $dirname\
				 -title "Select file to load:"]
	} else {
	    if { [file exists [file tail $filename]] } {
		set newfilename [tk_getOpenFile -filetypes $types -parent .\
				     -initialfile [file tail $filename]\
				     -title "Select file to load:"]
	    } else {
		set newfilename [tk_getOpenFile -filetypes $types -parent .\
				     -title "Select file to load:"]
	    }
	}
    }

    if { $newfilename != "" } {
	viewCloseAll
	cS; plb_update
	set ay(askedscriptdisable) 0
	update
	set filename $newfilename
	global ay_error ayprefs
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
	replaceScene $filename

	if { $ay_error < 2 } {
	    set windowfilename [file tail [file rootname $filename]]
	    wm title . "Ayam - Main - $windowfilename : --"
	    set ay(filename) $filename
	    ayError 4 "replaceScene" "Done reading scene from:"
	    ayError 4 "replaceScene" "$filename"

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

    winAutoFocusOn

 return;
}
# io_replaceScene


# io_insertScene:
#  insert a scene
#
proc io_insertScene { {ifilename ""} } {
    global ay ayprefs

    winAutoFocusOff

    if { $ifilename eq "" } {

	set filename $ay(filename)

	if { $filename == "" } {
	    set dirname [pwd]
	} else {
	    set dirname [file dirname $filename]
	    if { $dirname == "." } { set dirname [pwd] }
	}

	set types [subst {{"Ayam Scene" ".ay"}
	    {"Supported Files" {$ayprefs(ALFileTypes)}}
	    {"All files" *}}]

	if { $ay(ws) != "Aqua" } {
	    set ifilename [tk_getOpenFile -filetypes $types -parent .\
			       -initialfile [file tail $filename]\
			       -initialdir $dirname\
			       -title "Select file to load:"]
	} else {
	    if { [file exists [file tail $filename]] } {
		set ifilename [tk_getOpenFile -filetypes $types -parent .\
				   -initialfile [file tail $filename]\
				   -title "Select file to load:"]
	    } else {
		set ifilename [tk_getOpenFile -filetypes $types -parent .\
				   -title "Select file to load:"]
	    }
	}
    }

    if { $ifilename != "" } {
	cS; plb_update
	set ay(askedscriptdisable) 0
	update
	global ay ay_error
	set ay_error ""

	# see, if this is an Ayam scene file
	set ext [file extension $ifilename ]
	if { ($ext != "") && ([string compare -nocase $ext ".ay"]) } {
	    # no, try to import it
	    io_importScene $ifilename
	    return;
	}

	foreach view $ay(views) { viewUnBind $view }
	update

	insertScene $ifilename

	if { $ay_error < 2 } {
	    ayError 4 "insertScene" "Done inserting objects from:"
	    ayError 4 "insertScene" "$ifilename"
	} else {
	    ayError 2 "Ayam" "There were errors while loading:"
	    ayError 2 "Ayam" "$ifilename"
	}

	goTop
	selOb
	set ay(CurrentLevel) "root"
	set ay(SelectedLevel) "root"
	update
	uS
	rV
	io_mruAdd $filename
	set ay(sc) 1
	update
	foreach view $ay(views) { viewBind $view }
	update
	after idle viewMouseToCurrent
    }

    winAutoFocusOn

 return;
}
# io_insertScene


# io_saveScene:
#  save a scene file
#
proc io_saveScene { ask selected } {
    global ay ayprefs

    winAutoFocusOff

    set tmp $ay(filename)
    set filename $ay(filename)

    if { ($ask == "ask") || ($filename == "") } {
	if { $filename == "" } {
	    set filename "unnamed.ay"
	    set dirname [pwd]
	} else {
	    set dirname [file dirname $filename]
	    if { $dirname == "." } { set dirname [pwd] }
	}

	set types [subst {{"Ayam Scene" ".ay"}
	    {"Supported Files" {$ayprefs(ALFileTypes)}}
	    {"All files" *}}]

	if { $ay(ws) != "Aqua" } {
	    set filename [tk_getSaveFile -filetypes $types -parent .\
		    -initialfile [file tail $filename]\
		    -initialdir $dirname -title "Save scene to:"]
	} else {
	    set filename [tk_getSaveFile -filetypes $types -parent .\
		    -initialfile [file tail $filename]\
		    -title "Save scene to:"]
	}

	# see, if the user wants to save to an Ayam scene file
	set ext [file extension $filename ]
	if { ($ext != "") && ([string compare -nocase $ext ".ay"]) } {
	    # no, try to export the scene...
	    io_exportScene $filename
	    return;
	}
    }

    if { $filename != "" } {
	# append extension
	set newfilename [io_appext $filename ".ay"]

	# fix window positions
	viewUPos

	# save main window geometry to tag
	if { ( $ay(lb) == 0 ) || ( ! $selected ) } {
	    io_saveMainGeom
	}

	# save scene to disk
	global ay_error
	set ay_error ""
	saveScene $filename $selected
	if { $ay_error < 2 } {
	    set windowfilename [file tail [file rootname $filename]]
	    wm title . "Ayam - Main - $windowfilename : --"
	    set ay(filename) $filename
	    ayError 4 "saveScene" "Done saving scene to:"
	    ayError 4 "saveScene" "$filename"
	    io_mruAdd $filename
	    set ay(sc) 0
	} else {
	    ayError 2 "Ayam" "There were errors while saving to:"
	    ayError 2 "Ayam" "$filename"
	}
    }

    winAutoFocusOn

 return;
}
# io_saveScene


# io_exportRIB:
#
#
proc io_exportRIB { {expview "" } } {
    global ay ayprefs

    winAutoFocusOff

    set ay(expRIBFocus) [focus]

    set w .exportRIBw
    set t "RIB Export"
    winDialog $w $t

    if { $ayprefs(FixDialogTitles) == 1 } {
	pack [frame $w.fl] -in $w -side top
	pack [label $w.fl.l -text $t] -in $w.fl -side left -fill x -expand yes
    }

    set f [frame $w.f1]
    pack $f -in $w -side top -fill x

    label $f.l -text "Select View:"
    set f [frame $f.fview]
    scrollbar $f.sc -command {.exportRIBw.f1.fview.lb yview} -takefocus 0
    listbox $f.lb -width 15 -height 5 -selectmode single\
	    -exportselection 0 -yscrollcommand {.exportRIBw.f1.fview.sc set}

    set ocw $ay(currentView)

    set index 0
    set firstselect 0
    foreach i $ay(views) {
	eval [subst "$i.f3D.togl mc"]

	if { $expview != "" } {
	    if { $expview == $i } {
		set firstselect $index
	    }
	} else {
	    if { $ay(cVType) == 3 } { set firstselect $index }
	}

	if { [winfo toplevel $i] == $i } {
	    scan $i ".%s" name
	} else {
	    set last [string last . $i]
	    set name [string range $i [expr $last + 2] end]
	}
	if { $ay(cVType) != 4 } {
	    $f.lb insert end "$name ([lindex $ay(viewtypenames) $ay(cVType)])"
	    incr index
	}
    }
    # foreach

    if { $ocw != "" } {
	$ocw mc
    }

    #    catch {eval [subst "$currentView getViewConfig"]}

    $f.lb selection set $firstselect

    pack $f.lb -in $f -side left -fill x -fill y
    pack $f.sc -in $f -side right -fill y
    pack $w.f1.l -in $w.f1 -side top
    pack $f -in $w.f1 -side top -fill x -fill y

    set f [frame $w.f2]
    button $f.bok -text "Ok" -pady $ay(pady) -width 5 -command {
	global ay ayprefs

	set selection [.exportRIBw.f1.fview.lb curselection]
	if { $selection != "" } {
	    set name [lindex $ay(views) $selection]

	    set ribname [io_getRIBName]
	    set efilename [lindex $ribname 0]
	    set imagename [lindex $ribname 1]

	    if { $efilename != "" } {
		if { $imagename != "" } {
		    $name.f3D.togl wrib -file $efilename -image $imagename
		    ayError 4 "exportRIB" "Done exporting scene to:"
		    ayError 4 "exportRIB" "$efilename"
		} else {
		    .$name.f3D.togl wrib -file $efilename
		    ayError 4 "exportRIB" "Done exporting scene to:"
		    ayError 4 "exportRIB" "$efilename"
		}
		# if
	    }
	    # if
	}
	# if
	grab release .exportRIBw
	restoreFocus $ay(expRIBFocus)
	destroy .exportRIBw
    }
    # button

    button $f.bca -text "Cancel" -pady $ay(pady) -width 5 -command "\
	    grab release .exportRIBw;\
            restoreFocus $ay(expRIBFocus);\
            destroy $w"

    pack $f.bok $f.bca -in $f -side left -fill x -expand yes
    pack $f -in $w -side bottom -fill x

    # Esc-key && close via window decoration == Cancel button
    bind $w <Escape> "$f.bca invoke"
    wm protocol $w WM_DELETE_WINDOW "$f.bca invoke"

    winRestoreOrCenter $w $t
    grab $w
    focus $f.bok
    tkwait window $w

    winAutoFocusOn

 return;
}
# io_exportRIB


proc loadPlugin { pname } {
    if { [file exists $pname] } {
	io_lc $pname
    } else {
	io_lcAuto $pname
    }
 return;
}


# io_lc - load custom object:
#
#
proc io_lc { filename } {

    set sopath [file dirname $filename]
    if { "$sopath" != "" } {
	set oldcdir [pwd]
	cd $sopath
	load $filename
	cd $oldcdir
    } else {
	load $filename
    }

 return;
}
# io_lc


# io_lcAuto - automatically load custom object:
#
#
proc io_lcAuto { name } {
    global ay ayprefs

    set soname [string tolower $name].$ay(soext)

    set paths [split "$ayprefs(Plugins)" $ay(separator)]

    set owd [pwd]

    cd [file dirname [info nameofexecutable]]

    foreach path $paths {
	set filename [file join $path $soname]
	set sopath [file dirname $filename]
	if { ("$sopath" != "") && [file exists $sopath] } {
	    cd $sopath
	    set success [catch {load $soname}]
	    cd $owd
	} else {
	    set success [catch {load $filename}]
	}

	if { $success == 0 } {
	    break;
	}
    }
    # foreach

    cd $owd

 return;
}
# io_lcAuto


# io_loadCustom:
#  load a custom object/plugin
proc io_loadCustom { } {
    global ay ayprefs

    winAutoFocusOff

    set pftype .$ay(soext)

    set filetypes [list [list "Plugin" $pftype] {"All files" *}]

    set idir ""

    if { ( $ayprefs(Plugins) != "" ) && ( $ayprefs(Plugins) != "plugins" ) } {
	if { [file exists "$ayprefs(Plugins)"] } {
	    set idir "$ayprefs(Plugins)"
	}
    } else {
	set pl [file join [file dirname [info nameofexecutable]] plugins]
	if { [file exists "$pl"] } {
	    set idir $pl
	}
    }

    set oldcdir [pwd]

    if { $idir == "" } {
	set filename [tk_getOpenFile -filetypes $filetypes -parent .\
		-title "Select Plugin:"]
    } else {
	if { $ay(ws) != "Aqua" } {
	    set filename [tk_getOpenFile -filetypes $filetypes -parent .\
		    -initialdir $idir -title "Select Plugin:"]
	} else {
	    if { !$ayprefs(UseInternalFD) } {
		set filename [tk_getOpenFile -filetypes $filetypes -parent .\
			-initialdir $idir -title "Select Plugin:"]
	    } else {
		set filename [::tk::dialog::file:: open -filetypes $filetypes\
			-parent . -initialdir $idir -title "Select Plugin:"]
	    }
	}
    }

    if { $filename != "" } {
	io_lc $filename
    }

    cd $oldcdir

    winAutoFocusOn

 return;
}
# io_loadCustom


# io_mruAdd
#  add <filename> to MRU list menu entries
#  possibly rotating/swapping existing entries
proc io_mruAdd { filename } {

    global ayprefs

    set found 0
    set i 0
    foreach f $ayprefs(mru) {
	if { $f == $filename } { set found 1; break } else { incr i }
    }

    if { $found == 0 } {
	set ayprefs(mru) [concat [list $filename] $ayprefs(mru)]
	if { [llength $ayprefs(mru)] > 4 } {
	    set ayprefs(mru) [lreplace $ayprefs(mru) 4 end]
	}
    } else {
	# swap MRU list menu entries?
	if { $i > 0 } {
	    set tmp [lindex $ayprefs(mru) 0]
	    set ayprefs(mru) [lreplace $ayprefs(mru) 0 0 $filename]
	    set ayprefs(mru) [lreplace $ayprefs(mru) $i $i $tmp]
	}
    }
    # if

    io_mruUMenu

 return;
}
# io_mruAdd


# io_mruLoad:
#  load file from MRU menu entry <index>
proc io_mruLoad { index } {
    global ayprefs ay_error ay

    if { [lindex $ayprefs(mru) $index] != "" } {
	viewCloseAll
	cS; plb_update
	set ay_error ""
	set ay(askedscriptdisable) 0
	set filename [lindex $ayprefs(mru) $index]

	# swap MRU list menu entries?
	if { $index > 0 } {
	    set tmp [lindex $ayprefs(mru) 0]
	    set ayprefs(mru) [lreplace $ayprefs(mru) 0 0 $filename]
	    set ayprefs(mru) [lreplace $ayprefs(mru) $index $index $tmp]
	    io_mruUMenu
	}

	update

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

	    set filename [file tail $filename]

	    set .fl.con(-prompt) {[file tail [pwd]]>}
	    .fl.con delete end-1lines end
	    Console:prompt .fl.con "\n"
	}

	# read the file
	replaceScene $filename

	if { $ay_error < 2 } {
	    set windowfilename [file tail [file rootname $filename]]
	    wm title . "Ayam - Main - $windowfilename : --"
	    set ay(filename) $filename
	    ayError 4 "replaceScene" "Done reading scene from:"
	    ayError 4 "replaceScene" "$filename"
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
	update
	# reset scene change indicator
	set ay(sc) 0
	foreach view $ay(views) { viewBind $view }
	update

	after idle viewMouseToCurrent
    }

 return;
}
# io_mruLoad


# io_mruUMenu:
#  update MRU menu entries
proc io_mruUMenu { } {
    global ay ayprefs
    set i 0
    set m $ay(filemenu)
    foreach f $ayprefs(mru) {
	set label "[expr $i+1]. [lindex $ayprefs(mru) $i]"
	set len [string length $label]
	if { $len > 20 } {
	    set tail [string range $label [expr $len - 17] end]
	    set head [string range $label 0 2]
	    set label "$head ...$tail"
	}
	$m entryconfigure [expr $i+16] -label $label
	incr i
    }

 return;
}
# io_mruUMenu


# io_warnChanged:
#  raise a warning reqester, that the scene contains unsaved changes
proc io_warnChanged { } {
    global ay ayprefs

    if { $ayprefs(WarnChanged) == 1 } {

	if { $ay(sc) == 1 } {

	    winAutoFocusOff

	    set t "Scene has changed!"
	    set m "Select \"Ok\" to lose all changes.\nSelect \"Cancel\" to stop operation."

	    if { $ayprefs(FixDialogTitles) == 1 } {
		set m "$t\n\n$m"
	    }

set answer [tk_messageBox -title $t -type okcancel -icon warning -message $m]

	    winAutoFocusOn

	    if { $answer == "cancel" } {
		return 1;
	    } else {
		return 0;
	    }

	} else {
	    return 0;
	}
    } else {
	return 0;
    }

}
# io_warnChanged


# io_saveEnv:
#  save working environment scene file
proc io_saveEnv { } {
 global ay ayprefs ay_error tcl_platform

 winAutoFocusOff

 set filename [file nativename $ayprefs(EnvFile)]
 if { $tcl_platform(platform) == "windows" } {
    # Windows sucks big time!
     regsub -all {\\} $filename {/} filename
 }

 if { $filename == "" } {
     set filename "2view.ay"
     set dirname "~"
 } else {
     set dirname [file dirname $filename]
     if { $dirname == "." } { set dirname [pwd] }
 }

 set types {{"Ayam Scene" ".ay"} {"All files" *}}

 if { $tcl_platform(os) != "Darwin" } {
     set savefilename [tk_getSaveFile -filetypes $types -parent .\
	     -initialfile [file tail $filename]\
	     -initialdir $dirname -title "Save environment to:"]
 } else {
     set savefilename [tk_getSaveFile -filetypes $types -parent .\
	     -initialfile [file tail $filename]\
	     -title "Save environment to:"]
 }

 if { $savefilename != "" } {
     if { (![file exists $savefilename]) ||
     ([file exists $savefilename] && [file writable $savefilename]) } {
	 viewUPos
	 io_saveMainGeom
	 selOb
	 uCL cs
	 goTop
	 selOb 0
	 # save scene changed status
	 set temp $ay(sc)
	 set ay_error 0
	 saveScene $savefilename

	 if { $ay_error < 2 } {

	     ayError 4 "saveEnv" "Done writing environment to:"
	     ayError 4 "saveEnv" "$savefilename"

	     if { $ayprefs(EnvFile) != $savefilename } {

		 set t "Make Default?"\

		 set m "Select \"Ok\" to make the\
\nnewly written environment\
\nyour default environment."

                 if { $ayprefs(FixDialogTitles) == 1 } {
		     set m "$t\n\n$m"
		 }

                 set answer [tk_messageBox -type okcancel\
			 -title $t\
			 -icon question -message $m]

		 if { $answer == "cancel" } {
		     winAutoFocusOn
		     return;
		 } else {
		     set ayprefs(EnvFile) $savefilename
		     set ayprefse(EnvFile) $savefilename
		     set ayprefs(LoadEnv) 1
		     set ayprefse(LoadEnv) 1
		 }
	     } else {
		 set ayprefs(LoadEnv) 1
		 set ayprefse(LoadEnv) 1
	     }
	 } else {
	     ayError 2 "saveEnv" "There were errors while writing:"
	     ayError 2 "saveEnv" "$savefilename"
	 }
	 # reset scene changed status to old value
	 set ay(sc) $temp
	 selOb
	 uCL cs
	 rV
     } else {
	 ayError 2 "saveEnv" "Can not write to ${savefilename}!"
     }
     # if
 }
 # if

 winAutoFocusOn

 return;
}
# io_saveEnv


# io_getRIBName:
#  derive a RIB file name for export and image file name for rendering the RIB
#
proc io_getRIBName { } {
    global ay ayprefs

    winAutoFocusOff

    set filename $ay(filename)

    # first, create RIB file name
    if { $ayprefs(RIBFile) == "Ask" || $ayprefs(RIBFile) == "" } {
	set filetypes {{"RIB" ".rib"} {"All files" *}}
	set ribname [tk_getSaveFile -filetypes $filetypes -parent .\
		-title "Export to file:"]
    } else {
	if { $ayprefs(RIBFile) != "Scenefile" && \
		$ayprefs(RIBFile) != "Scene" } {
	    set ribname $ayprefs(RIBFile)
	}

	if { $ayprefs(RIBFile) == "Scenefile" } {
	    if { $filename != "" } {
		set ribname [file rootname [file tail $filename]].rib
	    } else {
		set ribname unnamed.rib
	    }
	}

	if { $ayprefs(RIBFile) == "Scene" } {
	    if { $filename != "" } {
		set ribname [file rootname $filename].rib
	    } else {
		set ribname unnamed.rib
	    }
	}
    }
    # if

    # second, create image file name
    if { $ayprefs(Image) == "Ask" } {
	set filetypes {{"TIF" ".tif"} {"TIFF" ".tiff"} {"All files" *}}
	set imagename "[tk_getSaveFile -filetypes $filetypes -parent .\
		-title "Render to file:"]"
	if { $imagename == "" } { set ribname "" }
    } else {
	if { $ayprefs(Image) == "RIB" } {
	    set imagename [file rootname [file tail $ribname]].tif
	} else {
	    set imagename $ayprefs(Image)
	}
    }
    # if

    winAutoFocusOn

 return [list $ribname $imagename]
}
# io_getRIBName


# exportRIBfC:
#  export RIB from selected camera object
#
proc io_exportRIBfC { } {
    global ay ayprefs ay_error

    set ribname [io_getRIBName]
    set efilename [lindex $ribname 0]
    set imagename [lindex $ribname 1]
    set ay_error ""

    if { $efilename != "" } {
	if { $imagename != "" } {
	    wrib $efilename -image $imagename
	    if { $ay_error < 2 } {
		ayError 4 "exportRIB" "Done exporting scene to:"
		ayError 4 "exportRIB" "$efilename"
	    } else {
		ayError 2 "exportRIB" "Could not export scene!"
	    }
	    # if
	} else {
	    wrib $efilename
	    if { $ay_error < 2 } {
		ayError 4 "exportRIB" "Done exporting scene to:"
		ayError 4 "exportRIB" "$efilename"
	    } else {
		ayError 2 "exportRIB" "Could not export scene!"
	    }
	    # if
	}
	# if
    }
    # if

 return;
}
# io_exportRIBfC


# io_RenderSM:
#  export shadow map RIB and render all shadow maps
#
proc io_RenderSM { w all } {
    global ayprefs ay ay_error

    if { $ayprefs(ShadowMaps) != 2 } {

	winAutoFocusOff

	set t "Need manual ShadowMaps!"
	set m "ShadowMaps are not enabled properly!\n
		\nSelect \"Ok\" to enable them and continue.\
		\nSelect \"Cancel\" to stop operation."

	if { $ayprefs(FixDialogTitles) == 1 } {
	    set m "$t\n\n$m"
	}

	set answer [tk_messageBox -title $t -type okcancel -icon warning\
		     -message $m]

	winAutoFocusOn

	if { $answer == "cancel" } {
	    return 1;
	} else {
	    set ayprefs(ShadowMaps) 2
	    set ayprefse(ShadowMaps) 2
	}
    }

    set ribname [io_getRIBName]
    set efilename [lindex $ribname 0]
    set imagename [lindex $ribname 1]

    set ay_error ""

    if { $efilename != ""} {
	if { $imagename != "" } {
	    if { $all == 1 } {
		wrib $efilename -image $imagename -smonly
	    } else {
		wrib $efilename -image $imagename -smonly -selonly
	    }
	    if { $ay_error < 2 } {
		ayError 4 "Create SM" "Done exporting scene to:"
		ayError 4 "Create SM" "$efilename"
		set render 1
	    } else {
		ayError 2 "Create SM" "Could not export scene!"
		set render 0
	    }
	} else {
	    if { $all == 1 } {
		wrib $efilename -image $imagename -smonly
	    } else {
		wrib $efilename -image $imagename -smonly -selonly
	    }
	    if { $ay_error < 2 } {
		ayError 4 "Create SM" "Done exporting scene to:"
		ayError 4 "Create SM" "$efilename"
		set render 1
	    } else {
		ayError 2 "Create SM" "Could not export scene!"
		set render 0
	    }
	}
	# if
    }
    # if

    if { $render } {
	ayError 4 "Create SM" "Now rendering shadow maps..."

	if { $ayprefs(SMRenderUI) != 1} {
	    set command "exec "
	    regsub -all {%s} $ayprefs(SMRender) $efilename command2
	    append command $command2
	    append command " &"
	    eval [subst "$command"]
	} else {
	    regsub -all {%s} $ayprefs(SMRender) $efilename command
	    runRenderer $w "$command" "$ayprefs(SMRenderPT)"
	}
	# if

	ayError 4 "Create SM" "Done rendering shadow maps!"
    }
    # if

    update
    tmp_clean 0

 return;
}
# io_RenderSM


# io_exportRIBSO:
#  export RIB from all selected objects
#
proc io_exportRIBSO { } {
    global ay ayprefs ay_error

    set ribname [io_getRIBName]
    set efilename [lindex $ribname 0]
    set ay_error ""

    if { $efilename != "" } {
	wrib $efilename -selonly
	if { $ay_error < 2 } {
	    ayError 4 "exportRIB" "Done exporting objects to:"
	    ayError 4 "exportRIB" "$efilename"
	} else {
	    ayError 2 "exportRIB" "Could not export RIB!"
	}
    }
    # if

 return;
}
# io_exportRIBSO


# io_saveMainGeom:
#  fill potentially present "SaveMainGeom" tag with current
#  main and toolbox window geometry
#  in SingleWindow GUI mode, do the same for the pane configuration
#  for a potentially present "SavePaneLayout" tag
proc io_saveMainGeom { } {
    global ay ayprefs tagnames tagvals

    set sel ""
    if { $ay(lb) == 0 } {
	set tree $ay(tree)
	set sel [$tree selection get]
	treeSelect -temp root:0
    } else {
	set lb $ay(olb)
	set sel [$lb curselection]
	goTop
	selOb 0
    }
    getTags tagnames tagvals
    set i 0
    foreach tagname $tagnames {
	if { $ayprefs(SingleWindow) } {
	    if { $tagname == "SavePaneLayout" } {
		setTags -index $i SavePaneLayout [winGetPaneLayout]
	    }
	} else {
	    if { $tagname == "SaveMainGeom" } {
		if { [winfo exists .tbw] } {
		    set geom "1,[winGetGeom .] [winGetGeom .tbw]"
		} else {
		    set geom 0,[winGetGeom .]
		}

		setTags -index $i SaveMainGeom $geom
	    }
	}
	# if
	incr i
    }
    if { $sel != "" } {
	if { $ay(lb) == 0 } {
	    eval [subst "treeSelect -temp $sel"]
	} else {
	    # we can not do this at the moment, level changed
	    # eval "selOb -lb $sel"
	    # so instead we just do
	    uS
	}
    }

 return;
}
# io_saveMainGeom


# io_readMainGeom:
#  update geometry of main and toolbox window with data from potentially
#  present SaveMainGeom tag
#  in SingleWindow GUI mode, do the same for the pane configuration with
#  data from a potentially present "SavePaneLayout" tag
proc io_readMainGeom { } {
    global ay ayprefs tagnames tagvals

    set sel ""
    if { $ay(lb) == 0 } {
	set tree $ay(tree)
	set sel [$tree selection get]
	treeSelect root:0
    } else {
	set lb $ay(olb)
	set sel [$lb curselection]
	goTop
	selOb 0
    }
    getTags tagnames tagvals
    set i 0
    foreach tagname $tagnames {
	if { $ayprefs(SingleWindow) } {
	    if { $tagname == "SavePaneLayout" } {
		winRestorePaneLayout [lindex $tagvals $i]
	    }
	} else {
	    if { $tagname == "SaveMainGeom" } {
		set hasTBGeom 0
		scan [lindex $tagvals $i] "%d," hasTBGeom

		if {$hasTBGeom} {
		    scan [lindex $tagvals $i] "%d,%s %s" dummy mgeom tgeom
		    if { [winfo exists .tbw] } {
			winMoveOrResize .tbw $tgeom
		    }
		} else {
		    scan [lindex $tagvals $i] "%d,%s" dummy mgeom
		}

		winMoveOrResize . $mgeom
	    }
	}
	# if
	incr i
    }
    # foreach

    if { $sel != "" } {
	if { $ay(lb) == 0 } {
	    $tree selection set $sel
	    treeSelect $sel
	} else {
	    $ay(olb) selection set $sel
	    selOb -lb $sel
	}
    }
    # if

 return;
}
# io_readMainGeom


if { $AYWITHAQUA } {
# OpenDocument:
#  some document(s) are dropped onto the application (on MacOSX-Aqua),
#  load them (replace the first, insert the remaining)
proc ::tk::mac::OpenDocument { args } {
    grab .fu
    set j 0

    foreach arg $args {
	if { ([file extension $arg] == ".ay") || \
		 ([file extension $arg] == ".AY") } {

	    regsub -all {\\} $arg {/} filename

	    if { $j == 0 } {
		# close all views
		viewCloseAll

		set ay_error ""

		replaceScene $filename
		if { $ay_error < 2 } {
		    set ay(filename) $filename
		    set windowfilename [file tail [file rootname $filename]]
		    wm title . "Ayam - Main - $windowfilename : --"
		    ayError 4 "replaceScene" "Done reading scene from:"
		    ayError 4 "replaceScene" "$filename"
		    if { [file exists $filename] } {
			set dirname [file dirname $filename]
			cd $dirname
			ayam_updateprompt ay uc w
		    }
		    io_mruAdd $filename
		} else {
		    ayError 2 "replaceScene" "There were errors while loading:"
		    ayError 2 "replaceScene" "$filename"
		}
		set j 1
	    } else {
		set ay_error ""
		insertScene $filename
		if { $ay_error < 2 } {
		    ayError 4 "insertScene" "Done inserting scene from:"
		    ayError 4 "insertScene" "$filename"
		} else {
		    ayError 2 "insertScene" "There were errors while loading:"
		    ayError 2 "insertScene" "$filename"
		}
		# if
	    }
	    # if
	    uS; rV
	} else {
	    # extension is not .ay
	    if { [file extension $filename] != "" } {
		# try to import the file
		io_importScene $filename
		return;
	    }
	}
	# if
    }
    # foreach

    grab release .fu

 return;
}
# OpenDocument

}
# if


# io_exit
#  arrange for preferences to be saved and temporary files to be removed;
#  then exit the application
proc io_exit { } {
    global ayprefs AYENABLEFEXIT

    if { ! [io_warnChanged] } {

        # remove all temporary files
        catch { tmp_clean 1 }

        if { $ayprefs(AutoSavePrefs) == 1 } {
            catch { prefs_save }
        }

        puts "Good bye!"
        update
        if { $AYENABLEFEXIT == 1 } {
            fastExit
        } else {
	    if { [ info commands realexit ] != "" } {
		realexit
	    } else {
		exit
	    }
        }
	# if
    }
    # if

 return;
}
# io_exit


# io_appext:
#  helper procedure to append extension <extension> to <filename> if
#  <filename> not already has the proper extension
#  (case insensitive match!)
proc io_appext { filename extension } {
    global ayprefs

    set newfilename $filename

    if { $ayprefs(AddExtensions) } {
	if { ![string equal -nocase [file extension $filename] $extension] } {
	    append newfilename $extension
	}
    }

 return $newfilename
}
# io_appext


# io_importScene:
#  import a scene, automatically loading the corresponding plugin
#
proc io_importScene { filename } {
    global ay ayprefs

    if { $filename == "" } {
	return;
    }

    set ext [file extension $filename ]
    set i 0
    set imported 0
    foreach alext $ayprefs(ALFileTypes) {
	# look for matching supported file name extension
	if { ! [string compare -nocase $ext $alext] } {
	    # get the name of the plugin that supports $ext files
	    set plugin [lindex $ayprefs(ALPlugins) $i]
	    # XXXX import procedure name
	    set import_proc "${plugin}_import"
	    #
	    if { ! [info exists $import_proc] } {
		io_lcAuto $plugin
	    }
	    # XXXX import options array
	    set option_array "${plugin}_options"
	    global $option_array
	    # set file name to import in the import options GUI
	    set ${option_array}(filename) $filename
	    update
	    set body ""
	    catch { set body [ info body $import_proc ] }
	    if { $body != "" } {
		set imported 1
		# now, call the import procedure
		$import_proc
	    } else {
		ayError 2 "io_importScene" "Failed to load plugin: $plugin"
	    }
	}
	# if
	if { $imported } {
	    break
	}
	incr i
    }
    # foreach

    if { ! $imported } {
	ayError 2 "io_importScene" "Unknown extension: $ext"
    }

    winAutoFocusOn

 return;
}
# io_importScene


# io_exportScene:
#  export a scene, automatically loading the corresponding plugin
#
proc io_exportScene { filename } {
    global ay ayprefs

    if { $filename == "" } {
	return;
    }

    set ext [file extension $filename ]
    set i 0
    set exported 0
    foreach alext $ayprefs(ALFileTypes) {
	# look for matching supported file name extension
	if { ! [string compare -nocase $ext $alext] } {
	    # get the name of the plugin that supports $ext files
	    set plugin [lindex $ayprefs(ALPlugins) $i]
	    # XXXX export procedure name
	    set export_proc "${plugin}_export"
	    #
	    if { ! [info exists $export_proc] } {
		io_lcAuto $plugin
	    }
	    # XXXX export options array
	    set option_array "${plugin}_options"
	    global $option_array
	    # set file name to export in the export options GUI
	    set ${option_array}(filename) $filename
	    update
	    set body ""
	    catch { set body [ info body $export_proc ] }
	    if { $body != "" } {
		set exported 1
		# now, call the export procedure
		$export_proc
	    } else {
		ayError 2 "io_exportScene" "Failed to load plugin: $plugin"
	    }
	}
	# if
	if { $exported } {
	    break
	}
	incr i
    }
    # foreach

    if { ! $exported } {
	ayError 2 "io_exportScene" "Unknown extension: $ext"
    }

    winAutoFocusOn

 return;
}
# io_exportScene
