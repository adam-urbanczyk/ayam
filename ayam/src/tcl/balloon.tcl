# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2001 by Randolf Schultz
# (rschultz@informatik.uni-rostock.de) and others.
#
# All rights reserved.
#
# See the file License for details.

# balloon.tcl - code for displaying help balloons
# original code taken from "The Tcl'ers Wiki" (Richard Suchenwirth)
# Changes:
# added browse balloons feature;
# do not let balloons pop up partially off-screen

# balloon timing depends on the _global_ ayprefs(Balloon)!


#
#
#
proc balloon_set {w help} {
    global balloonwin balloontime ayprefs
    set balloonwin ""
    set balloontime $ayprefs(Balloon)

    # 27.07.1999, #AmigaGER (IRCNet):
    # "<coto>Naja, tcl/tk ist eben geil :->"
    # (realizing, that I implemented the browse-balloons-feature in < 5 min) 

    regsub -all "%" $help "%%" help2

    bind $w <Enter> "
    update;
     if { \$balloonwin == \"\" } {
      after \$balloontime {set balloonwin %W ; balloon_show %W [list $help2] };
     }"

    bind $w <Leave> "
	if { \$balloonwin != \"\" } {
	    set balloontime 100;
	    set balloonwin \"\";
	}
	destroy %W.balloon;
	after cancel {set balloonwin %W ; balloon_show %W [list $help2] };
	after 500 [list set balloontime \$ayprefs(Balloon)]"

 return;
}
# balloon_set

#
#
#
proc balloon_show {w arg} {
    global ayprefs

    if { $ayprefs(showtt) == 0 } { return; }

    # sanity check
    if {[eval winfo containing  [winfo pointerxy .]] != $w} {return}

    # open the balloon window
    set top $w.balloon
    catch {destroy $top}
    toplevel $top -bd 1 -bg black
    wm overrideredirect $top 1
    pack [message $top.txt -width 100c -fg black -bg lightyellow -text \
	    [encoding convertfrom $arg]] 

    # do not move off-screen
    set wmx [winfo rootx $w]
    set rw [winfo reqwidth $top.txt]
    if { [expr $wmx + $rw] > [winfo screenwidth $top] } {
	set wmx [expr $wmx - (([expr $wmx + $rw])-[winfo screenwidth $top])]
    }

    set wmy [expr [winfo rooty $w]+[winfo height $w]]
    wm geometry $top \
	    [winfo reqwidth $top.txt]x[winfo reqheight $top.txt]+$wmx+$wmy

    raise $top

 return;
}
# balloon_show


#
#
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

 return;
}
# balloon_setsplit

#
#
#
proc balloon_clear { w } {

    bind $w <Enter> ""

    bind $w <Leave> ""

 return;
}
# balloon_clear