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
    global AYWRAPPED ayprefs ay

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

    set visuals [winfo visualsavailable .]
    if { [lsearch $visuals truecolor*] != -1 } {
	toplevel $w -visual truecolor
    } else {
	toplevel $w
    }

    bind $w <Visibility> "raise $w"
    wm resizable $w 0 0 
    wm title $w "Ayam"
    wm transient $w .

    label $w.image -image ayam-splash
    pack $w.image  -side left
    bind $w <ButtonRelease-1> "grab release $w; after 50 destroy $w"

    wm overrideredirect $w 1
    # center the window
    winCenter $w

    grab $w

    # arrange automatic closing after x seconds
    after 6000 {after idle splash_close}

 return;
}

proc splash_close { } {
    grab release .aysplash
    destroy .aysplash
 return;
}
