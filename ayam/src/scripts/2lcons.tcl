#
# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2005 by Randolf Schultz
# (randolf.schultz@gmail.com) and others.
#
# All rights reserved.
#
# See the file License for details.

# 2lcons.tcl - restrict console to display just two lines of text

# this version of pane_constrain allows more fine grained placement
proc pane_constrain {p h w0 w1 wh xy d} {
    global PANE
    regexp -- "\-rel$xy (\[^ \]+)" [place info $w0] junk t0
    regexp -- "\-rel$xy (\[^ \]+).*\-rel$wh (\[^ \]+)" \
	    [place info $w1] junk t1 t2
    # XXXX was /10.0
    set offset [expr ($t1+$t2-$t0)/100.0]
    array set PANE [list XY [winfo root$xy $p] WH [winfo $wh $p].0 \
	    W0 $w0 W1 $w1 XY0 $t0 XY1 [expr $t1+$t2] \
	    C0 [expr $t0+$offset] C1 [expr $t1+$t2-$offset]]
    bind $h <B1-Motion> "pane_motion %[string toup $xy] $p $h $wh $xy $d"
    if !$d {
	bind $h <ButtonRelease-1> \
		"pane_motion %[string toup $xy] $p $h $wh $xy 1"
    }
}
# pane_constrain

bind .fl.con.console <Configure> {
    after 500 {
	# restrict to exactly two lines
	set numlines 2

	# the number of lines could also come from the preferences:
	#set numlines $ayprefs(consolelines)

	# if you use this, the console will always maintain the current
	# number of lines (instead of claiming more space after user
	# resize operations):
	#set numlines [lindex [.fl.con.console conf height] 4]
	if { $::ay(ws) == "Aqua" } {
	    set m 8
	} else {
	    set m 4
	}
	if { $::tcl_platform(platform) == "windows" } { set m 2 }
	set textheight [lindex [font metrics [lindex\
			  [.fl.con.console conf -font] 4]] 5]
	set borderwidth [lindex [.fl.con.console conf -borderwidth] 4]
	set vheight [expr [winfo rooty .fl] + [winfo height .fl] -\
		     (($textheight * $numlines) + ($borderwidth * 2) + $m)]
	if { ! $::ayprefs(SingleWindow) } {
	    pane_constrain . .__h1 .fu .fl height y 1
	    pane_motion $vheight . .__h1 height y 1
	} else {
	    pane_constrain . .__h2 .fu .fl height y 1
	    pane_motion $vheight . .__h2 height y 1
	}
	# scroll console to end
	after idle {.fl.con.console see end}
    }
    # after
}
# bind

event generate .fl.con.console <Configure> -when tail