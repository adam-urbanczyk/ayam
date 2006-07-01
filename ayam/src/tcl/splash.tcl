# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2001 by Randolf Schultz
# (rschultz@informatik.uni-rostock.de) and others.
#
# All rights reserved.
#
# See the file License for details.

# splash.tcl - view the splash screen

proc splash_open { } {
    global AYWRAPPED AYWITHAQUA ayprefs ay

    # first, create image
    if { $AYWRAPPED == 1 } {
	set f [wrap::open ayam-splash.gif]
	set idata [read $f]
	close $f
	image create photo ayam-splash -format GIF -data $idata
    } else {
	set oldcd [ pwd ]
	set scdir [ info script ]
	if { $scdir != "" } {
	    cd [ file dirname $scdir ] }
	if  { ! [file exists $ay(aysplash) ] } {
	    if  { ! [file exists "ayam-splash.gif" ] } {
		puts stderr "Could not find splash image: \"$ay(aysplash)\"!"
		cd $oldcd
		return;
	    } else { set ay(aysplash) "ayam-splash.gif" }
	}

	image create photo ayam-splash -file $ay(aysplash)
	cd $oldcd
    }

    # create toplevel window
    set w .aysplash
    catch {destroy $w}
    set visuals [winfo visualsavailable .]
    if { [lsearch $visuals truecolor*] != -1 } {
	toplevel $w -class ayam -visual truecolor
    } else {
	toplevel $w -class ayam
    }

    if { $AYWITHAQUA } {
	::tk::unsupported::MacWindowStyle style $w floating {closeBox}
    } else {
	if { $ayprefs(ToolBoxTrans) == 1 } {
	    wm transient $w .
	}
	bind $w <Visibility> "raise $w"
	wm resizable $w 0 0
    }

    wm title $w "Ayam"

    label $w.image -image ayam-splash
    pack $w.image  -side left
    bind $w <ButtonRelease-1> "grab release $w; after 50 destroy $w; focus ."

    # center the window
    if { ! $AYWITHAQUA } {
	wm overrideredirect $w 1
	winCenter $w
    } else {
	after idle "winCenter $w"
    }

    grab $w

    # arrange automatic closing after x seconds
    after 6000 {after idle splash_close}

 return;
}

proc splash_close { } {
    if { [winfo exists .aysplash] } {
	grab release .aysplash
	destroy .aysplash
	focus -force .
    }
 return;
}
