# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2001 by Randolf Schultz
# (rschultz@informatik.uni-rostock.de) and others.
#
# All rights reserved.
#
# See the file License for details.

# prefs.tcl - managing preferences


# prefs_set:
#  
proc prefs_set {} {
    global ayprefs
    set tcl_precision $ayprefs(TclPrecision)
    setPrefs
 return;
}
# prefs_set

proc prefs_rcnb { nb page } {
    global ay
    $nb configure -height [winfo reqheight [$nb getframe $page] ]
    set ay(prefssection) $page
return;
}

# prefs_open:
#  
proc prefs_open {} {
    global ay ayprefs ayprefse

    # copy array ayprefs to ayprefse (we operate on this second array)
    set avnames [array names ayprefs]
    foreach j $avnames {
	set ayprefse($j) $ayprefs($j)
    }

    set w .prefsw
    catch {destroy $w}
    toplevel $w
    wm title $w "Ayam Preferences"
    wm iconname $w "Ayam"

    wm protocol $w WM_DELETE_WINDOW {
	global ay ayprefs ayprefse
	set avnames [array names ayprefs]
	foreach j $avnames {
	    # unset removes all traces
	    unset ayprefse($j)
	}
	focus .
	destroy .prefsw
    }

    # section select
#    set f [frame $w.f1 -bd 1]


    # Tabbed-Notebook
    set f [frame $w.f2 -relief sunken -bd 2]
    pack $f -in $w -fill both -expand yes -pady 2
    set nb [NoteBook $w.f2.nb -height 0]

    # PrefsGUIs
    # Main
    set fw [$nb insert end Main -text Main\
	    -raisecmd "prefs_rcnb $nb Main"]
    addText $fw e0 "Shaders:"
    addMDir $fw ayprefse Shaders
    addCommand $fw c1 "Scan Shaders" {
	set ayprefs(Shaders) $ayprefse(Shaders)
	update
	shader_scanAll
    }
    addText $fw e1 "GUI:"
    addCheck $fw ayprefse AutoResize
    addCheck $fw ayprefse TwmCompat
    addCheck $fw ayprefse ListTypes
    addParam $fw ayprefse TclPrecision { 4 5 6 }
    addCheck $fw ayprefse AutoSavePrefs
    addCheck $fw ayprefse LoadEnv
    addFile $fw ayprefse EnvFile
    addMFile $fw ayprefse Scripts
    addMDir $fw ayprefse Plugins
    addString $fw ayprefse Docs
    addString $fw ayprefse TmpDir

    # Modeling
    set fw [$nb insert end Modeling -text Modeling\
	    -raisecmd "prefs_rcnb $nb Modeling"]
    addParam $fw ayprefse PickEpsilon
    addParam $fw ayprefse HandleSize
    addCheck $fw ayprefse LazyNotify
    addCheck $fw ayprefse EditSnaps
    addParam $fw ayprefse UndoLevels { 1 10 20 }

    # Drawing
    set fw [$nb insert end Drawing -text Drawing\
	    -raisecmd "prefs_rcnb $nb Drawing"]

    addParam $fw ayprefse Tolerance { 5 10 25 50 75 90 }
    set l $ay(npdisplaymodes)
    addMenu $fw ayprefse DisplayMode $l
    set l $ay(ncdisplaymodes)
    addMenu $fw ayprefse NCDisplayMode $l
    addCheck $fw ayprefse UseMatColor
    addColor $fw ayprefse Background
    addColor $fw ayprefse Object
    addColor $fw ayprefse Selection
    addColor $fw ayprefse Grid
    addColor $fw ayprefse Tag
    addColor $fw ayprefse Shade

    # RIB Export
    set fw [$nb insert end RIB-Export -text RIB-Export\
	    -raisecmd "prefs_rcnb $nb RIB-Export"]
    addFile $fw ayprefse RIBFile
    addFile $fw ayprefse Image
    addCheck $fw ayprefse ResInstances
    addCheck $fw ayprefse CheckLights
    addMenu $fw ayprefse DefaultMat [list none matte default]
    addCheck $fw ayprefse RIStandard
    addCheck $fw ayprefse WriteIdent
    addCheck $fw ayprefse ShadowMaps
    addCheck $fw ayprefse ExcludeHidden
    addString $fw ayprefse QRender
    addCheck $fw ayprefse QRenderUI
    addString $fw ayprefse QRenderPT
    addString $fw ayprefse Render
    addCheck $fw ayprefse RenderUI
    addString $fw ayprefse RenderPT

    # Misc
    set fw [$nb insert end Misc -text Misc\
	    -raisecmd "prefs_rcnb $nb Misc"]

    addText $fw e0 "Errors:"
    addCheck $fw ayprefse RedirectTcl
    addCheck $fw ayprefse Logging
    addString $fw ayprefse LogFile
    addText $fw e1 "Mops Import:"
    addCheck $fw ayprefse MIResetDM
    addCheck $fw ayprefse MIResetST

    # select last selected preference section
    pack $nb -fill both -expand yes
    $nb raise $ay(prefssection)
#    update
    $nb see $ay(prefssection)
#    update
    prefs_rcnb $nb $ay(prefssection)

    # controlling buttons
    set f [frame $w.f3]
    button $f.bok -text "Ok" -width 8 -command { 
	global ay ayprefs ayprefse

	# copy array ayprefse to ayprefs
	set avnames [array names ayprefs]
	foreach j $avnames {

	    set ayprefs($j) $ayprefse($j)
	    # unset removes all traces
	    unset ayprefse($j)
	}
	prefs_set
	focus .
	destroy .prefsw
    }

    button $f.bap -text "Apply" -width 8 -command { 
	global ay ayprefs ayprefse

	# copy array ayprefse to ayprefs
	set avnames [array names ayprefs]
	foreach j $avnames {

	    set ayprefs($j) $ayprefse($j)
	}
	prefs_set
	rV
    }
    button $f.bdef -text "Def." -width 8 -command {
	global ay ayprefse ayprefsdefaults
	set avnames [array names ayprefsdefaults]
	foreach j $avnames {
	    set ayprefse($j) $ayprefsdefaults($j)
	}
    }
    button $f.bca -text "Cancel" -width 8 -command {
	global ay ayprefs ayprefse
	set avnames [array names ayprefs]
	foreach j $avnames {
	    # unset removes all traces
	    unset ayprefse($j)
	}
	focus .
	destroy .prefsw }

    pack $f.bok $f.bap $f.bdef $f.bca -in $f -side left -fill x -expand yes
    pack $f -in $w -side bottom -fill x -expand no

    # center window
    update
    winCenter $w

    focus $f.bok
    tkwait window $w

 return;
}
# prefs_open


