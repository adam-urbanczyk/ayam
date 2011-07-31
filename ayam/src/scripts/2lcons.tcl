#
# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2008 by Randolf Schultz
# (randolf.schultz@gmail.com) and others.
#
# All rights reserved.
#
# See the file License for details.

# 2lcons.tcl - restrict console to display just two lines of text

bind .fl.con.console <Configure> {
    after 500 {
	# restrict to exactly two lines, change at will
	set numlines 2

	# the number of lines could also come from the preferences:
	#set numlines $ayprefs(ConsoleLines)

	# the console could also always maintain the current
	# number of lines (instead of claiming more space after
	# user resize operations):
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
	    pane_constrain . .__h1 .fu .fl height y 1 100.0
	    pane_motion $vheight . .__h1 height y 1
	} else {
	    pane_constrain . .__h2 .fu .fl height y 1 100.0
	    pane_motion $vheight . .__h2 height y 1
	}
	# scroll console to end
	after idle {.fl.con.console see end}
    }
    # after
}
# bind

event generate .fl.con.console <Configure> -when tail

ayError 4 "2lcons" "Enabled."
