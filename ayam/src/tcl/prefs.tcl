
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
#  transfer preference settings to C-context
proc prefs_set {} {
    global ayprefs tcl_precision
    set tcl_precision $ayprefs(TclPrecision)
    setPrefs
 return;
}
# prefs_set


# prefs_rsnb:
#  resize notebook nb so that the page page is displayed in full size
proc prefs_rsnb { nb page } {
    global ay
    update
    $nb configure -height [winfo reqheight [$nb getframe $page] ]
    set ay(prefssection) $page
 return;
}
# prefs_rsnb


# prefs_open:
#  open the preferences editor
proc prefs_open {} {
    global ay ayprefs ayprefse tcl_platform aymainshortcuts

    # copy array ayprefs to ayprefse (we operate on this second array)
    set avnames [array names ayprefs]
    foreach j $avnames {
	set ayprefse($j) $ayprefs($j)
    }

    set w .prefsw
    catch {destroy $w}

    set width 370
    #if { $tcl_platform(platform) == "windows" } { 
    #set width 400
    #}
    toplevel $w -width $width -height 400
    wm title $w "Ayam Preferences"
    wm iconname $w "Prefs"
    wm withdraw $w

    # center window
    update idletasks

    set x [expr [winfo screenwidth $w]/2 - [winfo reqwidth $w]/2 \
            - [winfo vrootx [winfo parent $w]]]
    set y [expr [winfo screenheight $w]/2 - [winfo reqheight $w]/2 \
            - [winfo vrooty [winfo parent $w]]]

    winMoveOrResize $w "+${x}+${y}"

    # bind to close button of window decoration
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

    # Tabbed-Notebook
    set f [frame $w.f2 -relief sunken -bd 2]
    pack $f -in $w -fill both -expand yes -pady 2
    set nb [NoteBook $w.f2.nb -height 0]

    # PrefsGUIs
    # Main
    set fw [$nb insert end Main -text Main\
	    -raisecmd "prefs_rsnb $nb Main"]
    addText $fw e0 "Shaders:"
    addMDirB $fw ayprefse Shaders [ms ayprefse_Shaders]
    addCommandB $fw c1 "Scan Shaders" {
	set ayprefs(Shaders) $ayprefse(Shaders)
	update
	shader_scanAll
    } [ms ayprefse_ScanShaders]
    addText $fw e1 "GUI:"
    set l $ay(locales)
    addStringB $fw ayprefse Locale [ms ayprefse_Locale] $l
    addCheckB $fw ayprefse AutoResize [ms ayprefse_AutoResize]
    addCheckB $fw ayprefse AutoFocus [ms ayprefse_AutoFocus]
    addCheckB $fw ayprefse TwmCompat [ms ayprefse_TwmCompat]
    addCheckB $fw ayprefse ListTypes [ms ayprefse_ListTypes]
    addCheckB $fw ayprefse AutoSavePrefs [ms ayprefse_AutoSavePrefs]
    addCheckB $fw ayprefse LoadEnv [ms ayprefse_LoadEnv]
    addCheckB $fw ayprefse NewLoadsEnv [ms ayprefse_NewLoadsEnv]
    addFileB $fw ayprefse EnvFile [ms ayprefse_EnvFile]
    addMFileB $fw ayprefse Scripts [ms ayprefse_Scripts]
    addMDirB $fw ayprefse Plugins [ms ayprefse_Plugins]
    addStringB $fw ayprefse Docs [ms ayprefse_Docs]
    addStringB $fw ayprefse TmpDir [ms ayprefse_TmpDir]

    # Modeling
    set fw [$nb insert end Modeling -text Modeling\
	    -raisecmd "prefs_rsnb $nb Modeling"]
    addParamB $fw ayprefse PickEpsilon [ms ayprefse_PickEpsilon]
    addParamB $fw ayprefse HandleSize [ms ayprefse_HandleSize]
    addCheckB $fw ayprefse LazyNotify [ms ayprefse_LazyNotify]
    addCheckB $fw ayprefse EditSnaps [ms ayprefse_EditSnaps]
    addParamB $fw ayprefse UndoLevels [ms ayprefse_UndoLevels] { -1 2 5 10 20 }

    # Drawing
    set fw [$nb insert end Drawing -text Drawing\
	    -raisecmd "prefs_rsnb $nb Drawing"]

    addParamB $fw ayprefse Tolerance [ms ayprefse_Tolerance]\
	    { 5 10 25 50 75 90 }
    set l $ay(npdisplaymodes)
    addMenuB $fw ayprefse DisplayMode [ms ayprefse_DisplayMode] $l
    set l $ay(ncdisplaymodes)
    addMenuB $fw ayprefse NCDisplayMode [ms ayprefse_NCDisplayMode] $l
    addCheckB $fw ayprefse UseMatColor [ms ayprefse_UseMatColor]
    addColorB $fw ayprefse Background [ms ayprefse_Background]
    addColorB $fw ayprefse Object [ms ayprefse_Object]
    addColorB $fw ayprefse Selection [ms ayprefse_Selection]
    addColorB $fw ayprefse Grid [ms ayprefse_Grid]
    addColorB $fw ayprefse Tag [ms ayprefse_Tag]
    addColorB $fw ayprefse Shade [ms ayprefse_Shade]
    addColorB $fw ayprefse Light [ms ayprefse_Light]

    # RIB Export
    set fw [$nb insert end RIB-Export -text RIB-Export\
	    -raisecmd "prefs_rsnb $nb RIB-Export"]
    addFileB $fw ayprefse RIBFile [ms ayprefse_RIBFile]\
	    [list Scenefile Scene Ask]
    addFileB $fw ayprefse Image [ms ayprefse_Image] [list RIB Ask]
    addCheckB $fw ayprefse ResInstances [ms ayprefse_ResInstances]
    addCheckB $fw ayprefse CheckLights [ms ayprefse_CheckLights]
    addMenuB $fw ayprefse DefaultMat [ms ayprefse_DefaultMat]\
	    [list none matte default]
    addCheckB $fw ayprefse RIStandard [ms ayprefse_RIStandard]
    addCheckB $fw ayprefse WriteIdent [ms ayprefse_WriteIdent]
    addMenuB $fw ayprefse ShadowMaps [ms ayprefse_ShadowMaps]\
	    [list Never Automatic Manual]
    addCheckB $fw ayprefse ExcludeHidden [ms ayprefse_ExcludeHidden]
    addMenuB $fw ayprefse RenderMode [ms ayprefse_RenderMode]\
	    [list CommandLineArg RiDisplay]
    addStringB $fw ayprefse QRender [ms ayprefse_QRender]\
	    [list "rgl %s" "rgl -rd 10 %s"]
    addCheckB $fw ayprefse QRenderUI [ms ayprefse_QRenderUI]
    addStringB $fw ayprefse QRenderPT [ms ayprefse_QRenderPT]\
	    [list "R90000 %d" "Done computing %d" "%d"]
    addStringB $fw ayprefse Render [ms ayprefse_Render]\
	    [list "rendrib -d 4 -Progress %s" "rendrib -d 4 %s" "aqsis -fb %s"]
    addCheckB $fw ayprefse RenderUI [ms ayprefse_RenderUI]
    addStringB $fw ayprefse RenderPT [ms ayprefse_RenderPT]\
	    [list "R90000 %d" "Done computing %d" "%d"]
    addStringB $fw ayprefse SMRender [ms ayprefse_SMRender]\
	    [list "rendrib -Progress %s" "rendrib %s" "aqsis %s"]
    addCheckB $fw ayprefse SMRenderUI [ms ayprefse_SMRenderUI]
    addStringB $fw ayprefse SMRenderPT [ms ayprefse_SMRenderPT]\
	    [list "R90000 %d" "Done computing %d" "%d"]
    global AYENABLEPPREV
    if { $AYENABLEPPREV == 1 } {
	addStringB $fw ayprefse PPRender [ms ayprefse_PPRender] [list "rgl"]
    }
    # Misc
    set fw [$nb insert end Misc -text Misc\
	    -raisecmd "prefs_rsnb $nb Misc"]
    
    addText $fw e0 "Errors:"
    addCheckB $fw ayprefse RedirectTcl [ms ayprefse_RedirectTcl]
    addCheckB $fw ayprefse Logging [ms ayprefse_Logging]
    addStringB $fw ayprefse LogFile [ms ayprefse_LogFile]
    addText $fw e2 "UI:"
    addCheckB $fw ayprefse SaveAddsMRU [ms ayprefse_SaveAddsMRU]
    addCheckB $fw ayprefse ToolBoxTrans [ms ayprefse_ToolBoxTrans]
    addCheckB $fw ayprefse ToolBoxShrink [ms ayprefse_ToolBoxShrink]
    addCheckB $fw ayprefse RGTrans [ms ayprefse_RGTrans]
    addCheckB $fw ayprefse HideTmpTags [ms ayprefse_HideTmpTags]
    addParamB $fw ayprefse TclPrecision [ms ayprefse_TclPrecision]\
	    { 4 5 6 12 17 }
    addText $fw e3 "Tesselation:"
    addMenuB $fw ayprefse SMethod [ms ayprefse_SMethod]\
	    $ay(smethods)
    addParamB $fw ayprefse SParam [ms ayprefse_SParam] { 10 0.5 1 30 }

    # select last selected preference section
    pack $nb -fill both -expand yes

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

    button $f.bdef -text "Defaults" -width 8 -command {
	global ay ayprefse ayprefsdefaults
	set avnames [array names ayprefsdefaults]
	foreach j $avnames {
	    set ayprefse($j) $ayprefsdefaults($j)
	}
	# update color entries
	update
	set w .prefsw.f2.nb.fDrawing
	foreach c {Background Object Selection Grid Tag Shade Light} {
	    updateColorFromE $w ayprefse $c $w.f${c}.b1
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
	destroy .prefsw
    }

    pack $f.bok $f.bap $f.bdef $f.bca -in $f -side left -fill x -expand yes
    pack $f -in $w -side bottom -fill x -expand no

    #
    global ayprefs
    if { $ayprefs(AutoFocus) == 1 } {
	bind $w <Enter> {
	    if { "%W" == "[winfo toplevel %W]" } {
		focus %W
	    }
	}
    }

    # show the window
    wm deiconify $w
    update
    # show last selected section
    $nb raise $ay(prefssection)
    update
    $nb see $ay(prefssection)
    # resize notebook so that section is visible
    prefs_rsnb $nb $ay(prefssection)

    # establish "Help"-binding
    set m $ay(helpmenu)
    bind $w <[repcont $aymainshortcuts(Help)]> "$m invoke 0"

    # establish "Zap"-binding
    bind $w <[repcont $aymainshortcuts(Zap)]> zap
    bind $w <Map> unzap

    # bind function keys
    shortcut_fkeys $w

    focus $f.bok
    tkwait window $w

 return;
}
# prefs_open


# prefs_save:
#  save preference settings to ayamrc file
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
	set w [winfo width .]
	set h [winfo height .]

	set ng ""
	append ng "${w}x${h}"
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
	    set w [winfo width .tbw]
	    set h [winfo height .tbw]

	    set ng ""
	    append ng "${w}x${h}"
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


# prefs_toggleLazyNotification:
#
proc prefs_toggleLazyNotification { } {
    global ayprefs ayprefse aymainshortcuts

    if { $ayprefs(LazyNotify) == 1 } {
	set ayprefse(LazyNotify) 0
	set ayprefs(LazyNotify) 0
	ayError 4 $aymainshortcuts(SwLazyNotify) "LazyNotification turned off."
    } else {
	set ayprefse(LazyNotify) 1
	set ayprefs(LazyNotify) 1
	ayError 4 $aymainshortcuts(SwLazyNotify) "LazyNotification turned on."
    }

    setPrefs

 return;
}
# prefs_toggleLazyNotification


# prefs_toggleSurfaceWire:
#  toggle drawing of Curves/Surfaces vs. Wireframes (bound to F-Key)
proc prefs_toggleSurfaceWire { draw_surface } {
    global ayprefs ayprefse aymainshortcuts

    if { $draw_surface == 1 } {
	if { $ayprefs(DisplayMode) != 2 || $ayprefs(NCDisplayMode) != 0 } {
	    set ayprefse(DisplayMode) 2
	    set ayprefs(DisplayMode) 2
	    set ayprefse(NCDisplayMode) 0
	    set ayprefs(NCDisplayMode) 0
	    ayError 4  $aymainshortcuts(SwNURBS)\
		    "Drawing of Curves/Surfaces turned on."
	    setPrefs
	    update
	    rV
	}
    } else {
	if { $ayprefs(DisplayMode) != 0 || $ayprefs(NCDisplayMode) != 2 } {
	    set ayprefse(DisplayMode) 0
	    set ayprefs(DisplayMode) 0
	    set ayprefse(NCDisplayMode) 2
	    set ayprefs(NCDisplayMode) 2
	    ayError 4 $aymainshortcuts(SwWire)\
		    "Drawing of Wireframes turned on."
	    setPrefs
	    update
	    rV
	}
    }
    # if

 return;
}
# prefs_toggleSurfaceWire


# prefs_setSamplingTolerance:
#  set new sampling tolerance (bound to F-Key)
proc prefs_setSamplingTolerance { plus } {
    global ay ayprefs ayprefse aymainshortcuts

    if { $ay(sstsema) == 0 } {
	set ay(sstsema) 1
	update
	if { $plus == 1 } {
	    if { $ayprefs(Tolerance) < 90 } {
		set newval [ expr $ayprefs(Tolerance) + 10]
		set ayprefs(Tolerance) $newval
		set ayprefse(Tolerance) $newval
		ayError 4 $aymainshortcuts(SetSTP)\
			"SamplingTolerance set to ${newval}."
		setPrefs
		update
		rV
	    }
	} else {
	    if { $ayprefs(Tolerance) > 10 } {
		set newval [ expr $ayprefs(Tolerance) - 10]
		set ayprefs(Tolerance) $newval
		set ayprefse(Tolerance) $newval
		ayError 4 $aymainshortcuts(SetSTL)\
			"SamplingTolerance set to ${newval}."

		setPrefs
		update
		rV
	    }
	    
	}
	# if

	set ay(sstsema) 0
    }
    # if

 return;
}
# prefs_setSamplingTolerance