# prefs_save:
#
proc prefs_save { } {
    global ay
    set ayrc $ay(ayamrc)

    if { [file exists $ayrc ] } {
	if { ! [file writable $ayrc ] } {
	    ayError 1 "prefs_save" "\$ayrc is not writable! No preferences saved!"
	    return;
	}
 
	file copy -force -- $ayrc ${ayrc}.bak

	set oldfile [open ${ayrc}.bak]
	set newfile [open $ayrc w]


    } else {
	if { $ayrc == "" } { set ayrc "~/.ayamrc" }
	set newfile [open $ayrc w]
    }

    global ayprefs aymainshortcuts ayviewshortcuts riattr riopt

    # get main geometry
    global tcl_platform
    if { ($tcl_platform(platform) != "windows") &&\
	    ($ayprefs(TwmCompat) != 1) } {
	set x [winfo rootx .]
	set y [winfo rooty .]
	set ng ""
	if { $x >= 0 } { append ng "+$x" } else { append ng "-$x" }
	if { $y >= 0 } { append ng "+$y" } else { append ng "-$y" }
	set ayprefs(mainGeom) $ng
    } else {
	set ayprefs(mainGeom) [wm geom .]
    }

    # get toolbox geometry
    if { [winfo exists .tbw] } {
	if { ($tcl_platform(platform) != "windows") &&\
		($ayprefs(TwmCompat) != 1) } {
	    set x [winfo rootx .tbw]
	    set y [winfo rooty .tbw]
	    set ng ""
	    if { $x >= 0 } { append ng "+$x" } else { append ng "-$x" }
	    if { $y >= 0 } { append ng "+$y" } else { append ng "-$y" }
	    set ayprefs(toolBoxGeom) $ng
	} else {
	    set ayprefs(toolBoxGeom) [wm geom .tbw]
	}
    }

    # write header
    puts $newfile "# Emacs, this is -*- Mode: Tcl -*-\n"
    puts $newfile "# These are the saved preference settings for Ayam, a free"
    puts $newfile "# 3D modeling environment for the RenderMan interface."
    puts $newfile "# See: http://www.ayam3d.org/\n"
    puts $newfile "# Edit, if you wish, but keep in mind:"
    puts $newfile "# _This file is parsed by Tcl!_\n"
    puts $newfile "# Preferences:"

    # write preferences
    set names [lsort [array names ayprefs ]]
    foreach pref $names {
	eval [subst "set val {{\$ayprefs($pref)}}"]
	puts $newfile "set ayprefs($pref) $val"
    }

    # now write the hotkeys
    puts $newfile "# Hotkeys:"

    # hotkeys for main window
    puts $newfile "# main window:"
    foreach key [array names aymainshortcuts ] {
	eval [subst "set val {{\$aymainshortcuts($key)}}"]
	puts $newfile "set aymainshortcuts($key) $val"
    }

    # hotkeys for view windows
    puts $newfile "# view windows:"
    foreach key [array names ayviewshortcuts ] {
	eval [subst "set val {{\$ayviewshortcuts($key)}}"]
	puts $newfile "set ayviewshortcuts($key) $val"
    }

    # now write RiOption and RiAttribute tag database
    # RiOptions
    puts $newfile "# RiOptions:"
    foreach key [array names riopt ] {
	eval [subst "set val {{\$riopt($key)}}"]
	puts $newfile "set riopt($key) $val"
    }

    # RiAttributes
    puts $newfile "# RiAttributes:"
    foreach key [array names riattr ] {
	eval [subst "set val {{\$riattr($key)}}"]
	puts $newfile "set riattr($key) $val"
    }

    # write footer
    puts $newfile "return;"

    close $newfile
    catch {close $oldfile}
    update
    ayError 4 "prefs_save" "Done saving preferences to $ayrc."

    update
    return;
}
# prefs_save