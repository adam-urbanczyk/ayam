# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2005 by Randolf Schultz
# (rschultz@informatik.uni-rostock.de) and others.
#
# All rights reserved.
#
# See the file License for details.

# win.tcl - (top level) window management


# winCenter:
# center window w on screen
proc winCenter { w } {

    wm withdraw $w
    update idletasks

    set x [expr [winfo screenwidth $w]/2 - [winfo reqwidth $w]/2 \
            - [winfo vrootx [winfo parent $w]]]
    set y [expr [winfo screenheight $w]/2 - [winfo reqheight $w]/2 \
            - [winfo vrooty [winfo parent $w]]]

    winMoveOrResize $w "+${x}+${y}"

    wm deiconify $w

 return;
}
# winCenter


# winMoveOrResize:
#  move or resize window w according to newgeom
proc winMoveOrResize { w newgeom } {
global ayprefs tcl_platform

    if { $newgeom == "" } { return }

    update idletasks
    set oldgeom [wm geom $w]
    regexp {([0-9]+)?x?([0-9]+)?(\+|\-)?([0-9]+)?(\+|\-)?([0-9]+)?} $oldgeom blurb width height blurb2 x blurb3 y

    regexp {([0-9]+)?x?([0-9]+)?(\+|\-)?([0-9]+)?(\+|\-)?([0-9]+)?} $newgeom blurb nw nh blurb2 nx blurb3 ny

    if { $nw == "" } {
	if { $nx != "" } { set x $nx }
	if { $ny != "" } { set y $ny }

	# never move off screen
	if { [expr abs($x)] >= [winfo screenwidth $w] } {
	    set x 0
	}
	if { [expr abs($y)] >= [winfo screenheight $w] } {
	    set y 0
	}
	# make decoration visible in all cases
	if { ($ayprefs(TwmCompat) == 0) &&
	    ($tcl_platform(platform) != "windows") } {
	    if { $y <= 5 } { incr y 10 }
	}

	set newgeom ""
	if { $x >= 0 } { append newgeom "+$x" } else { append newgeom "-$x" }
	if { $y >= 0 } { append newgeom "+$y" } else { append newgeom "-$y" }
	wm geom $w $newgeom
    } else {
	if { $nx == "" } {

	    if { $nw != "" } { set width $nw }
	    if { $nh != "" } { set height $nh }


	    if { ($ayprefs(TwmCompat) == 0) &&
	    ($tcl_platform(platform) != "windows") } {
		set x [winfo rootx $w]
		set y [winfo rooty $w]
		set newgeom "${width}x${height}"

		if { $x >= 0 } {
		    append newgeom "+$x"
		} else {
		    append newgeom "-$x"
		}
		if { $y >= 0 } {
		    append newgeom "+$y"
		} else {
		    append newgeom "-$y"
		}
	    } else {
		set newgeom "${width}x${height}"
	    }
	    wm geom $w $newgeom
	} else {
	    if { $nx != "" } { set x $nx }
	    if { $ny != "" } { set y $ny }
	    if { $nw != "" } { set width $nw }
	    if { $nh != "" } { set height $nh }

	    if { [expr abs($x)] >= [winfo screenwidth $w] } {
		set x 0
	    }
	    if { [expr abs($y)] >= [winfo screenheight $w] } {
		set y 0
	    }
	    # make decoration visible in all cases
	    if { ($ayprefs(TwmCompat) == 0) &&
	    ($tcl_platform(platform) != "windows") } {
		if { $y <= 5 } { incr y 10 }
	    }

	    set newgeom "${width}x${height}"
	    if { $x >= 0 } {
		append newgeom "+$x"
	    } else {
		append newgeom "-$x"
	    }
	    if { $y >= 0 } {
		append newgeom "+$y"
	    } else {
		append newgeom "-$y"
	    }
	    wm geom $w $newgeom
	}
    }

    update idletasks

 return;
}
# winMoveOrResize


# winIconWindow:
#  set icon window for window w
proc winIconWindow { w img } {

#image create photo icon_img -format GIF -file /home/randi/ayamsf/ayam/src/ayam-icon-t.gif

    if { ! [winfo exists .icon] } {
	toplevel .icon
	pack [label .icon.l -image $img ]
    }

    wm iconwindow $w .icon
    wm withdraw $w
    wm deiconify $w

 return;
}
# winIconWindow


# winGetGeom:
#  get geometry of window w ready to be fed into a call to wm geom
#  to re-restablish this geometry; TwmCompat controls, whether the
#  window manager (Unix/X11) adds the decoration to wm geom information
#  forcing us to use winfo instead.
proc winGetGeom { w } {
    global tcl_platform ayprefs

    if { ($tcl_platform(platform) != "windows") &&\
	    ($ayprefs(TwmCompat) != 1) } {
	set x [winfo rootx $w]
	set y [winfo rooty $w]
	set wi [winfo width $w]
	set he [winfo height $w]

	set ng ""
	append ng "${wi}x${he}"
	if { $x >= 0 } { append ng "+$x" } else { append ng "-$x" }
	if { $y >= 0 } { append ng "+$y" } else { append ng "-$y" }
	return $ng
    } else {
	return [wm geom $w]
    }

}
# winGetGeom


# winAutoFocusOff:
#  disable AutoFocus
proc winAutoFocusOff { } {
    global ay ayprefs

    if { $ayprefs(SafeAutoFocus) } {
	set ay(oldAutoFocus) $ayprefs(AutoFocus)
	set ayprefs(AutoFocus) 0
    }

 return;
}
# winAutoFocusOff

# winAutoFocusOn:
#  (re-)enable AutoFocus
proc winAutoFocusOn { } {
    global ay ayprefs

    if { $ayprefs(SafeAutoFocus) } {
	set ayprefs(AutoFocus) $ay(oldAutoFocus)
    }
 return;
}
# winAutoFocusOn