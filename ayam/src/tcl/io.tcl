# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2001 by Randolf Schultz
# (rschultz@informatik.uni-rostock.de) and others.
#
# All rights reserved.
#
# See the file License for details.

# io.tcl - scene io helper procs

# io_replaceScene:
# clear scene, then read new
#
proc io_replaceScene { } {
    global ay tcl_platform

    set filename $ay(filename)

    if { $filename == "" } {
	set dirname [pwd]
    } else {
	set dirname [file dirname $filename]
	if { $dirname == "." } { set dirname [pwd] }
    }

    set types {{"Ayam Scene" ".ay"} {"All files" *}}

    if { $tcl_platform(os) != "Darwin" } {
	set newfilename [tk_getOpenFile -filetypes $types -parent .\
		-initialfile [file tail $filename] -initialdir $dirname\
		-title "Select file to load:"]
    } else {
	set newfilename [tk_getOpenFile -filetypes $types -parent .\
		-initialfile [file tail $filename]\
		-title "Select file to load:"]
    }

    if { $newfilename != "" } {
	viewCloseAll
	cS; plb_update
	#    .fu.fMain.fHier.fsel.bnon invoke
	update
	set filename $newfilename
	global ay_error ayprefs
	set ay_error ""
	update

	# make backup copy
	if { $ayprefs(BakOnReplace) == 1 } {
	    set err [ catch {
		file copy -force -- $filename ${filename}${ayprefs(BackupExt)}
	    } ]
	}

	replaceScene $filename

	if { $ay_error < 2 } {
	    set windowfilename [file tail [file rootname $filename]]
	    wm title . "Ayam - Main - $windowfilename"
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

 return;
}
# io_replaceScene


# io_insertScene:
#  insert a scene
#
proc io_insertScene { } {
    global ay tcl_platform

    set filename $ay(filename)

    set types {{"Ayam Scene" ".ay"} {"All files" *}}

    if { $filename == "" } {
	set dirname [pwd]
    } else {
	set dirname [file dirname $filename]
	if { $dirname == "." } { set dirname [pwd] }
    }
    set types {{"Ayam Scene" ".ay"} {"All files" *}}

    if { $tcl_platform(os) != "Darwin" } {
	set ifilename [tk_getOpenFile -filetypes $types -parent .\
		-initialfile [file tail $filename] -initialdir $dirname\
		-title "Select file to load:"]
    } else {
	set ifilename [tk_getOpenFile -filetypes $types -parent .\
		-initialfile [file tail $filename]\
		-title "Select file to load:"]
    }

    if { $ifilename != "" } {
	cS; plb_update
	update
	global ay ay_error
	set ay_error ""

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

 return;
}
# io_insertScene


# io_saveScene:
#  save a scene file
#
proc io_saveScene { ask selected } {
    global ay tcl_platform

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
	set types {{"Ayam Scene" ".ay"} {"All files" *}}

	if { $tcl_platform(os) != "Darwin" } {
	    set filename [tk_getSaveFile -filetypes $types -parent .\
		    -initialfile [file tail $filename]\
		    -initialdir $dirname -title "Save scene to:"]
	} else {
	    set filename [tk_getSaveFile -filetypes $types -parent .\
		    -initialfile [file tail $filename]\
		    -title "Save scene to:"]
	}
    }

    if { $filename != "" } {
	# fix window positions
	viewUPos
	global ay_error
	set ay_error ""
	saveScene $filename $selected
	if { $ay_error < 2 } {
	    set windowfilename [file tail [file rootname $filename]]
	    wm title . "Ayam - Main - $windowfilename"
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

 return;
}
# io_saveScene


# exportRIB:
# 
# 
proc io_exportRIB { {expview "" } } {
    global ay

    set w .exportRIBw
    catch {destroy $w}
    toplevel $w
    wm title $w "RIB Export"
    wm iconname $w "Ayam"
    wm transient $w .

    set f [frame $w.f1]
    pack $f -in $w -side top -fill x

    label $f.l -text "Select View:"
    set f [frame $f.fview]
    scrollbar $f.sc -command {.exportRIBw.f1.fview.lb yview} -takefocus 0
    listbox $f.lb -width 15 -height 5 -selectmode single\
	    -exportselection 0 -yscrollcommand {.exportRIBw.f1.fview.sc set}

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

	scan $i ".%s" name
	if { $ay(cVType) != 4 } {
	    $f.lb insert end "$name ([lindex $ay(viewtypenames) $ay(cVType)])"
	    incr index
	}


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
	    set sname [.exportRIBw.f1.fview.lb get $selection]
	    scan $sname "%s (%s)" name type

	    set ribname [io_getRIBName]
	    set efilename [lindex $ribname 0]
	    set imagename [lindex $ribname 1]

	    if { $efilename != "" } {
		if { $imagename != "" } {
		    .$name.f3D.togl wrib -file $efilename -image $imagename
		    ayError 4 "exportRIB" "Done exporting scene to:"
		    ayError 4 "exportRIB" "$efilename"

		} else {
		    .$name.f3D.togl wrib -file $efilename
		    ayError 4 "exportRIB" "Done exporting scene to:"
		    ayError 4 "exportRIB" "$efilename"
		}
	    }
	}
	focus .
	grab release .exportRIBw
	destroy .exportRIBw
    }

    button $f.bca -text "Cancel" -pady $ay(pady) -width 5 -command "\
	    grab release .exportRIBw; focus .; destroy $w"

    pack $f.bok $f.bca -in $f -side left -fill x -expand yes
    pack $f -in $w -side bottom -fill x

    winCenter $w
    grab $w
    focus $f.bok
    tkwait window $w

 return;
}
# io_exportRIB


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
proc io_lcAuto {  } {
    global ay ayprefs

    set name [string tolower $ay(autoload)].so

    set paths [split "$ayprefs(Plugins)" $ay(separator)]

    foreach path $paths {
	set filename [file join $path $name]
	set sopath [file dirname $filename]
	if { "$sopath" != "" } {
	    set oldcdir [pwd]
	    cd $sopath
	    catch {load $filename}
	    cd $oldcdir
	} else {
	    catch {load $filename}
	}

	set co $ay(co)

	foreach o $ay(co) {
	    if { $o == $ay(autoload) } { return }
	}

    }

 return;
}
# io_lcAuto


# loadCustomp:
# 
# 
proc io_loadCustom { } {
    global ayprefs

    set filetypes {{"Custom Object" ".so"} {"Custom Object" ".dll"} {"All files" *}}

    set filename [tk_getOpenFile -filetypes $filetypes -parent .\
	    -title "Select Custom Object:"]

    if { $filename != "" } { 
	io_lc $filename
    }

 return;
}
# io_loadCustom

uplevel #0 { array set mopsi_options {
ResetDM 1
ResetST 1
ifilename ""
}   }

