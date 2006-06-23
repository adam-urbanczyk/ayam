#!/usr/bin/env wish
#
# Ayam, a free 3D modeler for the RenderMan interface.
#
# Ayam is copyrighted 1998-2005 by Randolf Schultz
# (rschultz@informatik.uni-rostock.de) and others.
#
# All rights reserved.
#
# See the file License for details.

# repairAyam.tcl - repair Ayam if it is stuck; use only in emergency!


# do we run in wish?
if { [string first wish [file tail [info nameofexecutable]]] != -1 } {
    # yes, send the script to Ayam (via Tk send)

    # get (complete) path to this script
    if { [file pathtype [info script]] == "absolute" } {
	set repairscript [info script]
    } else {
	set repairscript [file join [pwd] [info script]]
    }

    puts "Sending myself to Ayam..."
    send ayam "source $repairscript"
    exit
}

# do we run in Wish Shell (on Mac OS X Aqua)?
if { [string first Wish [file tail [info nameofexecutable]]] != -1 } {
    # yes, send the script to Ayam (via AppleScript event)


    # get (complete) path to this script
    if { [file pathtype [info script]] == "absolute" } {
	set repairscript [info script]
    } else {
	set repairscript [file join [pwd] [info script]]
    }

    set script [subst "tell application \"Ayam\"\n  do script \"source \{$repairscript\}\"\nend tell"]
    puts "Sending myself to Ayam..."
    exec osascript -e $script
    exit
}

# no more error dialogs
set oldRedirectTcl $::ayprefs(RedirectTcl)
set ::ayprefs(RedirectTcl) 1

# close all view windows
catch [viewCloseAll]

# reset important global variables and semaphors
set ::ay(Views) ""
set ::ay(currentView) ""
set ::ay(cviewsema) 0
catch [cS]
set ::ay(PlbUpdateSema) 0
set ::ay(treeselectsema) 0
update
catch [plb_update]
catch [goTop]
catch [uS]

# remove potential grab
catch [grab release [grab current .]]

# reset mouse cursor
. configure -cursor {}
if { [winfo exists .tbw] == 1 } {
  .tbw configure -cursor {}
}

# reset console prompt
if { [winfo exists .fl.con] == 1 } {
    set .fl.con(-prompt)\
	    {\[Undo:$ay(undoo)/Redo:$ay(redoo)\].../[file tail [pwd]]>}
    Console:prompt .fl.con
}

# reset focus
focus -force .

# unzap (display all iconified windows)
unzap

# break potentially running endless loops
global cancelled
set cancelled 1
update
after 2000 {set cancelled 0}

# clear console
focus .fl.con.console
event generate .fl.con.console <Control-l>
focus -force .

set ::ayprefs(RedirectTcl) $oldRedirectTcl

# all done, hopefully it works again
update
