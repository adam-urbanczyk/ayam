# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2005 by Randolf Schultz
# (randolf.schultz@gmail.com) and others.
#
# All rights reserved.
#
# See the file License for details.

# balloon.tcl - code for displaying help balloons (tool tips)
# original code taken from "The Tcl'ers Wiki" (Richard Suchenwirth)
# Changes:
# added browse balloons feature;
# do not let balloons pop up partially off-screen

# balloon timing depends on the _global_ ayprefs(Balloon)!

# balloon_set:
#  set balloon help text <text> to window <w>
#
proc balloon_set {w help} {
    global balloonwin balloontime ayprefs

    set balloonwin ""
    set balloontime $ayprefs(Balloon)

    # 27.07.1999, #AmigaGER (IRCNet):
    # "<coto>Naja, tcl/tk ist eben geil :->"
    # (realizing, that I implemented the browse-balloons-feature in < 5 min)

    regsub -all "%" $help "%%" help2

    bind $w <Enter> "+
     if { \$balloonwin == \"\" } {
      after \$balloontime {set balloonwin %W ; balloon_show %W [list $help2] };
     }"

    bind $w <Leave> "+
	if { \$balloonwin != \"\" } {
	    set balloontime 100;
	    set balloonwin \"\";
	}
	after 50 { destroy %W.balloon;
	after cancel {set balloonwin %W ; balloon_show %W [list $help2] };
	after 500 [list set balloontime \$ayprefs(Balloon)] }"

 return;
}
# balloon_set


# balloon_show:
#  actually shows the balloon help window as toplevel window
#  without any decoration and with a light yellow background
proc balloon_show {w arg} {
    global ayprefs ay

    if { $ayprefs(showtt) == 0 } { return; }

    # sanity check
    if { [eval winfo containing  [winfo pointerxy .]] != $w } { return; }

    # open the balloon window
    set top $w.balloon
    catch {destroy $top}
    toplevel $top -bd 1 -bg black
    if { $ay(ws) == "Aqua" } {
	::tk::unsupported::MacWindowStyle style $top help noActivates
    } else {
	wm overrideredirect $top 1
    }
    pack [message $top.txt -width 100c -fg black -bg lightyellow -text \
	    [encoding convertfrom $arg]]

    set wmx [winfo rootx $w]
    set rw [winfo reqwidth $top.txt]

    # move near the pointer
    set px [winfo pointerx $w]
    if { [expr $wmx + $rw] < [expr $px] } {
	set wmx [expr $px - $rw - 10]
    }

    set tw [winfo toplevel $w]
    set rb [expr $wmx + [winfo reqwidth $tw]]
    set mw [lindex [wm maxsize $tw] 0]
    if { $mw <= [winfo screenwidth $top] } {
	# single monitor setup
	set rb [winfo screenwidth $top]
    } else {
	# multi monitor setup
	if { $mw < $rb } {
	    set rb $mw
	}
    }
    # do not move off-screen
    if { [expr $wmx + $rw] > $rb } {
	set wmx [expr $wmx - (([expr $wmx + $rw])-$rb)]
    }

    set wmy [expr [winfo rooty $w]+[winfo height $w]]
    wm geometry $top \
	    [winfo reqwidth $top.txt]x[winfo reqheight $top.txt]+$wmx+$wmy

    raise $top

 return;
}
# balloon_show


# balloon_setsplit:
#  set balloon help text <s> to window <w>
#  in contrast to balloon_set above, the text is a
#  list of e.g. path names that will be split here
#  into a proper string for balloon_set
proc balloon_setsplit { w s len } {
    global ay
    balloon_clear $w
    if {[string length $s] > $len } {
	regsub -all $ay(separator) $s  "\n" s2

	set ids [after info]
	foreach id $ids {
	    set script ""
	    catch {set script [after info $id]}
	    if { $script != "" } {
		if { [string first $w $script] != -1 } {
		    after cancel [lindex $script 0]
		}
	    }
	}
	update
	balloon_set $w $s2
	balloon_show $w $s2
    }
    # if
 return;
}
# balloon_setsplit


# balloon_clear:
#
#
proc balloon_clear { w } {

    bind $w <Enter> ""

    bind $w <Leave> ""

 return;
}
# balloon_clear

# EOF