# io_importMops:
#
#
proc io_importMops { } {
    global ay tcl_platform

    set filename $ay(filename)

    set types {{"Mops Scene" ".mop"} {"All files" *}}

    if { $filename == "" } {
	set dirname [pwd]
    } else {
	set dirname [file dirname $filename]
	if { $dirname == "." } { set dirname [pwd] }
    }
    set types {{"Mops Scene" ".mop"} {"All files" *}}

    if { $tcl_platform(os) != "Darwin" } {
	set ifilename [tk_getOpenFile -filetypes $types -parent .\
		-initialfile [file tail $filename] -initialdir $dirname\
		-title "Select file to import:"]
    } else {
	set ifilename [tk_getOpenFile -filetypes $types -parent .\
		-initialfile [file tail $filename]\
		-title "Select file to import:"]
    }

    if { $ifilename != "" } {
	global ay_error mopsi_options
	set ay_error ""
	set mopsi_options(ifilename) $ifilename

	set w .mopI
	catch {destroy $w}
	toplevel $w
	wm title $w "Import Options"
	wm iconname $w "Ayam"
	wm transient $w .

	set f [frame $w.f1]
	pack $f -in $w -side top -fill x

	addCheckB $f mopsi_options ResetDM [ms mopsi_options_ResetDM]
	addCheckB $f mopsi_options ResetST [ms mopsi_options_ResetST]
	
	set f [frame $w.f2]
	button $f.bok -text "Ok" -width 5 -command {
	    global mopsi_options ay_error
	    cS; plb_update
	    set ay_error ""
	    update
	    foreach view $ay(views) { viewUnBind $view }
	    update

	    set filename $mopsi_options(ifilename)
	    importMops $filename

	    if { $ay_error < 2 } {
		ayError 4 "importMops" "Done importing Mops scene from:"
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
	    
	    destroy .mopI
	    foreach view $ay(views) { viewBind $view }
	    update

	    after idle viewMouseToCurrent
	}

	button $f.bca -text "Cancel" -width 5 -command "\
		focus .;\
		destroy .mopI"

	pack $f.bok $f.bca -in $f -side left -fill x -expand yes
	pack $f -in $w -side bottom -fill x

	winCenter $w
	
	focus $w.f2.bok

    }

 return;
}
# io_importMops


# io_mruAdd
#  add <file> to MRU menu entries
#  possibly rotating existing entries
proc io_mruAdd { file } {

    global ayprefs

    set found 0

    foreach f $ayprefs(mru) {
	if { $f == $file } { set found 1 }
    }
    if { $found == 0 } {
	set ayprefs(mru) [concat [list $file] $ayprefs(mru)]
    }
    if { [llength $ayprefs(mru)] > 4 } {
	set ayprefs(mru) [lreplace $ayprefs(mru) 4 end]
    }

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
	set filename [lindex $ayprefs(mru) $index]
	update

	# make backup copy
	if { $ayprefs(BakOnReplace) == 1 } {
	    set err [ catch {
		file copy -force -- $filename ${filename}${ayprefs(BackupExt)}
	    } ]
	}

	replaceScene $filename

	if { $ay_error < 2 } {

	    set windowfilename [file tail [file rootname $filename]]
	    wm title . "Ayam - Main - $windowfilename"
	    set ay(filename) $filename
	    ayError 4 "replaceScene" "Done reading scene from:"
	    ayError 4 "replaceScene" "$filename"
	    set ay(sc) 0

	    if { [file exists $filename] } {
		set dirname [file dirname $filename]
		cd $dirname
		
		set .fl.con(-prompt) {[file tail [pwd]]>}
		.fl.con delete end-1lines end
		Console:prompt .fl.con "\n"
	    }
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
	update

	foreach view $ay(views) { viewBind $view }
	update

	after idle viewMouseToCurrent
	
    }

 return;
}
# io_mruLoad


# io_mruUMenu:
#  update MRU menu entries
proc io_mruUMenu {  } {
    global ay ayprefs
    set i 0
    set m $ay(filemenu)
    foreach f $ayprefs(mru) {
	set label "[expr $i+1]. [lindex $ayprefs(mru) $i]"
	set len [string length $label]
	if {  $len > 20 } {
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
#  
proc io_warnChanged {  } {
    global ay ayprefs

    if { $ayprefs(WarnChanged) == 1 } {

	if { $ay(sc) == 1 } {
	    set t "Scene has changed!"
	    set m "Select \"Ok\" to lose all changes.\nSelect \"Cancel\" to stop operation."
set answer [tk_messageBox -title $t -type okcancel -icon warning -message $m]
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
#  
proc io_saveEnv {  } {
 global ay ayprefs ay_error tcl_platform

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
	 selOb
	 uCL cs
	 goTop
	 selOb 0
	 # save scene changed status
	 set temp $ay(sc)
	 set ay_error 0
	 saveScene $savefilename 1

	 if { $ay_error < 2 } {
	     ayError 4 "saveEnv" "Done writing environment to:"
	     ayError 4 "saveEnv" "$savefilename"

	     if { $ayprefs(EnvFile) != $savefilename } {

		 set m "Select \"Ok\" to adapt the\n\
preferences to load the\nnewly written environment\n\
on the next start."

                 set answer [tk_messageBox -type okcancel\
			 -title "Adapt Preferences?"\
			 -icon question -message $m]

		 if { $answer == "cancel" } {
		     return;
		 } else {
		     set ayprefs(EnvFile) $savefilename
		 }
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
 }

 return;
}
# io_saveEnv


# io_getRIBName:
#
proc io_getRIBName { } {
    global ay ayprefs

    set filename $ay(filename)

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

 return [list $ribname $imagename ]
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
	    wrib -file $efilename -image $imagename
	    if { $ay_error < 2 } {
		ayError 4 "exportRIB" "Done exporting scene to:"
		ayError 4 "exportRIB" "$efilename"
	    } else {
		ayError 2 "exportRIB" "Could not export scene!"
	    }

	} else {
	    wrib -file $efilename
	    if { $ay_error < 2 } {
		ayError 4 "exportRIB" "Done exporting scene to:"
		ayError 4 "exportRIB" "$efilename"
	    } else {
		ayError 2 "exportRIB" "Could not export scene!"
	    }
	}
    }

 return;
}
# io_exportRIBfC


##############################
# io_RenderSM:
#  export shadow map RIB and render all shadow maps
proc io_RenderSM { } {
    global env ayprefs ay tcl_platform ay_error

    if { $ayprefs(ShadowMaps) != 2 } {
	set t "ShadowMaps are not enabled!"
	set m "ShadowMaps are not enabled\nin the preferences.\
\nSelect \"Ok\" to enable them and continue.\
\nSelect \"Cancel\" to stop operation."
set answer [tk_messageBox -title $t -type okcancel -icon warning -message $m]
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
	    wrib -file $efilename -image $imagename -smonly
	    if { $ay_error < 2 } {
		ayError 4 "Create SM" "Done exporting scene to:"
		ayError 4 "Create SM" "$efilename"
		set render 1
	    } else {
		ayError 2 "exportRIB" "Could not export scene!"
		set render 0
	    }
	} else {
	    wrib -file $efilename -image $imagename -smonly
	    if { $ay_error < 2 } {
		ayError 4 "Create SM" "Done exporting scene to:"
		ayError 4 "Create SM" "$efilename"
		set render 1
	    } else {
		ayError 2 "exportRIB" "Could not export scene!"
		set render 0
	    }
	}
	# if
    }
    # if

    if { $render } { 
	ayError 4 "Create SM" "Now rendering all shadow maps..."
    
	if { $ayprefs(SMRenderUI) != 1} {
	    set command "exec "

	    regsub -all {%s} $ayprefs(SMRender) $efilename command2

	    append command $command2
	    append command " &"

	    eval [subst "$command"]
	} else {
     
	    regsub -all {%s} $ayprefs(SMRender) $efilename command
	
	    runRenderer "$command" "$ayprefs(SMRenderPT)"

	}
	# if
    }
    # if

    update
    tmp_clean 0

 return;
}
#io_RenderSM


# exportRIBSO:
#  export RIB from all selected objects
# 
proc io_exportRIBSO { } {
    global ay ayprefs ay_error

    set ribname [io_getRIBName]
    set efilename [lindex $ribname 0]
    set ay_error ""

    if { $efilename != "" } {
	wrib -file $efilename -selonly
	if { $ay_error < 2 } {
	    ayError 4 "exportRIB" "Done exporting objects to:"
	    ayError 4 "exportRIB" "$efilename"
	} else {
	    ayError 2 "exportRIB" "Could not export RIB!"
	}
    }

 return;
}
# io_exportRIBSO
