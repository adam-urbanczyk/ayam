# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2001 by Randolf Schultz
# (rschultz@informatik.uni-rostock.de) and others.
#
# All rights reserved.
#
# See the file License for details.

# ayam.tcl - the main Ayam script

# Global Variables and Arrays:

# preference settings
array set ayprefs {
 showtt 1
 showtb 1
 showtr 1
 Balloon 1500
 TwmCompat 0
 toolBoxList {trafo trafo2 solids misco nurbs toolobjs points nctools1 nctools2 camera misc}
 AutoResize 1
 ListTypes 1
 AutoSavePrefs 1
 PickEpsilon 0.2
 PickTolerance 5.0
 UseMatColor 0
 HandleSize 6
 LazyNotify 0
 EditSnaps 1

 Tolerance 30.0
 DisplayMode 0
 NCDisplayMode 0

 Background_R 127
 Background_G 127
 Background_B 127

 Object_R 0
 Object_G 25
 Object_B 115

 Selection_R 255
 Selection_G 255
 Selection_B 255

 Grid_R 153
 Grid_G 153
 Grid_B 153

 Tag_R 127
 Tag_G 0
 Tag_B 0

 Shade_R 204
 Shade_G 191
 Shade_B 165

 Light_R 250
 Light_G 250
 Light_B 150

 RIBFile "Scene"
 Image "RIB"
 ResInstances 0
 CheckLights 1
 DefaultMat 1
 RIStandard 0
 WriteIdent 1
 ShadowMaps 0
 ExcludeHidden 1
 QRender "rgl -rd 4 %s"
 Render "rendrib -d 4 -Progress %s"
 RenderUI 1
 QRenderUI 0
 RenderPT "R90000 %d"
 QRenderPT ""
 SMRender "rendrib -Progress %s"
 SMRenderUI 1
 SMRenderPT "R90000 %d"
 PPRender "rgl"
 RenderMode 0
 TclPrecision 6
 Shaders ""

 TmpDir "/tmp"
 KeepNTmpFiles 5

 Plugins "plugins"

 Wpclip_pastetosel 1
 WarnChanged 1

 NewLoadsEnv 1
 SaveAddsMRU 1

 RedirectTcl 1
 Logging 0
 LogFile "/tmp/ay.log"

 UndoLevels 10

 mainGeom ""
 toolBoxGeom ""
 LoadEnv 1
 EnvFile "~/.ayam2view.ay"
 EFlush 2000
 mru {}
 ToolBoxTrans 0
 ToolBoxShrink 1
 RGTrans 0
 HideTmpTags 0
 Kill "kill"
 Cat "cat"
 MaxTagLen 30

 Locale "en"

 Scripts ""
 Docs "http://ayam.sourceforge.net/docs/"
    DailyTips {
{Always click on drawn pixels, when picking vertices.}
{There is a console-menu on the right mouse button.}        
{Use <Shift+TAB> to move the focus away from the console.}
{Documentation is available in the doc subdirectory.}
{Drag the borderline above the console to adjust it's height.}
{Point editing does not work right? Adjust PickEpsilon in Prefs!}
{The NURB circles and spheres are not to be tampered with!}
{New! Documentation is available in the doc subdirectory!}
{Tired of that "ever-auto-resizing" main window? See Prefs!}
{Copy a single parameter of a property by doubleclicking
on its name, then press <Ctrl+Shift+i> (Copy Marked Prop).}
{All views appear as childs of the Root-object!}
{Have I mentioned the documentation lately?}
{Drop geometric objects on to materials to connect them.}
{Drop lights on to views to check where they are lighting to.}
{Tired of the splash screen? Start Ayam with -nosplash.}
{Hold down the Alt-key to rotate the view while in any mode.}
{Start Ayam with -failsafe to reset prefs to factory defaults.}
{Ayam may be completely iconified using <Ctrl+Shift+z> (zap).}
{To create a material for a selected object, press <Ctrl+m>.}
{Extensive documentation is available as HTML and PDF.}
}
}
# array ayprefs


# standard shortcuts for main window
array set aymainshortcuts {
    Quit "Ctrl-q"
    New  "Ctrl-N"
    Replace "Ctrl-o"
    Insert "Ctrl-i"
    SaveAs "Ctrl-S"
    Save "Ctrl-s"
    ExportRIB "Ctrl-E"

    MRU1 "Ctrl-KeyPress-1"
    MRU2 "Ctrl-KeyPress-2"
    MRU3 "Ctrl-KeyPress-3"
    MRU4 "Ctrl-KeyPress-4"

    Apply "Ctrl-A"

    Copy "Ctrl-c"
    Cut "Ctrl-x"
    Paste "Ctrl-v"

    CopyP "Ctrl-C"
    ICopyP "Ctrl-I"
    PasteP "Ctrl-V"

    Undo "Ctrl-z"
    Redo "Ctrl-y"
    Material "Ctrl-m"
    Prefs "Ctrl-p"

    Help "F1"
    Zap "Ctrl-Z"
    SwCon "Shift-Tab"
    SwLazyNotify "F6"
    SwNURBS "F5"
    SwWire "F4"
    SetSTP "F3"
    SetSTL "F2"
}
# array aymainshortcuts


# standard shortcuts for a view window
array set ayviewshortcuts {
    Close "Ctrl-w"
    QRender "Ctrl-r"
    Render "Ctrl-R"
    Redraw "Ctrl-d"

    Front "Ctrl-f"
    Side "Ctrl-s"
    Top "Ctrl-t"
    Persp "Ctrl-p"
    Trim "Ctrl-T"

    Halve "Ctrl-H"
    Double "Ctrl-D"
    
    RotL "Left"
    RotR "Right"
    RotU "Up"
    RotD "Down"

    ZoomI "KP_Add"
    ZoomO "KP_Subtract"
    
    Auto "Ctrl-A"
    Shade "Ctrl-S"
    DGrid "Ctrl-g"
    UGrid "Ctrl-u"
    SGrid "Ctrl-G"

    Align "Ctrl-a"
    ZoomTO "Ctrl-o"

    MoveO "m"
    RotO "r"
    RotA "a"
    ScalX "x"
    ScalY "y"
    ScalZ "z"
    Scal2 "s"
    Scal3 "S"
    Stretch "Alt-s"

    Edit "e"
    EditD "p"
    WeightE "w"
    WeightR "W"
    Select "t"
    DeSelect "D"
    InsertP "i"
    DeleteP "d"
    SplitNC "c"
    FindU "u"

    MoveV "v"
    MoveZV "V"
    RotV "R"
    ZoomV "o"

    Break "Escape"
    MoveVButton "3"
    ZoomVButton "2"

    RotMod "Alt"
    RotModKey "Alt_L"
}
# array ayviewshortcuts


# standard RiAttributes DB
array set riattr {
    caustic { { maxpixeldist f {16} }
              { ngather i {75} }
              { specularcolor c {{0 0 0}} }
              { refractioncolor c {{0 0 0}} }
              { refractionindex f {1.0} }
            }
    indirect { { maxerror f {0.25 0.2 0.175 0.15 0.125 0.1} }
               { maxpixeldist f {20} }
               { nsamples i {256} }
	     }
    light { { nphotons i {0 10000 25000 50000} }
	  }
    radiosity { { averagecolor c }
                { emissioncolor c }
                { patchsize f {4} }
	        { elemsize f {2} }
                { minsize f {1} }
                { zonal s {none zonal_receives zonal_shoots fully_zonal} }
              }
    render { { use_shadingrate i {1 0} }
             { patch_multiplier f {1.0} }
	     { patch_maxlevel f {256} }
             { patch_minlevel f {1} }
           }

    trimcurve { {sense s {"inside" "outside"}} }

}
# array riattr


# standard RiOptions DB
array set riopt {
    indirect { {savefile s {"indirect.dat"} }
               {seedfile s {"indirect.dat"} }
             }

    limits { {derivmemory i {2} }
           }

    radiosity { { steps i {0 32 64 128} }
                { minpatchsamples i {1 2 3 4} }
              }

    render { {minsamples i {8}}
             {maxsamples i {64}}
	     {minshadowbias f {0.01 0.005 0.0025 0.001} }
	     {max_raylevel i {4 2 8 16} }
	     {prmanspecular i {0 1} }
	     {useprmandspy i {0 1} }
           }

    runtime { { verbosity s { "silent" "normal" "stats" "debug" } } }
    shadow { { bias0 f {0.01 0.1 0.25 0.5} } { bias1 f {0.01 0.1 0.25 0.5} } }
    statistics { {endofframe i {0 1 2 3} } {filename s} }
}
# array riopt

array set rioptval {}
array set riattrval {}

# just in case some binding fires...
set oldx 0
set oldy 0

# miscellaneous global values
array set ay {
 pady 3
 failsafe 0
 showsplash 1
 aysplash "../lib/ayam-splash.gif"
 filename ""
 tmpfiles ""
 ayamrc "~/.ayamrc"
 separator ":"
 autoload ""
 pasteProp 0
 views ""
 zapped ""
 currentView ""
 cviewsema 0
 viewtypenames { Front Side Top Persp Trim }
 surfaceshaders ""
 displacementshaders ""
 imagershaders ""
 volumeshaders ""
 transformationshaders ""
 npdisplaymodes { ControlHull OutlinePolygon OutlinePatch }
 ncdisplaymodes { Curve CurveAndHull ControlHull }
 prefssection Main
 GridSize 1.0
 FOV 45.0
 nclen 4
 npwidth 4
 npheight 4
 iclen 4
 pmwidth 4
 pmheight 4
 elevd 1
 insknu 0.5
 insknr 1
 refinekn 0.5
 splitu 0.5
 ncircarc 360.0
 cbspnum 6
 ts 0
 ul ""
 TreeUpdateSema 0
 co ""
 cm ""
 rnum 0
 sc 0
 tblayoutsema 0
 LastSelection ""
 slcext ""
 sext ""
 locales { en de fr }
 ScanShaders 0
 need_redraw 1
 sstsema 0
}
# array ay

# (re)set global variable that holds the error number
set ay_error 0

# Platform Specific Initialization:
if { $tcl_platform(platform) == "windows" } {
    set ay(separator) ";"
    set ay(pady) 0
    set ayprefs(TmpDir) "$env(TEMP)"
    regsub -all {\\} $ayprefs(TmpDir) {/} ayprefs(TmpDir)
    set ayprefs(EnvFile) "~/2view.ay"
    set ayprefs(PickEpsilon) 0.0
    set sc ""
    set scdir ""
    set sc [ info nameofexecutable ]
    if { $sc != "" } { set scdir [ file dirname $scdir ] }
	
    if { $ayprefs(Cat) == "cat" } {
	if { $scdir != "" } {
	    set ayprefs(Cat) [file join $scdir "cat.exe"]
	}
    }
    if  { $ayprefs(Cat) == "kill" } {
	if { $scdir != "" } {
	    set ayprefs(Kill) [file join $scdir "kill.exe"]
	}
    }
    set ayprefs(QRenderUI) 0
    set ayprefs(RenderUI) 0
    set ayprefs(ToolBoxTrans) 1
    set ayprefs(RGTrans) 1

    # we use this "emptyimg" for checkbuttons
    image create photo emptyimg
    emptyimg blank
    emptyimg configure -width 6 -height 6
} else {
    set ayprefs(Plugins) "[file dirname [info nameofexecutable]]/plugins"
    set ayprefs(Scripts) "plugins/loadrrib.tcl;plugins/loadidr.tcl"
}

# are true color visuals available?
if { [llength [info commands winfo]] != 0 } {
    set visuals [winfo visualsavailable .]
    if { [lsearch $visuals truecolor*] != -1 } {
	set ay(truecolor) 1
    } else {
	set ay(truecolor) 0
    }
}

# if envvar AYAMRC is set, use it
if { [string length [array names env AYAMRC]] != 0 } {
    set ay(ayamrc) $env(AYAMRC)
} else {
    if { $tcl_platform(platform) == "windows" } {
# if envvar HOME is not set (Win95?), use TEMP
	if { [string length [array names env HOME]] == 0 } {
	    set ay(ayamrc) "$env(TEMP)/ayamrc"
	} else {
	    set ay(ayamrc) "$env(HOME)/ayamrc"
	}
	regsub -all {\\} $ay(ayamrc) {/} ay(ayamrc)
    }
}


# Some Important Procedures:

# uCL: an optimized "update Selection", that just updates
# the current Level
# addargs - additional args, passed through to uS
proc uCL { mode {addargs ""} } {
    global ay

    if { $ay(lb) == 0 } {
	set oldnodes [$ay(tree) nodes  $ay(CurrentLevel)]

	if { $mode == "cl" } {

	    getLevel newnodes dummy
	    set i 0
	    foreach oldnode $oldnodes {
		set newnode [lindex $newnodes $i]
		if { $newnode != ".." } {
		    $ay(tree) itemconfigure $oldnode -text $newnode
		} else {
		    incr i
		    set newnode [lindex $newnodes $i]
		    $ay(tree) itemconfigure $oldnode -text $newnode
		}
		incr i
	    }

	}

	if { $mode == "cs" } {
	    $ay(tree) selection clear
	}



    } else { eval [subst "uS $addargs"] }

 return;
}
# uCL


# uCR: an optimized "update Selection" just for the
# case of newly created objects (CR); just adds new nodes
# to the current level of the tree
proc uCR { } {
    global ay

    if { $ay(lb) == 0} {
	set oldcount [llength [$ay(tree) nodes  $ay(CurrentLevel)]]
	set l ""
	getLevel l dummy

	set count 0
	foreach node $l {
	    if { $node != ".." } {
		if { $count >= $oldcount } {
		    $ay(tree) insert end $ay(CurrentLevel)\
		    $ay(CurrentLevel):$count -text $node -drawcross auto\
		    -open 0 -fill black -image emptybm
		    #tree_createSub $ay(tree) $ay(CurrentLevel) $node
		}
		incr count
	    }
	}

    } else { uS }

 return;
}
# uCR


# uS - update Selection
# if update_prop is 0 no update of the property
# GUIs will take place
# if maintain_selection is 1 the old selection will
# be established again; Note that this does only work
# properly, if you do not change the objects in
# the current level
proc uS { {update_prop "" } {maintain_selection "" } } {
    global ay

    if { $update_prop == "" } { set update_prop 1 }

    if { $maintain_selection == "" } { set maintain_selection 0 }

    if { $ay(lb) == 1 } {
	# plain ListBox is active
	global curlevel curtypes

	set lb $ay(olb)

	if { $maintain_selection } { set sel [$lb curselection] }

	# delete all current entries
	$lb delete 0 end

	# get current objects
	getLevel curlevel curtypes

	eval [subst "$lb insert end $curlevel"]

	if { $update_prop == 1 } {
	    plb_update
	}
	if { $maintain_selection && ($sel != "") } {
	    $lb selection set $sel
	}
    } else {
	update
	# TreeView is active
	set t $ay(tree)
	
	if { $maintain_selection } { set sel [$t selection get] }

	if { $ay(ul) == "" } {
	    tree_update root
	} else {
	    tree_update $ay(ul)
	    set ay(ul) ""
	}
	if { $ay(SelectedLevel) != "" } {
	    if { [$ay(tree) exists $ay(SelectedLevel)] } {
		tree_openTree $ay(tree) $ay(SelectedLevel)
	    } else {
		set ay(SelectedLevel) "root"
		set ay(CurrentLevel) "root"
		update
		tree_paintLevel $ay(SelectedLevel)
	    }
	    update
	} else {
	    set ay(SelectedLevel) "root"
	    update
	    tree_paintLevel $ay(SelectedLevel)
	}
	if { $update_prop == 1 } {
	    plb_update
	}

	if { $maintain_selection && ($sel != "") } {
	    $t selection set $sel
	}

    }

 return;
}
# uS


# sL - select Last
# select the last object in the current level
proc sL { } {
    global ay

    if { $ay(lb) == 1 } {
	# plain ListBox is active
	undo savsel
	set lb $ay(olb)
	$lb selection clear 0 end
	selOb
	$lb selection set end
	selOb [expr [$lb index end] - 1]
	plb_update
	$lb see end
    } else {
	# TreeView is active
	undo savsel
	$ay(tree) selection clear
	$ay(tree) selection set [$ay(tree) nodes $ay(CurrentLevel) end]
	tree_handleSelection
	plb_update
	$ay(tree) see [$ay(tree) nodes $ay(CurrentLevel) end]
    }

 return;
}
# sL


# cS - clear Selection
proc cS { } {
    global ay

    if { $ay(lb) == 1 } {
	# ListBox is active
	set lb $ay(olb)
	$lb selection clear 0 end
	selOb
    } else {
	# TreeView is active
	$ay(tree) selection clear

	if { $ay(CurrentLevel) == "root" } {
	    tree_paintLevel "root"
	}

	treeSelect
    }

 return;
}
# cS


# rV - redraw all Views
proc rV { } {
    global ay

    set tmp $ay(currentView)

    if { $ay(views) != "" } {
	foreach view $ay(views) {
	  	if { [winfo ismapped $view] } {
	    set view ${view}.f3D.togl
#	    $view mc
	    $view render
	}  
	}
    }

    if { $tmp != "" } {
	$tmp mc
	set ay(currentView) $tmp
    }

 return;
}
# rV


# tipoftheDay - print a "random" entry from $ayprefs(DailyTips) to the console
proc tipoftheDay { } {
    global ayprefs

    set count [llength $ayprefs(DailyTips)]
    expr srand([clock seconds])

    set tip [lindex $ayprefs(DailyTips) [expr round(rand()*double($count-1))]]
    puts stdout $tip

 return;
}
# tipoftheDay


# ayam_loadscript - load a script
proc ayam_loadscript { file } {
    global AYWRAPPED
    if { $AYWRAPPED == 1 } {
	uplevel #0 wrap::source tcl/${file}.tcl
    } else {
	set oldcd [ pwd ] 
	set scdir [ info script ]
	if { $scdir != "" } { cd [ file dirname $scdir ] }
	if { [ file exists ${file}.tcl ] } { uplevel #0 source ${file}.tcl }
	cd $oldcd
    }

 return;
}
# ayam_loadscript

# Ayam Startup Sequence:

# first, process some arguments
set i 0
set j 0
while { $i < $argc } {
    set arg [lindex $argv $i]
    if { (! [ string compare "-h" $arg ]) ||
         (! [ string compare "-help" $arg ]) } {
	puts " -h:        Display this help"
	puts " -nosplash: Do not display splash-image"
	puts " -failsafe: Do not load preferences and do not open a view"
	puts " 1.ay 2.ay: Load 1.ay, insert 2.ay"
	puts "\n Ayam - Reconstruct the World!"
	update
	exit
    } else {
	if { ! [ string compare "-failsafe" $arg ] } {
	    set ay(failsafe) 1
	}
	if { ! [ string compare "-nosplash" $arg ] } {
	    set ay(showsplash) 0
	}
    }
 incr i
}

# show splash screen
ayam_loadscript win
ayam_loadscript splash
if { $ay(showsplash) == 1 } { splash_open }

# create UI
ayam_loadscript balloon
set w .
wm title $w "Ayam - Main"
wm iconname $w "Ayam"
wm withdraw .
# XXXX Does this meet ICCCM requirements?
wm command . "[info nameofexecutable] $argv"
wm client . [info hostname]

# create the upper frame...
frame .fu
pack .fu -in . -side top
# with the real user interface
frame .fu.fMain
update

# initialize io procedures
ayam_loadscript io

# create the main menu
ayam_loadscript mmenu

# properly exit the program when the main window is closed
wm protocol . WM_DELETE_WINDOW {
    global ay
    set m $ay(filemenu)
    $m invoke 21
}

frame .fu.fMain.fHier
# create the tree widget
treeInit
#XXXX wie laedt man BWidgets in einem wrapped executable?
if { $AYWRAPPED == 1 } {
    if { $tcl_platform(platform) == "windows" } {
	lappend auto_path [file dirname [info nameofexecutable]]
    } else {
	set curdir [pwd]
	cd $ayprefs(TmpDir)

	if {$ay(failsafe) == 1 } { catch {file delete -force BWidgets} }
	if { [auto_execok unzip] != "" } {
	    exec unzip -u -qq [info nameofexecutable] BWidgets/\*
	} else {
	    puts "Could not execute unzip, please install it!"
	    exit
	}
	cd $curdir
	lappend auto_path ${ayprefs(TmpDir)}/BWidgets
    }
    ayam_loadscript tree
} else {

    set scdir [file dirname [info script]]
    if { [file pathtype $scdir] == "relative" } {
	set bwdir [file join [pwd] $scdir]/BWidget-1.2.1
    } else {
	set bwdir ${scdir}/BWidget-1.2.1
    }
    lappend auto_path $bwdir
    ayam_loadscript tree
}

# load script for object listbox (olb)
ayam_loadscript olb

tree_open .fu.fMain.fHier
update

#olb_open .fu.fMain.fHier
#update

# object bar
# has to be initialized before the pane for Object Hierarchy!
#frame .fu.fobjbar
#source contrib/objbar.tcl
#objbar_open .fu.fobjbar
#pack .fu.fobjbar -side top -fill x -expand yes

pack .fu.fMain -in .fu -side top -fill both -expand yes
pack .fu.fMain.fHier -in .fu.fMain -side left -expand no

# the lower frame (contains the console)
frame .fl

# create console
ayam_loadscript widget
ayam_loadscript console

pack .fl -in . -side bottom -fill both
frame .fl.dummy
console .fl.con -showmenu 0 -height 5 -width 60
pack .fl.con -in .fl -expand 1 -fill both
# additional key/mouse bindings for the console
bind .fl.con.console $aymainshortcuts(SwCon) { focus [tk_focusNext %W] }

# fix Shift-Tab binding
if { ( $tcl_platform(platform) != "windows" ) &&
     ( $tcl_platform(os) != "Darwin" ) } {
	bind all <ISO_Left_Tab> {tkTabToWindow [tk_focusPrev %W]}
}

bind .fl.con.console <ButtonPress-4> {
    .fl.con.console yview scroll -1 pages; break
}

bind .fl.con.console <ButtonPress-5> {
    .fl.con.console yview scroll 1 pages; break
}

# establish paned window management for console
update
if { $tcl_platform(platform) == "windows" } {
    wm deiconify .
    update
}
ayam_loadscript pane
set vheight [winfo rooty .fl]
pane .fu .fl -orient vertical
pane_constrain . .__h1 .fu .fl height y 1
pane_motion $vheight . .__h1 height y 1

# clear console
if { [winfo exists .fl.con] == 1 } { .fl.con clear }

# console prompt == current dir
if { [winfo exists .fl.con] == 1 } {
    set .fl.con(-prompt) {[file tail [pwd]]>}
    Console:prompt .fl.con
}

# create property listbox (plb) and property GUI (property canvas)
ayam_loadscript plb

frame .fu.fMain.fProp
pack .fu.fMain.fProp -in .fu.fMain -side left -expand yes -fill both
plb_open .fu.fMain.fProp
update

# establish paned window management for hierarchy
set vwidth [expr [winfo rootx .fu.fMain.fProp]+5]
pane .fu.fMain.fHier .fu.fMain.fProp
pane_constrain . .fu.fMain.__h1 .fu.fMain.fHier .fu.fMain.fProp width x 1
pane_motion $vwidth . .fu.fMain.__h1 width x 1
if { $tcl_platform(platform) == "windows" } {
    update
}

# load "some" external scripts
ayam_loadscript run
ayam_loadscript uie
ayam_loadscript shortcut
ayam_loadscript ms
ayam_loadscript box
ayam_loadscript bpatch
ayam_loadscript camera
ayam_loadscript cap
ayam_loadscript clone
ayam_loadscript cone
ayam_loadscript concatnc
ayam_loadscript cylinder
ayam_loadscript disk
ayam_loadscript extrude
ayam_loadscript hyperb
ayam_loadscript icurve
ayam_loadscript instance
ayam_loadscript level
ayam_loadscript light
ayam_loadscript material
ayam_loadscript ncurve
ayam_loadscript npatch
ayam_loadscript pamesh
ayam_loadscript pomesh
ayam_loadscript parab
ayam_loadscript riinc
ayam_loadscript revolve
ayam_loadscript root
ayam_loadscript sphere
ayam_loadscript skin
ayam_loadscript sweep
ayam_loadscript torus
ayam_loadscript prop
ayam_loadscript pclip
ayam_loadscript control
ayam_loadscript tmp
ayam_loadscript render
ayam_loadscript icons
ayam_loadscript action
ayam_loadscript vmenu
ayam_loadscript view
ayam_loadscript shader
ayam_loadscript riattr
ayam_loadscript riopt
ayam_loadscript prefs
ayam_loadscript zap
ayam_loadscript toolbox
ayam_loadscript about
ayam_loadscript browser
ayam_loadscript ai
ayam_loadscript prog
ayam_loadscript objsel
ayam_loadscript tc

# load scripts of statically linked additional modules
if { $AYIDRWRAPPED == 1 } {
    ayam_loadscript idr
}

if { $AYMETAWRAPPED == 1 } {
    ayam_loadscript metaobj
    ayam_loadscript metacomp
}

if { $AYRRIBWRAPPED == 1 } {
    ayam_loadscript rrib
}

# bind . <Configure> { if { "%W" == "." } { puts stderr "%w" } }
# olb_update
# EOF
if { $tcl_platform(platform) != "windows" } {
    wm deiconify .
}

# ayam_flush - flush error messages each 2s
proc ayam_flush { } {
    global ayprefs
    ayError 3
    after $ayprefs(EFlush) { ayam_flush }
}
# ayam_flush
after $ayprefs(EFlush) { ayam_flush }

# Ignition:
puts stdout "Ayam-Startup-Sequence initiated."

# read ayamrc
if { $ay(failsafe) == 0 } {
    puts stdout "Sourcing $ay(ayamrc)..."
    if { [file exists $ay(ayamrc)] } {
	if { [source $ay(ayamrc)] != "" } {
	    puts stderr "Oops? ayamrc returned something?"
	}
    } else {

	puts stderr "Not found. Please save preferences."
	
    }
}

# make (new) preference settings known to C-context
prefs_set

# immediately switch to ListBox?
if { $ayprefs(showtr) == 0 } {
    tree_close .fu.fMain.fHier
    olb_open .fu.fMain.fHier
    olb_update
}
update

shader_scanAll

puts stdout "Establishing key bindings..."
shortcut_main .

if { $ayprefs(showtb) == 1 } {
    toolbox_open
}

if { $ayprefs(mainGeom) != "" } {
    winMoveOrResize . $ayprefs(mainGeom)
}

if { ($ayprefs(LoadEnv) == 1) && ($ay(failsafe) == 0) } {
    viewCloseAll
    puts stdout "Loading environment from $ayprefs(EnvFile)..."
    set filename [file nativename $ayprefs(EnvFile)]
    if { $tcl_platform(platform) == "windows" } {
     regsub -all {\\} $filename {/} filename
    }
    replaceScene $filename
    uS
    rV
} else {
    uS
}


if { $ayprefs(Scripts) != "" } {
puts stdout "Running user defined scripts..."

set scripts [split "$ayprefs(Scripts)" $ay(separator)]

foreach script $scripts {
    if { [file exists $script] } {
	puts ${script}...
	catch {source $script}
    }
}
}

# now, process remaining arguments
puts stdout "Processing remaining arguments..."
set i 0
set j 0
grab .fu

while { $i < $argc } {
    set arg [lindex $argv $i]
    if { [file extension $arg] == ".ay" } {
	set newfilename $arg
	
	if { $j == 0 } {
	    viewCloseAll
	    set filename $newfilename
	    set ay_error ""
	    
	    replaceScene $filename
	    if { $ay_error < 2 } {
		set ay(filename) $filename
		set windowfilename [file tail [file rootname $newfilename]]
		wm title . "Ayam - Main - $windowfilename"
		ayError 4 "replaceScene" "Done reading scene from:"
		ayError 4 "replaceScene" "$filename"
		if { [file exists $filename] } {
		    set dirname [file dirname $filename]
		    cd $dirname
		    set .fl.con(-prompt) {[file tail [pwd]]>}
		    .fl.con delete end-1lines end
		    Console:prompt .fl.con "\n"
		}
		io_mruAdd $filename
	    } else {
		ayError 2 "Ayam" "There were errors while loading:"
		ayError 2 "Ayam" "$filename"
	    }
	    set j 1
	} else {
	    set ifilename $newfilename
	    insertScene $ifilename
	}
	uS; rV
    }
 incr i
}
grab release .fu

puts stdout "The tip of the day is:"
tipoftheDay
puts stdout "Ayam-Startup-Sequence finished. Reconstruct the World!"

# save preference settings for use with the "Def."-Button in the
# preferences dialog
set avnames [array names ayprefs]
foreach j $avnames {
    set ayprefsdefaults($j) $ayprefs($j)
}

# build most recently used files menu entries
io_mruUMenu

# auto scroll property canvas to item with focus
bind all <Tab> +plb_focus
bind all <Shift-Tab> +plb_focus
if { ( $tcl_platform(platform) != "windows" ) &&
     ( $tcl_platform(os) != "Darwin" ) } {
    bind all <ISO_Left_Tab> +plb_focus
}

# redirect all tcl errors to the console?
# this first catch works around a buglet in Tcl8.0.x where
# bgerror is not renamable until called a first time
catch {::bgerror}
catch {rename ::bgerror orig_bgerror}

# bgerror - handle all Tcl error messages
proc bgerror { message } {
    global ayprefs

    if { $ayprefs(RedirectTcl) == 1 } {
	regsub -all "\"" $message "\\\"" message
	ayError 2 Tcl $message
    } else {
	orig_bgerror $message
    }
}
# bgerror

# now activate all views
foreach view $ay(views) { viewBind $view }
# if there is a view window under the mouse pointer, make it current
after idle viewMouseToCurrent

# Reconstruct the World!
